#include "rqueue.h"
#include "functional.h"

void _threadCallback (RQ::RQServer *qserver) {
	RQ::ThreadQueue <RQ::Request> *_request_queue =
									qserver->_getRequestQueue();
	RQ::ThreadQueue <RQ::Response> *_response_queue =
									 qserver->_getResponseQueue();

	while( !(qserver->_getShutDownStatus()) ) {
		RQ::Request req = _request_queue->popIfExist();
		if(req.exist){
			RQ::Response res;

			if(req.method == "camera-list")
				res = _camera_list(req);
			
			else if(req.method == "recording-list")
				res = _recording_list(req);
			
			else if(req.method == "playback")
				res = _playback(req);
			
			else if(req.method == "live-stream")
				res = _live_stream(req);
			
			else { // Method not recognized
				res.id   = req.id;
				res.body = "METHOD NOT RECOGNIZED";
				res.code = 400;
			}

			_response_queue->push(res);
		} // end if
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	} // end while
}

/* ----------
    RQServer
   ---------- */
RQ::RQServer::RQServer (int threads) {
	_threads = threads;
}

RQ::RQServer::~RQServer () {
	std::lock_guard<std::mutex> lock(std::mutex);
	_shutdown = true;
	for(int i=0; i<_threads; i++)
		active_threads[i].join();
}

RQ::ThreadQueue <RQ::Request> *RQ::RQServer::_getRequestQueue () {
	return &_request_queue;
}

RQ::ThreadQueue <RQ::Response> *RQ::RQServer::_getResponseQueue () {
	return &_response_queue;
}

bool RQ::RQServer::_getShutDownStatus () {
	std::lock_guard<std::mutex> lock(std::mutex);
	return _shutdown;
}

void RQ::RQServer::run () {
	_shutdown = false;
	for(int i=0; i<_threads; i++)
		active_threads.push_back(std::thread(_threadCallback, this));
}

/* ----------
    RQClient
   ---------- */

RQ::RQClient::RQClient (){}

RQ::ack RQ::RQClient::connect (RQServer &qserver) {
	_request_queue  = qserver._getRequestQueue();
	_response_queue = qserver._getResponseQueue();
	return true;
}

RQ::ack RQ::RQClient::publish (RQ::Request req) {
	_request_queue -> push(req);
	return true;
}

RQ::Response RQ::RQClient::findResponse (uint32_t req_id) {
	while(true) {
		if(! _response_queue->empty() &&
			_response_queue->front().id == req_id)
			return _response_queue->pop();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
