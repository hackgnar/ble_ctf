[![Follow Hackgnar](static/twitter_hackgnar.png)](https://twitter.com/hackgnar)

## BLE Capture the Flag v2.0
The purpose of BLE CTF is to teach the core concepts of Bluetooth Low Energy client and server interactions.  While it has also been built to be fun, it was built with the intent to teach and reinforce core concepts that are needed to plunge into the world of Bluetooth hacking.  After completing this CTF, you should have everything you need to start fiddling with any BLE GATT device you can find.

## Setting Up the CTF
In order to set up the CTF you will need the following:
1. The code in this repository to build the CTF GATT server
2. An esp32 microcontroller ([I sell overpriced pre-flashed ones here](https://www.ebay.com/itm/173370426012?ssPageName=STRK:MESELX:IT&_trksid=p3984.m1558.l2649))
3. A Linux box (OSX/Win + Linux VM works) with a bluetooth controller or a bluetooth usb dongle ([I ❤️ UD100s](https://www.amazon.com/Sena-UD100-Bluetooth-Class1-Adapter/dp/B01BHD7WR2/ref=cm_cr_arp_d_product_top?ie=UTF8))
4. Bluetooth tools such as Bluez tools (hcitool, gatttool, etc) or [bleah](https://github.com/evilsocket/bleah)

To get setup, [read this documentation](docs/setup.md)

## BLE CTF Architecture Overview

As of v2.0, the BLE CTF now hosts multiple GATT servers. As the ESP32 can only host one GATT server at a time, you must cycle though GATT servers in order to solve flags and submit flag solutions.  Documentation on v2.0 is still a WIP... Stay tuned... 

## Flags

As of version 2.0, flags can now only be submitted when your esp32 device is serving the "Score Dashboard" GATT server.  Documentation on v2.0 is still a WIP.... Stay tuned...

### How to Submit Flags
Documentation on v2.0 is still a WIP.... Stay tuned...

### Flag Hints
Documentation on v2.0 is still a WIP.... Stay tuned...

