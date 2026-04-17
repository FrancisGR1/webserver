NAME     = webserv
LIB      = libwebserv.a

CC       = c++
OBJ_DIR  = obj
VERSION  = -std=c++98
CFLAGS   = -Werror -Wall -Wextra
DEBUG    = -g
OPTIMIZE = -O3
INC      = -Isrc
FLAGS    = $(VERSION) $(CFLAGS) $(INC) $(DEBUG)

SOURCES  = \
	   src/main.cpp \
	  \
	  src/core/Logger.cpp \
	  src/core/MimeTypes.cpp \
	  src/core/Path.cpp \
	  src/core/Timer.cpp \
	  src/core/constants.cpp \
	  src/core/utils.cpp \
	  \
	  src/server/Connection.cpp \
	  src/server/ConnectionPool.cpp \
	  src/server/EventAction.cpp \
	  src/server/EventManager.cpp \
	  src/server/Socket.cpp \
	  src/server/Webserver.cpp \
	  \
	  src/config/Config.cpp \
	  src/config/ConfigTypes.cpp \
	  src/config/parser/ConfigLexer.cpp \
	  src/config/parser/ConfigParser.cpp \
	  src/config/parser/Token.cpp \
	  src/config/types/Directive.cpp \
	  src/config/types/Listener.cpp \
	  src/config/types/LocationConfig.cpp \
	  src/config/types/Route.cpp \
	  src/config/types/ServiceConfig.cpp \
	  \
	  src/http/StatusCode.cpp \
	  src/http/http_utils.cpp \
	  \
	  src/http/request/Request.cpp \
	  src/http/request/RequestParser.cpp \
	  \
	  src/http/processor/RequestConfig.cpp \
	  src/http/processor/RequestContext.cpp \
	  src/http/processor/RequestProcessor.cpp \
	  \
	  src/http/processor/handler/CgiHandler.cpp \
	  src/http/processor/handler/DeleteHandler.cpp \
	  src/http/processor/handler/ErrorHandler.cpp \
	  src/http/processor/handler/GetHandler.cpp \
	  src/http/processor/handler/IRequestHandler.cpp \
	  src/http/processor/handler/PostHandler.cpp \
	  \
	  src/http/response/Response.cpp \
	  src/http/response/ResponseError.cpp

OBJ      = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp, $(SOURCES)))
OBJ     += $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out src/%.cpp, $(SOURCES)))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

$(LIB): $(filter-out $(OBJ_DIR)/main.o,$(OBJ))
	ar rcs $@ $^

lib: $(LIB)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	rm -f $(LIB)
	cd $(TESTS_DIR) && $(MAKE) -C unit/ fclean

re: fclean all

TESTS_DIR = tests/
.PHONY: tests
tests: all lib
	cd $(TESTS_DIR) && $(MAKE) -C unit/
	cd $(TESTS_DIR) && $(MAKE) tests -C unit/
