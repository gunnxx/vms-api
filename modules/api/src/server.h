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

 private:
    static void _exec_request(const crow::request &req,
                              const crow::response &res,
                              std::string method);

    crow::SimpleApp _app;

    RQ::RQServer _qserver(3);	// 3 threads
};

}  // namespace api
}  // namespace vms

#endif
