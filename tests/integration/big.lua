#!/usr/bin/env lua

local BASE_URL = "http://127.0.0.1:8091"
local SMALL_TEST_FILE = "./data_to_send/10_bytes.txt"
local TEST_FILE = "./data_to_send/10k_bytes.txt"
local BIG_TEST_FILE = "./data_to_send/pdf_file.pdf"
local CGI_SCRIPT = "./scripts/good/echo_stdin.py"

print("=== BIG PDF FILE ===")
-- POST
print("=== POST ===")
local post_cmd = string.format(
    'curl -s -X POST "%s/post" --data-binary @"%s" -H "Content-Type: application/octet-stream" -H "Expect:" -D /tmp/headers.txt -o /dev/null',
    BASE_URL, TEST_FILE
)
print("cmd: " .. post_cmd)
os.execute(post_cmd)

-- extract location from headers
local location = nil
for line in io.lines("/tmp/headers.txt") do
    local loc = line:match("^[Ll]ocation:%s*(.-)%s*$")
    if loc then
        location = loc
        break
    end
end

if not location then
    print("FAIL - no Location header")
    os.exit(1)
end
print("Location: " .. location)

-- GET
print("=== GET ===")
local get_cmd = string.format('curl -s "%s%s" -o /tmp/received.pdf', BASE_URL, location)
print("cmd: " .. get_cmd)
os.execute(get_cmd)

-- DIFF
print("=== DIFF ===")
local diff_cmd = string.format('diff -q "%s" /tmp/received.pdf > /dev/null', TEST_FILE)
print("cmd: " .. diff_cmd)
local diff = os.execute(diff_cmd)
if diff == true or diff == 0 then
    print("OK - files are identical")
else
    print("FAIL - files differ")
    os.execute(string.format('cmp "%s" /tmp/received.pdf', TEST_FILE))
    os.exit(1)
end

-- POST to CGI script
local target_file = BIG_TEST_FILE

print("=== POST to CGI ===")
local post_to_cgi_cmd = string.format(
    'curl -s -X POST "%s/%s" --data-binary @"%s" -H "Content-Type: application/octet-stream" -H "Expect:" -o /tmp/cgi_response.pdf',
    BASE_URL, CGI_SCRIPT, target_file
)
print("cmd: " .. post_to_cgi_cmd)
os.execute(post_to_cgi_cmd)

-- DIFF
print("=== DIFF ===")
diff = os.execute(string.format('diff -q "%s" /tmp/cgi_response.pdf > /dev/null', target_file))
if diff == true or diff == 0 then
    print("OK - CGI echoed file correctly")
else
    print("FAIL - CGI response differs from original")
    os.execute(string.format('cmp "%s" /tmp/cgi_response.pdf', target_file))
    os.exit(1)
end
