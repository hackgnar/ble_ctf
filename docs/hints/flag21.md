## Flag 21 Hint

Read handle 0x005a and it will tell you that you are in the wrong place.  It is
telling the truth.

A characteristic is more than its value.  Most tools list characteristics by
default and stop there, but every characteristic can carry descriptors hanging
off it, and those have handles of their own.  Try `gatttool --char-desc` instead
of `--characteristics` and see what turns up next to this one.
