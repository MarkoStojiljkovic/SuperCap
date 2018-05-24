
/*
 After the FM24V10 (as receiver) acknowledges the slave
address, the master can place the memory address on the bus
for a write operation. The address requires a 1-bit page select
and two bytes. Since the device uses 17-bit address, the page
select bit is the MSB of the address followed by the remaining
16-bit address. The complete 17-bit address is latched internally.
Each access causes the latched address value to be incremented
automatically. The current address is the value that is
held in the latch; either a newly written value or the address
following the last access. The current address will be held for as
long as power remains or until a new value is written. Reads
always use the current address. A random read address can be
loaded by beginning a write operation as explained below.
After transmission of each data byte, just prior to the
acknowledge, the FM24V10 increments the internal address
latch. This allows the next sequential byte to be accessed with
no additional addressing. After the last address (1FFFFh) is
reached, the address latch will roll over to 00000h. There is no
limit to the number of bytes that can be accessed with a single
read or write operation.
 */


