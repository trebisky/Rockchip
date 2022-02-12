usb_load

This is a linux side program that can download binaries to the RK3399
via a USB cable.  It talks to the bare bootrom (sometimes called "MASKROM").

I learned enough to write this by studying the disassembled bootrom code along
with the rkdeveloptool sources.  A person could use rkdeveloptool for this,
but this is much cleaner, simpler, and handier.  Besides that, writing it was
a good chance for me to put to work what I had learned.

unpack

"Unpack" is another linux side tool.  I does what the "unpack" option in rkdeveloptool
does.  I wrote it before I found and fixed a bug in rkdeveloptool, and once again
it had educational value for me.  What it is all about is that you will sometimes
find "boot files" that are composite items intended to be fed to rkdeveloptool, and
who knows what else.  These typically contain 4 "entries" with the first being a
DDR loader, intended to run in SRAM, the second a fancier usb loader intended to
run in DDR ram, followed by a couple of other things (the first of which is yet
another loader) intended to be installed onto SD or eMMC.  Such a composite file
begins with the letters "BOOT" and has a header, followed by as many "entry headers",
followed by the entries themselves (which are RC4 encrypted in the file).
This is hardly a convenient package for anything I care to do, hence this tool.

Tom Trebisky  2-11-2022

