## Flag 28 Hint

Write anything to handle 0x006a and listen.  You will get five notifications
back rather than one.

Do not simply glue them together in the order they arrive.  Notifications are
not ordered or guaranteed, and this server deliberately sends them out of
sequence with random delays between them.  Look at the first character of each
one - that is its position.  Sort by it, strip it (i.e. remove the first/ordering char of each notification), then concatenate.
