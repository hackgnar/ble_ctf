## Flag 1 Hint

Flag one is a gift!  You can only obtain it by reading this document or peaking at the source code.  In short, this flag is to get you familiar with doing a simple write to a BLE handle.  Do the following to get your first flag.  Make sure you replace the MAC address in the examples below with your devices mac address!

First, check out your score:   
```` gatttool -b 30:ae:a4:20:79:da --char-read -a 12 ````

Next, lets sumbmit the following flag.
```` gatttool -b 30:ae:a4:20:79:da --char-write-req -a 0x002f -n 0x12345678901234567890 ````

Finaly, check out your score again to see your flag got accepted:   
```` gatttool -b 30:ae:a4:20:79:da --char-read -a 12 ````
