#!/bin/bash

# build
docker build -t webserv-siege .

# Linux — uses host network so container can reach 127.0.0.1
docker run --rm --network host webserv-siege
