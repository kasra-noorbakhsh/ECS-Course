#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/device.h>

#define DEVICE_NAME "lifo"
#define BUFFER_SIZE (1024 * 1024) // 1 MB
#define READ_ONLY_MINOR 0
#define WRITE_ONLY_MINOR 1
#define MAX_MINORS 2

// Structure to hold LIFO device data
struct lifo_dev {
    char *buffer;               // LIFO buffer
    spinlock_t lock;            // Spinlock for synchronization
    wait_queue_head_t read_queue; // Wait queue for readers
    struct cdev cdev;           // Character device structure
};

// Global variables
static struct lifo_dev *lifo_devices;
static dev_t lifo_dev_number;
static struct class *lifo_class;
static size_t shared_top = 0; // Shared top index for LIFO

// Open function
static int lifo_open(struct inode *inode, struct file *filp) {
    struct lifo_dev *dev = container_of(inode->i_cdev, struct lifo_dev, cdev);
    filp->private_data = dev;
    
    // Check permissions based on minor number
    unsigned int minor = iminor(inode);
    if (minor == READ_ONLY_MINOR && (filp->f_mode & FMODE_WRITE)) {
        printk(KERN_WARNING "lifo: Write access denied on read-only device\n");
        return -EACCES;
    }
    if (minor == WRITE_ONLY_MINOR && (filp->f_mode & FMODE_READ)) {
        printk(KERN_WARNING "lifo: Read access denied on write-only device\n");
        return -EACCES;
    }
    
    printk(KERN_INFO "lifo: Device opened, minor = %u\n", minor);
    return 0;
}

// Release function
static int lifo_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "lifo: Device released, minor = %u\n", iminor(inode));
    return 0;
}

// Read function
static ssize_t lifo_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct lifo_dev *dev = filp->private_data;
    unsigned long flags;
    ssize_t ret = 0;

    spin_lock_irqsave(&dev->lock, flags);
    
    // If LIFO is empty, return EOF immediately
    if (shared_top == 0) {
        spin_unlock_irqrestore(&dev->lock, flags);
        return 0; // EOF
    }
    
    // Read the topmost characters (LIFO)
    count = min(count, shared_top);
    shared_top -= count;
    spin_unlock_irqrestore(&dev->lock, flags);
    
    // Copy data to user space, starting from the top (LIFO order)
    if (copy_to_user(buf, dev->buffer + shared_top, count)) {
        ret = -EFAULT;
    } else {
        ret = count;
    }
    
    printk(KERN_INFO "lifo: Read %zd bytes, top = %zu\n", ret, shared_top);
    return ret;
}

// Write function
static ssize_t lifo_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct lifo_dev *dev = filp->private_data;
    unsigned long flags;
    ssize_t ret = 0;

    spin_lock_irqsave(&dev->lock, flags);
    
    // Check if buffer has space
    if (shared_top + count > BUFFER_SIZE) {
        spin_unlock_irqrestore(&dev->lock, flags);
        printk(KERN_WARNING "lifo: Buffer full, cannot write %zu bytes\n", count);
        return -ENOSPC;
    }
    
    // Copy data from user space
    if (copy_from_user(dev->buffer + shared_top, buf, count)) {
        spin_unlock_irqrestore(&dev->lock, flags);
        return -EFAULT;
    }
    
    shared_top += count;
    ret = count;
    
    spin_unlock_irqrestore(&dev->lock, flags);
    
    // Wake up any waiting readers
    wake_up_interruptible(&dev->read_queue);
    
    printk(KERN_INFO "lifo: Wrote %zd bytes, top = %zu\n", ret, shared_top);
    return ret;
}

// File operations structure
static const struct file_operations lifo_fops = {
    .owner = THIS_MODULE,
    .open = lifo_open,
    .release = lifo_release,
    .read = lifo_read,
    .write = lifo_write,
};

// Module initialization
static int __init lifo_init(void) {
    int ret, i;
    struct device *device;

    // Allocate device numbers
    ret = alloc_chrdev_region(&lifo_dev_number, 0, MAX_MINORS, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "lifo: Failed to allocate chrdev region\n");
        return ret;
    }
    
    // Create device class
    lifo_class = class_create(DEVICE_NAME);
    if (IS_ERR(lifo_class)) {
        printk(KERN_ERR "lifo: Failed to create class\n");
        unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
        return PTR_ERR(lifo_class);
    }
    
    // Allocate memory for devices
    lifo_devices = kzalloc(MAX_MINORS * sizeof(struct lifo_dev), GFP_KERNEL);
    if (!lifo_devices) {
        printk(KERN_ERR "lifo: Failed to allocate devices\n");
        class_destroy(lifo_class);
        unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
        return -ENOMEM;
    }
    
    // Initialize each device
    for (i = 0; i < MAX_MINORS; i++) {
        // Allocate buffer (shared for both devices, but only initialize once)
        if (i == 0) {
            lifo_devices[i].buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
            if (!lifo_devices[i].buffer) {
                printk(KERN_ERR "lifo: Failed to allocate buffer\n");
                kfree(lifo_devices);
                class_destroy(lifo_class);
                unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
                return -ENOMEM;
            }
            shared_top = 0;
        } else {
            lifo_devices[i].buffer = lifo_devices[0].buffer; // Share buffer
        }
        
        // Initialize synchronization primitives
        spin_lock_init(&lifo_devices[i].lock);
        init_waitqueue_head(&lifo_devices[i].read_queue);
        
        // Setup cdev
        cdev_init(&lifo_devices[i].cdev, &lifo_fops);
        lifo_devices[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&lifo_devices[i].cdev, MKDEV(MAJOR(lifo_dev_number), i), 1);
        if (ret) {
            printk(KERN_ERR "lifo: Failed to add cdev for minor %d\n", i);
            kfree(lifo_devices[0].buffer);
            kfree(lifo_devices);
            class_destroy(lifo_class);
            unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
            return ret;
        }
        
        // Create device file
        device = device_create(lifo_class, NULL, MKDEV(MAJOR(lifo_dev_number), i),
                               NULL, i == READ_ONLY_MINOR ? "lifo_rd" : "lifo_wr");
        if (IS_ERR(device)) {
            printk(KERN_ERR "lifo: Failed to create device file for minor %d\n", i);
            cdev_del(&lifo_devices[i].cdev);
            kfree(lifo_devices[0].buffer);
            kfree(lifo_devices);
            class_destroy(lifo_class);
            unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
            return PTR_ERR(device);
        }
    }
    
    printk(KERN_INFO "lifo: Driver initialized, major = %d\n", MAJOR(lifo_dev_number));
    return 0;
}

// Module cleanup
static void __exit lifo_exit(void) {
    int i;
    
    // Cleanup devices
    for (i = 0; i < MAX_MINORS; i++) {
        device_destroy(lifo_class, MKDEV(MAJOR(lifo_dev_number), i));
        cdev_del(&lifo_devices[i].cdev);
    }
    
    // Free buffer (only once, as it's shared)
    kfree(lifo_devices[0].buffer);
    kfree(lifo_devices);
    
    // Destroy class and unregister device numbers
    class_destroy(lifo_class);
    unregister_chrdev_region(lifo_dev_number, MAX_MINORS);
    
    printk(KERN_INFO "lifo: Driver unloaded\n");
}

module_init(lifo_init);
module_exit(lifo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xAI");
MODULE_DESCRIPTION("Virtual LIFO Character Device Driver");
