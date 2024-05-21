# ester
Ester, a really funny OS

### To do
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
- Examine all multiboo2 parameters into the info structure (maybe there is also PCIe there, ACPI MCFG...)

What I call VGA, actually is the legacy VGA text mode.