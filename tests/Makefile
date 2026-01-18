CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -lboost_system \
           -I../src

LDFLAGS = -L.. -lwebserv

TESTS = test_http_parser main

all: $(TESTS)

main: main.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

test_http_parser: test_http_parser.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TESTS)
