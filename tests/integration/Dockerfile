FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    siege \
    curl \
    lua5.4 \
    golang-go \
    && rm -rf /var/lib/apt/lists/*

# Copy test files
WORKDIR /tests
COPY *.go .
COPY *.lua .
COPY go.mod go.sum ./
COPY config/ config/
COPY data_to_send/ data_to_send/
COPY server_files/ server_files/
COPY hello.txt .
COPY siege_test.sh run_tests.sh .
RUN chmod +x run_tests.sh siege_test.sh

# Copy the webserver binary (built outside, passed in)
COPY webserv.out .

RUN go mod download

ENTRYPOINT ["/tests/run_tests.sh"]
