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
	  src/core/constants.cpp \
	  src/core/utils.cpp \
	  \
	  src/config/Config.cpp \
	  src/config/ConfigTypes.cpp \
	  src/config/types/ServiceConfig.cpp \
	  src/config/types/LocationConfig.cpp \
	  src/config/types/Route.cpp \
	  src/config/parser/ConfigLexer.cpp \
	  src/config/parser/ConfigParser.cpp \
	  src/config/parser/Token.cpp \
	  \
	  src/http/HttpRequest.cpp \
	  src/http/RequestDispatcher.cpp \
	  src/http/HttpRequestConfig.cpp \
	  src/http/HttpRequestContext.cpp \
	  src/http/HttpRequestParser.cpp \
	  src/http/HttpResponse.cpp \
	  src/http/NewHttpResponse.cpp \
	  src/http/HttpResponseException.cpp \
	  src/http/StatusCode.cpp \
	  src/http/AMethodHandler.cpp \
	  src/http/PostHandler.cpp \
	  src/http/GetHandler.cpp \
	  src/http/DeleteHandler.cpp \
	  src/http/ErrorHandler.cpp \
	  src/http/CgiHandler.cpp \
	  src/http/http_utils.cpp

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

re: fclean all

#watch:
#implementar: https://stackoverflow.com/questions/7539563/is-there-a-smarter-alternative-to-watch-make

TESTS_DIR = tests/
.PHONY: tests
tests:
	cd $(TESTS_DIR)
	./run_tests #TODO: mudar para um makefile próprio
