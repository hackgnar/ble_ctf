## Flag 1 Hint

Flag one is a gift!  You can only obtain it by reading this document or peaking at the source code.  In short, this flag is to get you familiar with doing a simple write to a BLE handle.  Do the following to get your first flag.  Make sure you replace the MAC address in the examples below with your devices mac address!

First, check out your score:   
```` gatttool -b de:ad:be:ef:be:f1 --char-read -a 0x002a|awk -F':' '{print $2}'|tr -d ' '|xxd -r -p;printf '\n' ````

Next, lets sumbmit the following flag.
```` gatttool -b de:ad:be:ef:be:f1 --char-write-req -a 0x002c -n $(echo -n "12345678901234567890"|xxd -ps) ````

Finaly, check out your score again to see your flag got accepted:   
```` gatttool -b de:ad:be:ef:be:f1 --char-read -a 0x002a|awk -F':' '{print $2}'|tr -d ' '|xxd -r -p;printf '\n' ````
