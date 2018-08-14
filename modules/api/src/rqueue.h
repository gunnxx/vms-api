#ifndef REQ_QUEUE
#define REQ_QUEUE

#include <stdexcept>
#include <string>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

namespace RQ {

struct Request {
	bool exist = false;
  long long id;
	std::string method;
	std::string body;
};

struct Response {
	long long id;
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

    datatype popIfExist();    // Check empty() -> if not empty then pop() [prevent race condition]

  private:
  	std::queue<datatype> elems;
  	std::mutex m;
};

class RQServer {
  public:
	RQServer (int threads);
  ~RQServer();

	ThreadQueue <Request>  *_getRequestQueue();
	ThreadQueue <Response> *_getResponseQueue();

  bool _getShutDownStatus();
	void run();
  
  private:
  	int _threads;
  	std::vector <std::thread> active_threads;

  	ThreadQueue <Request>  _request_queue;
  	ThreadQueue <Response> _response_queue;

  	bool _shutdown;
};

class RQClient {
  public:
  	RQClient ();

  	ack connect (RQServer &qserver);
  	ack publish (Request req);

  	Response findResponse (uint32_t req_id);

  private:
  	ThreadQueue <Request>  *_request_queue;
  	ThreadQueue <Response> *_response_queue;
};

} // namespace RQ

/* -------------
    ThreadQueue
   ------------- */

template <class datatype>
void RQ::ThreadQueue<datatype>::push (datatype elem) {
  std::lock_guard<std::mutex> lock(m);
  elems.push(elem);
}

template <class datatype>
datatype RQ::ThreadQueue<datatype>::pop() {
  std::lock_guard<std::mutex> lock(m);
  
  if(elems.empty())
    throw std::out_of_range("Empty queue. Can't call method pop().\n");

  datatype elem = elems.front();
  elems.pop();
  return elem;
}

template <class datatype>
datatype RQ::ThreadQueue<datatype>::front () {
  std::lock_guard<std::mutex> lock(m);

  if(elems.empty())
    throw std::out_of_range("Empty queue. Can't call method front().\n");

  return elems.front();
}

template <class datatype>
bool RQ::ThreadQueue<datatype>::empty () {
  std::lock_guard<std::mutex> lock(m);
  return elems.empty();
}

template <class datatype>
datatype RQ::ThreadQueue<datatype>::popIfExist () {
  std::lock_guard<std::mutex> lock(m);

  datatype elem;
  if(!elems.empty()){ // If not empty
    elem = elems.front();
    elem.exist = true;
    elems.pop();
  }
  return elem;
}

#endif