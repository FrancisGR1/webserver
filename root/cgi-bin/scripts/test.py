#!/usr/bin/env python3
import os
import sys

# Use \r\n explicitly so your parser doesn't need the \n\n fallback
sys.stdout.write("Status: 200\r\n")
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")  # explicit header terminator
sys.stdout.flush()

# body
sys.stdout.write("<html><body>\r\n")
sys.stdout.write("<h1>CGI works</h1>\r\n")

# env dump — verifies your init_env()
sys.stdout.write("<h2>Environment</h2><pre>\r\n")
for key, val in sorted(os.environ.items()):
        sys.stdout.write("{} = {}\r\n".format(key, val))

        sys.stdout.write("</pre>\r\n")

        # stdin dump — verifies POST later
        sys.stdout.write("<h2>Stdin</h2><pre>\r\n")
        try:
            content_length = int(os.environ.get("CONTENT_LENGTH") or 0)
            if content_length > 0:
                body = sys.stdin.read(content_length)
                sys.stdout.write(body)
        except Exception as e:
            sys.stdout.write("stdin error: {}\r\n".format(str(e)))
            sys.stdout.write("</pre>\r\n")
            sys.stdout.write("</body></html>\r\n")
            sys.stdout.flush()
