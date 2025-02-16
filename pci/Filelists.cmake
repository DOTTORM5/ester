set(ESTER_PCI_SRC
    ${ESTER_DIR}/pci/pci.c
)

target_sources(ester.bin PRIVATE ${ESTER_PCI_SRC})
