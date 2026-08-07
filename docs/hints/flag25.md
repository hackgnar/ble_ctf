## Flag 25 Hint

Every other characteristic on this device uses a short 16 bit UUID.  Exactly one
uses a full 128 bit UUID, and it is deliberately sitting at a handle you have no
reason to guess.

Enumerate the characteristics and look for the odd one out.  This is the flag
you cannot solve by copying a handle number out of someone else's writeup.

Reading it is not enough - it will only tell you it wants a write.  Write
anything to it, then read it again.
