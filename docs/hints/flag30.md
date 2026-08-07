## Flag 30 Hint

Read handle 0x006f.  You get a random value.  Write that exact value straight
back to the same handle, then read it once more - the second read is your flag.

The catch is that the value is freshly generated for every connection.  There is
no static answer to this one, not in any writeup and not from asking a chatbot.
Whatever you read has to be written back within the same connection, so if your
tool opens a new connection for every command the value will have changed
underneath you before your write lands.  Use interactive mode to hold a single
connection open for all three steps.

A wrong write is harmless - the handle just goes back to offering the value
again.
