[![Follow Hackgnar](static/twitter_hackgnar.png)](https://twitter.com/hackgnar)

## BLE Capture the Flag
The purpose of BLE CTF is to teach the core concepts of Bluetooth Low Energy client and server interactions.  While it has also been built to be fun, it was built with the intent to teach and reinforce core concepts that are needed to plunge into the world of Bluetooth hacking.  After completing this CTF, you should have everything you need to start fiddling with any BLE GATT device you can find.

## Setting Up the CTF
In order to set up the CTF you will need the following:
1. The pre-compiled firmware or source code in this repository to build and flash an ESP32 with the CTF GATT server.  
2. An esp32 microcontroller ([I sell overpriced pre-flashed ones here](https://www.ebay.com/itm/173370426012?ssPageName=STRK:MESELX:IT&_trksid=p3984.m1558.l2649))
3. A Linux box (OSX/Win + Linux VM works) with a bluetooth controller or a bluetooth usb dongle
4. Bluetooth tools such as Bluez tools (hcitool, gatttool, etc) or [bleah](https://github.com/evilsocket/bleah)

For instructions to compile/flash your own firmware or flash the provided pre-compiled firmware [read this documentation](docs/setup.md)

## Flags

### How to Submit Flags

Before you can submit flags, you have to discover the Bluetooth MAC address of your device.  Here are a couple example commands to help you find your device:

Discover MAC using hcitool:   
```` sudo hcitool lescan ````

Discover MAC using bleah:   
```` sudo bleah ````

Now that you have found your device’s MAC address, you can now communicate with it.  Before we get started with flags, let’s check out how we can see our current score.  In order to see where you are in the CTF, you can read from handle 42 on the device to see how many flags you have.  The following are example commands of how to view your current score.  Make sure you replace the MAC address in the example commands with the MAC address of your device. 

Show score with gatttool:  
```` gatttool -b de:ad:be:ef:be:f1 --char-read -a 0x002a|awk -F':' '{print $2}'|tr -d ' '|xxd -r -p;printf '\n'  ````

Show score with bleah:  
```` sudo bleah -b "30:ae:a4:20:79:da" -e ````

Ok, ok, ok, on to the flags! All flags are md5 sums truncated to 20 characters to avoid MTU limits by some hardware.  They can be submitted to the gatt server on handle 44.  The following are examples of how to submit a flag.  Make sure you replace the MAC address in the example commands with the MAC address of your device:   

Submit using gatttool:  
```` gatttool -b de:ad:be:ef:be:f1 --char-write-req -a 0x002c -n $(echo -n "some flag value"|xxd -ps) ````

Submit using bleah:  
```` sudo bleah -b "30:ae:a4:20:79:da" -n 0x002c -d "some flag value" ````

### Flag Hints
| Flag | Description | Hint |
| ------- | ----------------------------- | ------- |
| Flag 1 | This flag is a gift and can only be obtained from reading the hint! | [Read Me!](docs/hints/flag1.md) |
| Flag 0x002e | Learn how to read handles | [More](docs/hints/flag2.md) |
| Flag 0x0030 | Read handle puzzle fun | [More](docs/hints/flag3.md) |
| Flag 0x0016 | Learn about discoverable device attributes | [More](docs/hints/flag4.md) |
| Flag 0x0032 | Learn about reading and writing to handles | [More](docs/hints/flag5.md) |
| Flag 0x0034 | Learn about reading and writing ascii to handles | [More](docs/hints/flag6.md) |
| Flag 0x0036 | Learn about reading and writing hex to handles | [More](docs/hints/flag7.md) |
| Flag 0x0038 | Learn about reading and writing to handles differently | [More](docs/hints/flag8.md) |
| Flag 0x003c | Learn about write fuzzing | [More](docs/hints/flag9.md) |
| Flag 0x003e | Learn about read and write speeds | [More](docs/hints/flag10.md) |
| Flag 0x0040 | Learn about single response notifications | [More](docs/hints/flag11.md) |
| Flag 0x0042 | Learn about single response indicate | [More](docs/hints/flag12.md) |
| Flag 0x0046 | Learn about multi response notifications | [More](docs/hints/flag13.md) |
| Flag 0x0048 | Learn about multi response indicate | [More](docs/hints/flag14.md) |
| Flag 0x004c | Learn about BT client device attributes | [More](docs/hints/flag15.md) |
| Flag 0x004e | Learn about message sizes MTU | [More](docs/hints/flag16.md) |
| Flag 0x0050 | Learn about write responses | [More](docs/hints/flag17.md) |
| Flag 0x0052 | Hidden notify property | [More](docs/hints/flag18.md) |
| Flag 0x0054 | Use multiple handle properties | [More](docs/hints/flag19.md) |
| Flag 0x0056 | OSINT the author! | [More](docs/hints/flag20.md) |
