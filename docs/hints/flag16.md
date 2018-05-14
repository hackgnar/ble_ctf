## Flag 16 Hint

Read handle 0x0048 and do what it says.  Setting MTU can be a tricky thing.  Some tools may provide mtu flags, but they dont seem to really trigger MTU negotiations on servers.  Try using gatttool's interactive mode for this task.  By default, the BLECTF server is set to force an MTU size of 20.  The server will listen for MTU negotiations, and look at them, but we dont really change the MTU in the code.  We just trigger the flag code if you trigger an MTU event with the value specified in handle 0x0048.  GLHF!
