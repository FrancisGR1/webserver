SERVER      = ../../webserv
LIB         = ../../libwebserv.a
CXX         = c++ -g
CXXFLAGS    = -Wall -Werror -Wextra -std=c++23 -I../../src/

# Add more test binaries here
TESTS       = test_resolve_target.out \
	      test_RequestParser.out \
	      test_GetHandler.out \
	      test_CgiHandler.out

.PHONY: all tests clean

all: $(SERVER) $(LIB) $(TESTS)

tests: $(TESTS)
	@for t in $(TESTS); do \
		echo "Running $$t..."; \
		./$$t; \
	done

$(SERVER):
	$(MAKE) -C ../../

$(LIB): $(SERVER)
	$(MAKE) -C ../../ lib

test_%.out: test_%.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LIB) -o $@

clean:
	rm -f $(TESTS)

fclean:
	$(MAKE) -C ../../ fclean

re: clean fclean all $(TESTS)
