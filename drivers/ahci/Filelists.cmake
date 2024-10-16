set(AHCI
    ${ESTER_DIR}/drivers/ahci/ahci.c 
)

include_directories(
    ${ESTER_DIR}/drivers/ahci
)

target_sources(ester.bin PRIVATE ${AHCI})