RC to Pi
====
I needed a way to control a GoPro via RC receiver.  GoPro can communicate via Wifi and web services to a Raspberry Pi device.  So the Raspberry Pi can communicate with GoPro HERO4 via python calls to GoPro's services.  I used a ATTiny85 device connected to the GPIO pins and a couple of RC channels via PWM pins on a receiver.  This allowed me to signal the Pi device to execute web service commands to the GoPro.

This repo consists of the Arduino C code for the ATTiny85 and the python code running in a loop on the Rasberry Pi.

It should be fairly straight forward from there.
