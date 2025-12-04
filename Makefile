NAME     = webserv

CC       = c++
OBJ_DIR  = obj
VERSION  = -std=c++98
CFLAGS   = -Werror -Wall -Wextra
DEBUG    = -g
OPTIMIZE = -O3
FLAGS    = $(VERSION) $(CFLAGS) $(DEBUG)

SOURCES  = \
	  src/Config.cpp \
	  src/ConfigLexer.cpp \
	  src/ConfigParser.cpp \
	  src/ConfigTypes.cpp \
	  src/constants.cpp \
	  src/HttpRequest.cpp \
	  src/HttpRequestParser.cpp \
	  src/Logger.cpp \
	  src/main.cpp \
	  src/utils.cpp

HEADERS = -Isrc

OBJ      = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp, $(SOURCES)))
OBJ     += $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out src/%.cpp, $(SOURCES)))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CC) $(FLAGS) $(HEADERS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

TESTS_DIR = tests/
.PHONY: tests
tests:
	cd $(TESTS_DIR)
	./run_tests #TODO: mudar para um makefile próprio
