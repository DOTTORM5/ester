# ester
Ester, a really funny OS

## Getting Started
```
mkdir build; 
cd build;
cmake .. -DDEBUG=0;
make;
dd if=/dev/zero of=ext2_example.img bs=1M count=100;
mkfs.ext2 ext2_example.img;
make iso;
```
The system uses Qemu for debugging. 
If DEBUG=1: 
"Make iso" launches Qemu with the ester iso and waits for GDB connection.
To use GDB
```
gdb ester.bin
target remote localhost:1234
```

### Common Issues
#### GTK problem
```
unset GTK_PATH;
```
