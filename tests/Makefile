CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -lboost_system \
           -I../src
LIB=../libwebserv.a

LDFLAGS = -L.. -lwebserv

TESTS = quick #test_http_parser main

all: $(TESTS) lib
	@echo $(TESTS)
	@echo $(LIB)

lib: $(LIB)
	cd .. && $(MAKE) lib

quick: quick_tests_main.cpp lib
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

main: main.cpp lib
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

test_http_parser: test_http_parser.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TESTS)
