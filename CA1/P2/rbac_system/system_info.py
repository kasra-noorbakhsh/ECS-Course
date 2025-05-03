import psutil
import platform
import shutil
import os

def get_cpu_info():
    print("=== CPU Info ===")
    print(f"Logical CPUs: {psutil.cpu_count(logical=True)}")
    print(f"Physical CPUs: {psutil.cpu_count(logical=False)}")
    print(f"CPU Usage per core: {psutil.cpu_percent(percpu=True)}")
    print(f"Total CPU Usage: {psutil.cpu_percent()}%")
    print()

def get_memory_info():
    print("=== Memory Info ===")
    virtual_mem = psutil.virtual_memory()
    print(f"Total: {virtual_mem.total / (1024 ** 3):.2f} GB")
    print(f"Available: {virtual_mem.available / (1024 ** 3):.2f} GB")
    print(f"Used: {virtual_mem.used / (1024 ** 3):.2f} GB")
    print(f"Percentage: {virtual_mem.percent}%")
    print()

def get_disk_info():
    print("=== Disk Info ===")
    disk_usage = psutil.disk_usage('/')
    print(f"Total: {disk_usage.total / (1024 ** 3):.2f} GB")
    print(f"Used: {disk_usage.used / (1024 ** 3):.2f} GB")
    print(f"Free: {disk_usage.free / (1024 ** 3):.2f} GB")
    print(f"Percentage: {disk_usage.percent}%")
    print()

def main():
    print(f"System: {platform.system()} {platform.release()}")
    print(f"Machine: {platform.machine()}")
    print(f"Processor: {platform.processor()}")
    print("-" * 30)

    get_cpu_info()
    get_memory_info()
    get_disk_info()

if __name__ == "__main__":
    main()
