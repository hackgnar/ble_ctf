## Flag 22 Hint

Read handle 0x005d and it will tell you exactly what to hash.

A connection has two ends.  The server can see the address you connected from,
and that is what it wants an md5 of.  Find your own adapter's address with
`hciconfig` or `bluetoothctl list`, format it lowercase with colons, and md5 it.

Note that everyone's answer to this one is different.
