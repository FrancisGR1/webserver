#!/bin/bash
set -e

echo "Starting webserver..."
./webserv.out config/default.conf > server.log 2>&1 &
SERVER_PID=$!
echo "Server pid: '$$'"
sleep 1

echo "=== Lua tests ==="
lua5.4 quick.lua
lua5.4 big.lua

echo "=== Go tests ==="
go test ./...

echo "=== Siege tests ==="
./siege_test.sh

echo "All tests done."
