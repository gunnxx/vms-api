#include "server.h"

int main() {
  vms::api::Server server;

  server.run();
  server.shutdown();

  return 0;
}
