# ester
Ester, a really funny OS

## Getting Started
```
mkdir build; 
cd build;
cmake ..;
make;
make iso;
```
The system uses Qemu for debugging. 
"Make iso" launches Qemu with the ester iso and wait for GDB connection.
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

### To do
- Implement a strong printk system (through vga for now)
- Implement keyboard support (ps/2 for now)
- Implement PCI driver/logic
- TEST session 1
- Implement AHCI (for SATA controller) over PCI driver
- TEST session 2
- Implement PCIe Enhanced Configuration Mechanism and Extended Configuration Space driver/logic
- TEST session 3
- Create a file system on AHCI SATA SSD
- TEST session 4
- Implement memory management (expecially heap)
- TEST session 5
- Review old code, should use dinamic allocation?
- TEST session 6
- Implement task management
- TEST session 7
- Execute a program from the SSD
- TEST session 8
- TEST session 9 (final test)
- First release !!!

Each TEST session includes also a complete refactor since for now I'm putting all stuff of something in a single file (PCI for example)

### Extra to do
- Examine every gcc flag and setting (using a modified gcc cross compiler)
- Examine all multiboot2 parameters into the info structure (maybe there is also PCIe there, ACPI MCFG...)

What I call VGA, actually is the legacy VGA text mode.