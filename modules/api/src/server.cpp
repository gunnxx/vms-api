#include "server.h"
#include "spdlog/spdlog.h"

namespace api = vms::api;

auto console = spdlog::stdout_color_mt("server");
// api::Session session;

// helper function
long long get_microtime_now();

api::Server::Server() {
  CROW_ROUTE(_app, "/<string>").methods("POST"_method)(_exec_request);
}

void api::Server::run(int port) {
  _qserver.run();
  _app.port(port).multithreaded().run();
}

void api::Server::_exec_request(
    const crow::request &req,
    const crow::response &res,
    std::string method) {
  RQ::Request request;
  RQ::RQClient qclient;

  // Get request body
  request.id     = get_microtime_now();
  request.method = method;
  request.body   = req.body;

  // Append to RQServer queue
  qclient.connect(qserver);
  qclient.publish(request);
  
  // Find (wait) and fill in response
  RQ::Response response = qclient.findResponse(request.id);
  res.code = response.code;
  res.add_header("Content-Type", "application/json");
  res.write(response.body)
  res.end();
}

long long get_microtime_now(){
  auto ts = chrono::time_point_cast<chrono::microseconds>(chrono::system_clock::now());
  return static_cast<long long>(chrono::system_clock::to_time_t(ts));
}