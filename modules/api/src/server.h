#ifndef SERVER_H
#define SERVER_H

#include "crow/crow_all.h"
#include "hw_ivs.h"
#include "session.h"
#include "rqueue.h"

namespace vms {
namespace api {

class Server {
 public:
    Server();
    void run(int port = 8000);
    void shutdown();;

 private:
    static void _exec_request(const crow::request &req,
                              const crow::response &res,
                              std::string method);

    crow::SimpleApp _app;

    RQ::RQServer qserver(3);
};

}  // namespace api
}  // namespace vms

#endif
