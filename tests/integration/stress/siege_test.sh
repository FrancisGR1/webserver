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
siege -c $CONCURRENCY -t $DURATION -b "$BASE/listing"

echo -e "\n--- POST upload ---"
siege -c $CONCURRENCY -t $DURATION -b \
    --content-type "application/octet-stream" \
    "$BASE/post POST hello"

echo -e "\n--- CGI ---"
siege -c $CONCURRENCY -t $DURATION -b "$BASE/scripts/good/hello.py"

echo -e "\n--- Redirection ---"
siege -c $CONCURRENCY -t $DURATION -b "$BASE/redir"
