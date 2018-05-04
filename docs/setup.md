## CTF Setup 

First, you need an esp32 micro controller.  If you do not yet have one, I typically buy [these ones](https://www.amazon.com/HiLetgo%C2%AE-ESP-WROOM-32-Development-Microcontroller-Integrated/dp/B0718T232Z/ref=sr_1_4?ie=UTF8&qid=1525458705&sr=8-4&keywords=esp32) from Amazon.  If you are reading this because you know I will be at BSidesLV, BlackHat USA and DEFCON, you can ping me on Twitter and Ill hook you up with a pre-flashed on at co$t in Vegas.

Now, to get started, setup your esp32 build environment by following [this documentation](http://esp-idf.readthedocs.io/en/latest/get-started/#setup-toolchain).  Once complete, you can build and flash the code from this repository just the same as you would from the example bluetooth programs in that project which are located in ```/esp-idf/examples/bluetooth/```

If you need a reminder, do the following once you set up your whole esp build environment from the link above.
````
cd ble_ctf
make menuconfig   # set your serial device in bla -> bla -> bla
make
# plug in your esp32 to usb
make flash
````
