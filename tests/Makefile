CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -lboost_system \
           -I../src
LIB=../libwebserv.a

LDFLAGS = -L.. -lwebserv

TESTS = quick #test_http_parser main

all: $(TESTS) $(LIB)

$(LIB):
	cd .. && $(MAKE) lib

lib: $(LIB)

quick: quick_tests_main.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

main: main.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

test_http_parser: test_http_parser.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TESTS)
