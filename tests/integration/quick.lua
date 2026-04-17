#!/usr/bin/env lua

-- quick script to test the basic workflow of the server:
-- POST -> GET -> DELETE -> GET (error)

local BASE_URL = "http://127.0.0.1:8085"

-- run a shell command and return its output
local function run(cmd)
    local handle = io.popen(cmd)
    local result = handle:read("*a")
    handle:close()
    return result
end

-- POST
print("=== POST ===")
local post_out = run(string.format(
    'curl -si -X POST %s/post -H "Content-Type: text/plain" --data-binary "hello world"',
    BASE_URL
))
print(post_out)

-- extract Location header
local location = post_out:match("Location: (/[^\r\n]+)")
if not location then
    print("FAIL: no Location header in POST response")
    os.exit(1)
end
print("Location: " .. location)

-- GET
print("=== GET (expect 200) ===")
local get_out = run(string.format('curl -si %s%s', BASE_URL, location))
print(get_out)
local get_status = get_out:match("HTTP/%d%.%d (%d+)")
if get_status ~= "200" then
    print("FAIL: expected 200, got " .. (get_status or "nil"))
    os.exit(1)
end
print("OK: got 200")

-- DELETE
print("=== DELETE ===")
local del_out = run(string.format('curl -si -X DELETE %s%s', BASE_URL, location))
print(del_out)
local del_status = del_out:match("HTTP/%d%.%d (%d+)")
if del_status ~= "204" and del_status ~= "200" then
    print("FAIL: expected 204/200, got " .. (del_status or "nil"))
    os.exit(1)
end
print("OK: deleted")

-- GET after delete (expect 404)
print("=== GET after DELETE (expect 404) ===")
local get2_out = run(string.format('curl -si %s%s', BASE_URL, location))
print(get2_out)
local get2_status = get2_out:match("HTTP/%d%.%d (%d+)")
if get2_status ~= "404" then
    print("FAIL: expected 404, got " .. (get2_status or "nil"))
    os.exit(1)
end
print("OK: got 404")

print("\n=== ALL TESTS PASSED ===")
