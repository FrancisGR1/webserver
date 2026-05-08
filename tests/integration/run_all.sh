#!/bin/bash
set -e

# start the webserver in background
./webserv.out config/... &
SERVER_PID=$!
echo "Server pid: '$$'"
sleep 1  # give it time to start

lua5.4 quick.lua
lua5.4 big.lua
go test ./...
./stress/siege_test.sh
