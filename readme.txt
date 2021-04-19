
Пример загрузчика и приложения для контроллера STM32F103RET6 с загрузкой
по USB в режиме DFU (device firmware upgrade).

В HEX прошивку приложения, с помощью srec_cat.exe, добавляется размер 
прошивки и контрольная сумма.

Адреса размещения приложения: 0x0800С000

Адреса размещения информации в HEX прошивке приложения:
0x0800С01C - адрес размещения размера приложения (uint32_t).
0x0800С020 - адрес размещения контрольной суммы приложения (uint32_t).

При расчете и проверке контрольной суммы приложения адреса: 
0x0800С01C - 0x0800С023 исключены из расчета.

Параметры для приложение.
-------------------------------------------------------------------------------
1. В файле LinkerScript.ld, блок MEMORY, значение 
   "FLASH (rx): ORIGIN = 0x8000000, LENGTH = 512K"
   заменить на
   "FLASH (rx): ORIGIN = 0x800С000, LENGTH = 512K"
2. В файле system_stm32f1xx.c значение параметра VECT_TAB_OFFSET изменить
   на 0x0000C000U (смещение таблицы векторов прерываний)

Назначение портов.
-------------------------------------------------------------------------------
UART1, UART2 - 38400, 8N1

UART1 - вывод информации при работе приложения. 
UART2 - вывод информации при работе загрузчика.

LED1 - BLK - ожидание загрузки приложения в режиме DFU.
       
LED3 - BLK 1-2-3 - работа приложения.

Пример вывода информации загрузчиком при отсутствии приложения.
--------------------------------------------------------------------------------
Application address: 0x0800C000
Application size:    0x0800C01C => 0xFFFFFFFF (-1)
Application CRC:     0x0800C020 => 0xFFFFFFFF
Application not found.

DFU mode ...
Flash unlock ... OK

Пример вывода информации загрузчиком при наличии приложения.
--------------------------------------------------------------------------------
Application address: 0x0800C000
Application size:    0x0800C01C => 0x000030F4 (12532)
Application CRC:     0x0800C020 => 0xE51D79D0

Calculate CRC: 0x0800C000-0x0800C01B, 0x0800C024-0x0800F0D8 = 0xE51D79D0
Checksum - OK
Start application: 0x0800C8A5

Пример вывода информации загрузчиком при загразки приложения.
--------------------------------------------------------------------------------
Flash lock ... OK
Flash unlock ... OK
Get status flash.
Erase flash, address: 0x0800C000, pages: 0x0007 (7), page size: 2048 ... OK
Flash lock ... OK
Flash unlock ... OK
Get status flash.
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800C000: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800C800: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800D000: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800D800: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800E000: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800E800: Len: 0x0800
Get status flash.
Write flash: Src: 0x2000030C Dest: 0x0800F000: Len: 0x00F4

Пример вывода информации загрузчиком при верификации приложения.
--------------------------------------------------------------------------------
Flash lock ... OK
Flash unlock ... OK
Get status flash.
Read flash: 0x0800C000 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800C800 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800D000 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800D800 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800E000 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800E800 Dest: 0x2000030C: Len: 0x0800 (2048)
Read flash: 0x0800F000 Dest: 0x2000030C: Len: 0x00F4 (244)

