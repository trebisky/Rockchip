This is my second "truly bare metal" project for the Rockchip RK3399.

It is truly bare metal, because it does not rely on U-boot to get
loaded (via network tftp).  This gets placed onto an SD card and
gets started directly by the bootrom.

Now that I have the bare_blink demo to work, I would like to emit
messages via the console uart.  This was easy when I was being
loaded from U-Boot because the uart initialization had already
been done.  Here I need to do all the initialization (or at least
I think I do).

Tom Trebisky  1-18-2022

