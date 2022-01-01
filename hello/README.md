This is my first bit of "bare metal" code for the Rockchip RK3399.
It is loaded by U-boot and so is not truly "bare metal" and
in fact it relies on the uart initialization already done
by U-boot.  The goal is to get something (anything!) to run
with the least amount of code possible!

Tom Trebisky  12-31-2021

This program prints "hello" endlessly on the serial port.

The starting point for this was my Bitcoin Miner (EBAZ4205)
archive, project "fish".

This was derived from my Orange Pi Github archive, project "h5_hello".
