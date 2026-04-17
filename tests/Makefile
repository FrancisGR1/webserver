SERVER      = webserv.out # @MUST be the name of executable
LIB         = ../libwebserv.a
CXX         = c++ -g
CXXFLAGS    = -Wall -Werror -Wextra -std=c++23 -I../src/

# Unit test binaries
# Add here
UNITS       = unit/test_resolve_target.out \
	      unit/test_RequestParser.out \
	      unit/test_GetHandler.out \
	      unit/test_PostHandler.out \
	      unit/test_DeleteHandler.out \
	      unit/test_ErrorHandler.out \
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
	@echo "\nMakefile: Starting webserv for integration tests..."
	@echo "Makefile: Server logs -> integration/webserv.log"; \
	cd ./integration; \
	./$(SERVER) > webserv.log 2>&1 & \
	WEBSERV_PID=$$!; \
	sleep 1; \
	echo $${PWD}; \
	echo "Makefile: Running lua quick.lua..."; \
	lua quick.lua; \
	echo "Makefile: Running go test..."; \
	go test; \
	kill $$WEBSERV_PID 2>/dev/null; \
	wait $$WEBSERV_PID 2>/dev/null; \
	echo "Makefile: webserv stopped."

tests: unit integration

$(SERVER):
	$(MAKE) -C ../
	cp ../$(SERVER) integration/

$(LIB): $(SERVER)
	$(MAKE) -C ../ lib

unit/test_%.out: unit/test_%.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $< $(LIB) -o $@

clean:
	rm -f $(UNITS)

fclean: clean

re: clean fclean all tests
