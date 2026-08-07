## Flag 26 Hint

Read handle 0x0065.  Writing to it does nothing at all, no matter what you send.

Descriptors are not read only decoration.  This characteristic has one, it is
writable, and reading it will tell you what word it wants.  Find it with
`gatttool --char-desc`, then write to the descriptor's handle rather than the
characteristic's.

Flag 21 taught you to read a descriptor.  This one is the other half.
