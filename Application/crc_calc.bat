
copy .\Debug\Application.hex Application.hex

srec_cat.exe Application.hex -intel -exclude 0x0800C01C 0x0800C024 -STM32 0x0800C020 -exclusivelength-l-e 0x0800C01C 4 -multiple -o Application_crc.hex -intel

