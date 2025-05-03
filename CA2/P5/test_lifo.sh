#!/bin/bash

# Exit on any error
set -e

# Variables
DRIVER_NAME="lifo_driver"
MODULE_NAME="lifo_driver.ko"
DEVICE_RD="/dev/lifo_rd"
DEVICE_WR="/dev/lifo_wr"
READER="reader"
WRITER="writer"
MAJOR=""
TEST_MESSAGE="HelloLIFO"

# Step 1: Compile driver and programs
echo "Compiling driver and test programs..."
make
gcc -o $READER reader.c
gcc -o $WRITER writer.c

# Step 2: Unload existing module (if loaded)
echo "Unloading existing driver (if any)..."
sudo rmmod $DRIVER_NAME 2>/dev/null || true

# Step 3: Load the driver
echo "Loading driver..."
sudo insmod $MODULE_NAME

# Step 4: Find major number and create device nodes
echo "Creating device nodes..."
MAJOR=$(grep lifo /proc/devices | awk '{print $1}')
if [ -z "$MAJOR" ]; then
    echo "Error: Driver not found in /proc/devices"
    sudo rmmod $DRIVER_NAME
    exit 1
fi

# Remove existing device nodes (if any)
sudo rm -f $DEVICE_RD $DEVICE_WR

# Create device nodes
sudo mknod $DEVICE_RD c $MAJOR 0
sudo mknod $DEVICE_WR c $MAJOR 1

# Set permissions
sudo chmod 666 $DEVICE_RD $DEVICE_WR

# Step 5: Test functionality
echo "Running tests..."

# Test 1: EOF on empty LIFO
echo "Test 1: Reading from empty LIFO (expect EOF)"
./$READER $DEVICE_RD

# Test 2: Write and read (LIFO behavior)
echo "Test 2: Writing and reading (expect reversed message)"
./$WRITER $DEVICE_WR "$TEST_MESSAGE"
./$READER $DEVICE_RD

# Test 3: Read with data available
echo "Test 3: Reading with data available (expect immediate read)"
./$WRITER $DEVICE_WR "AvailableTest" &
sleep 1 # Ensure writer completes
./$READER $DEVICE_RD

# Test 4: Read-only enforcement
echo "Test 4: Attempt to write to read-only device (expect failure)"
./$WRITER $DEVICE_RD "ShouldFail" && {
    echo "Error: Write to read-only device succeeded"
    exit 1
} || echo "Write to read-only device failed (as expected)"

# Test 5: Write-only enforcement
echo "Test 5: Attempt to read from write-only device (expect failure)"
./$READER $DEVICE_WR && {
    echo "Error: Read from write-only device succeeded"
    exit 1
} || echo "Read from write-only device failed (as expected)"

# Step 6: Cleanup
echo "Cleaning up..."
sudo rm -f $DEVICE_RD $DEVICE_WR
sudo rmmod $DRIVER_NAME
make clean
rm -f $READER $WRITER

echo "All tests completed successfully!"
