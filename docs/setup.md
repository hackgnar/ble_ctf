## CTF Setup 

First, you need an esp32 micro controller.  If you do not yet have one, I typically buy [these ones](https://www.amazon.com/HiLetgo%C2%AE-ESP-WROOM-32-Development-Microcontroller-Integrated/dp/B0718T232Z/ref=sr_1_4?ie=UTF8&qid=1525458705&sr=8-4&keywords=esp32) from Amazon.  If you are reading this because you know I will be at BSidesLV, BlackHat USA and DEFCON, you can ping me on Twitter and Ill hook you up with a pre-flashed on at co$t in Vegas.

## Precompiled Binaries

The easiest way to get this projects firmware flashed to an ESP32 is to flash the provided pre-compiled binaries. You will need to install [esptool](https://github.com/espressif/esptool)  Then  do the following:
Clone the repository
```
git clone https://github.com/hackgnar/ble_ctf
```


Chage directory into the repository
```
cd ble_ctf
```

Flash the pre-compiled binaries
```
esptool.py -p (PORT) -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/ble_ctf.bin
```

## Build From Docker

If you want to compile the code yourself, but are having issues setting up an environment, you can use the docker build method.  This will provide you with a clean uniform build environment each time.  To build from docker do the following:

Build your base docker image and compile the code
```
docker build -t blectf .
```

Start up a docker instance to pull out the binaries you compiled
```
docker run -it -v ./:/ble_ctf --name blectf blectf
```

Setup and build from your docker instance. Make sure to enable bluetooth in your menuconfig (Component config -> Bleutooth).
```
cd /ble_ctf
idf.py set-target esp32
idf.py menuconfig
idf.py build
```

Shutdown and kill your docker
```
exit
docker stop blectf
docker rm blectf
```

Flash the firmware you built (you will need [esptool](https://github.com/espressif/esptool) installed)
```
esptool.py -p (PORT) -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/ble_ctf.bin
```

## Build From Source

Setup your esp32 build environment by following [this documentation](http://esp-idf.readthedocs.io/en/latest/get-started/#setup-toolchain).  Once complete, you can build and flash the code from this repository just the same as you would from the example bluetooth programs in that project which are located in ```/esp-idf/examples/bluetooth/```

If you need a reminder, do the following once you set up your whole esp build environment from the link above.
````
cd ble_ctf
idf.py set-target esp32  # set your serial device in bla -> bla -> bla
idf.py menuconfig
idf.py build
idf.py flash
````
