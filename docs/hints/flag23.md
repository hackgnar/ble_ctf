## Flag 23 Hint

Read handle 0x005f and it will point you somewhere that is not GATT at all.
This flag never appears in any characteristic - you do not even need to connect
to the device to get it.

A peripheral can put arbitrary bytes in its advertising data, and it can put
even more in a scan response.  Watch the air with `btmon` in one terminal while
you scan in another, and look for the manufacturer specific data.

Two things that trip people up:

* A passive scan never asks for a scan response, so it will never see this.
  `hcitool lescan --passive` gets you nothing; drop the flag and it appears.
* The flag is already a hex string, so a raw byte dump shows you hex of hex.
  Decode it once, not twice.
