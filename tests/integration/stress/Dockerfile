FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    siege \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY siege_test.sh /siege_test.sh
RUN chmod +x /siege_test.sh

ENTRYPOINT ["/siege_test.sh"]
