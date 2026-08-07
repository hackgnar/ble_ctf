## Flag 24 Hint

Try to read handle 0x0061.  It fails.

Do not stop there.  A failed ATT operation carries an error code, and error
codes above 0x7f are application defined - the server chose that number
deliberately.  Find out what it actually returned rather than just seeing that
it went wrong.  `btmon` will show you the raw error in the ATT response.

Then write that code back to the same handle and read it again.
