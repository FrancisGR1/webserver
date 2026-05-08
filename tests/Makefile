SERVER      = webserv.out # @MUST be the name of executable
LIB         = ../libwebserv.a
CXX         = c++ -g
CXXFLAGS    = -Wall -Werror -Wextra -std=c++20 -I../src/

# Unit test binaries
# Add here
UNITS       = unit/test_resolve_target.out \
	      unit/test_RequestParser.out \
	      unit/test_GetHandler.out \
	      unit/test_DeleteHandler.out \
	      unit/test_ErrorHandler.out \
	      unit/test_PostHandler.out \
	      unit/test_CgiHandler.out

.PHONY: all unit integration clean fclean re

all: $(SERVER) $(LIB) $(UNITS)

unit: $(UNITS)
	@echo "\t=============="
	@echo "\t==== UNIT ===="
	@echo "\t=============="
	@for t in $(UNITS); do \
		echo "\nMakefile: Running $$t..."; \
		(cd unit && ./`basename $$t`); \
	done

integration: ../$(SERVER)
	cp ../$(SERVER) integration/$(SERVER)
	cd integration && ./build_and_run.sh

tests: unit integration

$(SERVER):
	$(MAKE) -C ../
	@if [ ! -f integration/$(SERVER) ]; then \
		cp ../$(SERVER) integration/; \
	fi

$(LIB): $(SERVER)
	$(MAKE) -C ../ lib

unit/test_%.out: unit/test_%.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LIB) -o $@

clean_garbage:
	@find ./integration -name "*.data" -delete
	@echo "Cleaning every '*.data' file in ./integration"

clean: clean_garbage
	rm -f $(UNITS)

fclean: clean

re: clean fclean all tests
