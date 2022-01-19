This is my first "truly bare metal" project for the Rockchip RK3399.

It is truly bare metal, because it does not rely on U-boot to get
loaded (via network tftp).  This gets placed onto an SD card and
gets started directly by the bootrom.

The idea is for this to be the simplest possible code to just
confirm that the whole process works.
I have already debugged a U-boot launched "blink" and that
was the starting point for this.

The onboard LED is green and is connected to "GPIO0_B3"

Tom Trebisky  1-18-2022

