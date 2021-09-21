
.\bin\srec_cat .\Objects\f072.hex -intel -STM32_Little_Endian -maximum-addr .\Objects\f072.hex -intel -o .\Objects\f072.hex -intel
.\bin\srec_cat .\Objects\f072.hex -intel -offset - -minimum-addr .\Objects\f072.hex -intel -o program.bin -binary
.\bin\srec_cat .\BOOTLOADER\Objects\f072.hex -intel -fill 0xFF 0x8000000 0x8004000 .\Objects\f072.hex -intel -o KBK3_BootAndMainProg.hex -intel