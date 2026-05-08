import sys
import os

chunk = sys.stdin.buffer.read(4096)
sys.stdout.buffer.write(b"Content-type: text/html\r\n\r\n")
while chunk:
    sys.stdout.buffer.write(chunk)
    sys.stdout.buffer.flush()
    chunk = sys.stdin.buffer.read(4096)
sys.stdout.buffer.flush()
