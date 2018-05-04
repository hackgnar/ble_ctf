## BLE Capture the Flag
some info about the project....

## Flags

### How to Submit Flags

Before you can submit flags, you have to discover the Bluetooth mac address of your device.  Use some of the following commands to find your device:

Discover MAC using hcitool:   
```` sudo hcitool blescan ````

Discover MAC using bleah:   
```` sudo bleah ````

Now that you have found your devices MAC address, you can now submit flags to it.  All flags are md5 sums trunkated to 20 characters to avoid MTU limits by some hardware.  They can be submited to the gatt server on handle 42.  The following are examples of how to submit a flag.  Make sure you replace the MAC address in the example commands with the MAC address of your device:   

Submit using gatttool:  
```` gatttool -b 30:ae:a4:20:79:da --char-write-req -a 0x002f -n 0x0001 ````

Submit using bleah:  
```` sudo bleah -b "30:ae:a4:20:79:da" -u "0000ff03-0000-1000-8000-00805f9b34fb" -d "hello world" ````

### Flag Hints
| Flag | Description | Hint |
| ------- | ----------------------------- | ------- |
| Flag 1 | This flag is a gift and can \nonly be obtained from reading the hint! | Read Me! |
| Flag 2 | | |
| Flag 3 | | |
| Flag 4 | | |
| Flag 5 | | |
| Flag 6 | | |
| Flag 7 | | |
| Flag 8 | | |
| Flag 9 | | |
| Flag 10 | | |
| Flag 11 | | |
| Flag 12 | | |
| Flag 13 | | |
| Flag 14 | | |
| Flag 15 | | |
| Flag 16 | | |
| Flag 17 | | |
| Flag 18 | | |
| Flag 19 | | |
| Flag 20 | | |
