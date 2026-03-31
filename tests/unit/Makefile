SERVER      = ../../webserv
LIB         = ../../libwebserv.a
CXX         = c++
CXXFLAGS    = -Wall -Werror -Wextra -std=c++23 -I../../src/

# Add more test binaries here
TESTS       = test_resolve_target \
	      test_RequestParser

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

# Pattern rule — each test_*.cpp compiles into its own binary
test_%: test_%.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LIB) -o $@

clean:
	rm -f $(TESTS)

fclean:
	$(MAKE) -C ../../ fclean

re: clean fclean all $(TESTS)
