import sys

sys.stdout.write("Status: 999 Whatever\r\n")
sys.stdout.write("Content-Type: text/plain\r\n")
sys.stdout.write("\r\n")
sys.stdout.write("This is the body\r\n")
