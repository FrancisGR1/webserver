#include "config/Config.hpp"
#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "server/Webserver.hpp"

int main(int argc, char *argv[]) {
  if (argc > 2) {
    Logger::error("./webserv <configurations_path>.conf");
    return 1;
  }
  try {
    // config
    const char *file_path = argc == 2 ? argv[1] : constants::default_conf;
    Config config;
    config.load(file_path);

    // server
    Webserver server(config);
    server.setup();
    server.run();
  } catch (const std::exception &e) {
    Logger::fatal("%s", e.what());
  }

  Logger::finalize();

  return (0);
}
