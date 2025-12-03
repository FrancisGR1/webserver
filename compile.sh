#c++ -g src/main.cpp src/utils.cpp src/ConfigLexer.cpp src/Logger.cpp src/ConfigParser.cpp src/Config.cpp -o webserv

c++ -g src/main.cpp src/utils.cpp src/HTTP*.cpp src/Logger.cpp -o webserv
