#!/bin/bash
set -e

HOST=${HOST:-"localhost"}
PORT=${PORT:-8091}
DURATION=${DURATION:-30s}
CONCURRENCY=${CONCURRENCY:-10}

BASE="http://$HOST:$PORT"

echo "=============================="
echo "Testing $BASE"
echo "=============================="

echo -e "\n--- GET static ---"
siege -c $CONCURRENCY -t $DURATION -b "$BASE/listing" 2>&1

echo -e "\n--- POST upload ---"
siege -c $CONCURRENCY -t $DURATION -b \
    --content-type "application/octet-stream" \
    "$BASE/post POST hello" 2>&1

echo -e "\n--- CGI ---"
siege -c $CONCURRENCY -t $DURATION -b "$BASE/scripts/good/hello.py" 2>&1
