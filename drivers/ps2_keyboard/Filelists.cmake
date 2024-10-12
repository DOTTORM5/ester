set(PS2_KEYBOARD_FILES
    ${ESTER_DIR}/drivers/ps2_keyboard/ps2_keyboard.c 
)

include_directories(
    ${ESTER_DIR}/drivers/ps2_keyboard
)

target_sources(ester.bin PRIVATE ${PS2_KEYBOARD_FILES})