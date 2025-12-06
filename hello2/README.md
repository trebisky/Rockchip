This is my second bit of "bare metal" code for the Rockchip RK3399.

I differs from "hello" by splitting out the uart code into a
separate file uart.c and performing initialization rather than
just relying on the U-boot init.

Tom Trebisky  12-31-2021

This program prints "bullfrog" 500 times on the serial port.

Working fine 12-5-2025
