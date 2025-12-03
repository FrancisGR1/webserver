In file included from src/webserver.hpp:20,
                 from src/Config.hpp:2,
                 from src/Config.cpp:1:
src/ConfigParser.hpp:13:17: error: ‘Config’ does not name a type
   13 |                 Config m_config;
      |                 ^~~~~~
src/ConfigParser.hpp:20:46: error: ‘ServiceConfig’ has not been declared
   20 |                 void parse_service(Token& t, ServiceConfig& service);
      |                                              ^~~~~~~~~~~~~
src/ConfigParser.hpp:21:47: error: ‘LocationConfig’ has not been declared
   21 |                 void parse_location(Token& t, LocationConfig& location);
      |                                               ^~~~~~~~~~~~~~
src/ConfigParser.hpp:23:69: error: ‘Directive’ has not been declared
   23 |                 void parse_directive(Token& t, Token::Type context, Directive& directive);
      |                                                                     ^~~~~~~~~
src/ConfigParser.hpp:25:47: error: ‘Directive’ has not been declared
   25 |                 void parse_listener(Token& t, Directive& directive);
      |                                               ^~~~~~~~~
src/ConfigParser.hpp:26:50: error: ‘Directive’ has not been declared
   26 |                 void parse_server_name(Token& t, Directive& directive);
      |                                                  ^~~~~~~~~
src/ConfigParser.hpp:27:49: error: ‘Directive’ has not been declared
   27 |                 void parse_upload_dir(Token& t, Directive& directive);
      |                                                 ^~~~~~~~~
src/ConfigParser.hpp:28:52: error: ‘Directive’ has not been declared
   28 |                 void parse_max_body_size(Token& t, Directive& directive);
      |                                                    ^~~~~~~~~
src/ConfigParser.hpp:29:49: error: ‘Directive’ has not been declared
   29 |                 void parse_error_page(Token& t, Directive& directive);
      |                                                 ^~~~~~~~~
src/ConfigParser.hpp:30:43: error: ‘Directive’ has not been declared
   30 |                 void parse_root(Token& t, Directive& directive);
      |                                           ^~~~~~~~~
src/ConfigParser.hpp:31:46: error: ‘Directive’ has not been declared
   31 |                 void parse_methods(Token& t, Directive& directive);
      |                                              ^~~~~~~~~
src/ConfigParser.hpp:32:51: error: ‘Directive’ has not been declared
   32 |                 void parse_default_file(Token& t, Directive& directive);
      |                                                   ^~~~~~~~~
src/ConfigParser.hpp:33:46: error: ‘Directive’ has not been declared
   33 |                 void parse_listing(Token& t, Directive& directive);
      |                                              ^~~~~~~~~
src/ConfigParser.hpp:34:45: error: ‘Directive’ has not been declared
   34 |                 void parse_upload(Token& t, Directive& directive);
      |                                             ^~~~~~~~~
src/ConfigParser.hpp:35:42: error: ‘Directive’ has not been declared
   35 |                 void parse_cgi(Token& t, Directive& directive);
      |                                          ^~~~~~~~~
src/ConfigParser.hpp:36:47: error: ‘Directive’ has not been declared
   36 |                 void parse_redirect(Token& t, Directive& directive);
      |                                               ^~~~~~~~~
