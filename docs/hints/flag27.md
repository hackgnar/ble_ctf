## Flag 27 Hint

Read handle 0x0068 and it will tell you what phrase it wants.

That phrase is longer than a single ATT write can carry.  A plain write request
tops out at the negotiated MTU minus a few bytes of header, so the value has to
be queued up in pieces and then committed - prepare write, prepare write,
prepare write, execute.

Most clients will do this for you automatically if you just hand them the whole
string.  Watch it happen in `btmon` and you will see the queue.
