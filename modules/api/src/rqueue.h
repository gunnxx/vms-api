#ifndef REQ_QUEUE
#define REQ_QUEUE

#include <string>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

#include "crow/crow_all.h"

namespace RQ {

struct Request {
	uint32_t id;
	std::string method;
	std::string body;
};

struct Response {
	uint32_t id;
	int code;
	std::string body;
};

typedef bool ack;

template <class datatype>
class ThreadQueue {
  public:
  	void push(datatype elem);	// Push element to queue
  	datatype pop();				    // Pop the front element and return it
  	datatype front();			    // Return front element
  	bool empty();				      // true if empty

  private:
  	std::queue<datatype> elems;
  	std::mutex m;
};

class RQServer {
  public:
	RQServer (int threads = 3);

	ThreadQueue <Request>  *_getRequestQueue();
	ThreadQueue <Response> *_getResponseQueue();

	void run();
	void _shutDown();
  
  private:
  	bool _shutdown = false;

  	int _threads;
  	std::vector <std::thread *> active_threads;

  	ThreadQueue <Request>  _request_queue;
  	ThreadQueue <Response> _response_queue;

  	void _threadCallback();
};

class RQClient {
  public:
  	RQClient ();

  	ack connect (RQServer qserver);
  	ack publish (Request req);

  	Response findResponse (uint32_t req_id);

  private:
  	ThreadQueue <Request>  *_request_queue;
  	ThreadQueue <Response> *_response_queue;
};

} // namespace RQ

#endif

/*
[ DONE ]
1. Made class queue (thread-safe)
	- #include <mutex>
	- push() method just use push()
	- pop() method combine front() and pop() from std::queue
	- front() method just use front() method
[ DONE ]
2. Create threads with each thread :
	- while true, check queue
	- call method according to request.method
	- get the response
	- append the response to the _response_queue
[ X ]
3. Add timeout to findResponse() always use front() method
[ DONE ] -> check _shutDown()
4. How to close threads
*/
