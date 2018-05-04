## BLE Capture the Flag
some info about the project....

## Flags

### How to Submit Flags

Before you can submit flags, you have to discover the Bluetooth mac address of your device.  Use some of the following commands to find your device:

Discover MAC using hcitool:   
```` sudo hcitool blescan ````

Discover MAC using bleah:   
```` sudo bleah ````

Now that you have found your devices MAC address, you can now talk to it.  Before we get started with flags, lets check out how we can see our current score.  In order to see where you are in the CTF, you can read from handle 12 on the device to see how many flags you have.  The following are examples of how to view your current score.  Make sure you replace the MAC address in the example commands with the MAC address of your device. 

Show score with gatttool:  
```` gatttool -b 30:ae:a4:20:79:da --char-read -a 12  ````

Show score with bleah:  
```` sudo bleah -b "30:ae:a4:20:79:da" -h 12 ````

Ok, ok, ok, on with the flags.  All flags are md5 sums trunkated to 20 characters to avoid MTU limits by some hardware.  They can be submited to the gatt server on handle 42.  The following are examples of how to submit a flag.  Make sure you replace the MAC address in the example commands with the MAC address of your device:   

Submit using gatttool:  
```` gatttool -b 30:ae:a4:20:79:da --char-write-req -a 0x002f -n 0x0001 ````

Submit using bleah:  
```` sudo bleah -b "30:ae:a4:20:79:da" -u "0000ff03-0000-1000-8000-00805f9b34fb" -d "hello world" ````

### Flag Hints
| Flag | Description | Hint |
| ------- | ----------------------------- | ------- |
| Flag 1 | This flag is a gift and can only be obtained from reading the hint! | Read Me! |
| Flag 2 | Learn about discoverable device attributes | More |
| Flag 3 | Learn how to read handles | More |
| Flag 4 | Read handle puzzle fun | More |
| Flag 5 | Learn about BT client device attributes | More |
| Flag 6 | Learn about connection attributes | More |
| Flag 7 | Learn about reading and writing to handles | More |
| Flag 8 | Learn about brute write fuzzing | More |
| Flag 9 | Learn about notifications | More |
| Flag 10 | Learn about BT broadcast messages | More |
| Flag 11 | Learn about responses | More |
| Flag 12 | Learn about connection security attributes | More |
| Flag 13 | Learn about indications | More |
| Flag 14 | Handle puzzle fun | More |
| Flag 15 | Hard handle puzzle fun | More |
| Flag 16 | Harder handle puzzle fun | More |
| Flag 17 | Harderer handle puzzle fun | More |
| Flag 18 | Abuse me | More |
| Flag 19 | Exaust me | More |
| Flag 20 | Not gonna tell! | More |
