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
	  src/config/Config.cpp \
	  src/config/ConfigLexer.cpp \
	  src/config/ConfigParser.cpp \
	  src/config/ConfigTypes.cpp \
	  src/http/HttpRequest.cpp \
	  src/http/HttpRequestParser.cpp \
	  src/http/HttpResponse.cpp \
	  src/http/StatusCode.cpp \
	  src/http/CgiHandler.cpp \
	  src/core/Logger.cpp \
	  src/core/MimeTypes.cpp \
	  src/core/Path.cpp \
	  src/core/constants.cpp \
	  src/core/utils.cpp \
	  src/server/Webserver.cpp \
	  src/server/Connection.cpp \
	  src/server/EventManager.cpp


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
