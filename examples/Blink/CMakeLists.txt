file(GLOB SRC_FILES *.h *.cpp *.ino)

generate_energia_firmware(Blink
    SKETCH ${SRC_FILES}
#    LIBS Timers
    BOARD lpcc3200
    PORT /dev/tty.usbserial-cc3101B)