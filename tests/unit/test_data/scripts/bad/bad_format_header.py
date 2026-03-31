import sys

sys.stdout.write("Content-Type text/plain\r\n")  # missing colon
sys.stdout.write("Status 200 OK\r\n")            # missing colon
sys.stdout.write("\r\n")
sys.stdout.write("This is the body\r\n")
