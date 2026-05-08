SERVER      = webserv.out # @MUST be the name of executable
LIB         = ../libwebserv.a
CXX         = c++ -g
CXXFLAGS    = -Wall -Werror -Wextra -std=c++23 -I../src/

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

integration: $(SERVER)
	@echo "\t======================="
	@echo "\t===== INTEGRATION ====="
	@echo "\t======================="
	@cd ./integration; \
	echo "Makefile: Please start the test server inside ./integration:"; \
	echo ""; \
	echo "    cd integration && ./$(SERVER)"; \
	echo ""; \
	read -p "Press Enter when the server is ready..." dummy; \
	echo "Makefile: Running lua quick.lua..."; \
	lua quick.lua; \
	echo "Makefile: Running lua big.lua..."; \
	lua big.lua; \
	echo "Makefile: Running go test..."; \
	go test; \
	echo "Makefile: Running siege..."; \
	./stress/run.sh; \
	echo "Makefile: All tests done. You can stop the server now."

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

clean:
	rm -f $(UNITS)

fclean: clean

re: clean fclean all tests
