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
esptool.py --chip esp32 --port /dev/ttyUSB0 \
--baud 115200 --before default_reset --after hard_reset write_flash \
-z --flash_mode dio --flash_freq 40m --flash_size detect \
0x1000 build/bootloader/bootloader.bin \
0x10000 build/gatt_server_service_table_demo.bin \
0x8000 build/partitions_singleapp.bin
```

## Build From Docker

If you want to compile the code yourself, but are having issues setting up an environment, you can use the docker build method.  This will provide you with a clean uniform build environment each time.  To build from docker do the following:

Build your base docker image and compile the code
```
docker build -t blectf:v1 .
```

Start up a docker instance to pull out the binaries you compiled
```
docker run blectf:v1
```

Copy the build from your docker instance
```
docker cp <instance_name>:/ble_ctf/build .
```

Shutdown and kill your docker
```
docker stop <instance_name>
docker rm <instance_name>
```

Flash the firmware you built (you will need [esptool](https://github.com/espressif/esptool) installed)
```
esptool.py --chip esp32 --port /dev/ttyUSB0 \
--baud 115200 --before default_reset --after hard_reset write_flash \
-z --flash_mode dio --flash_freq 40m --flash_size detect \
0x1000 build/bootloader/bootloader.bin \
0x10000 build/gatt_server_service_table_demo.bin \
0x8000 build/partitions_singleapp.bin
```

## Build From Source

Setup your esp32 build environment by following [this documentation](http://esp-idf.readthedocs.io/en/latest/get-started/#setup-toolchain).  Once complete, you can build and flash the code from this repository just the same as you would from the example bluetooth programs in that project which are located in ```/esp-idf/examples/bluetooth/```

If you need a reminder, do the following once you set up your whole esp build environment from the link above.
````
cd ble_ctf
make menuconfig   # set your serial device in bla -> bla -> bla
make
# plug in your esp32 to usb
make flash
````
