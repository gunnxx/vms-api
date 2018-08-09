#include "rqueue.h"
#include "functional.h"

/* -------------
    ThreadQueue
   ------------- */
template <class datatype>
void RQ::ThreadQueue<datatype>::push (datatype elem) {
	std::lock_guard<std::mutex> lock(m);
	elems.push(elem);
}

template <class datatype>
datatype RQ::ThreadQueue<datatype>::pop () {
	std::lock_guard<std::mutex> lock(m);
	datatype elem = elems.front();
	elems.pop();
	return elem;
}

template <class datatype>
datatype RQ::ThreadQueue<datatype>::front () {
	std::lock_guard<std::mutex> lock(m);
	return elems.front();
}

template <class datatype>
bool RQ::ThreadQueue<datatype>::empty () {
	std::lock_guard<std::mutex> lock(m);
	return elems.empty();
}

/* ----------
    RQServer
   ---------- */
RQ::RQServer::RQServer (int threads = 3) {
	_threads = threads;
}

std::queue <RQ::Request> *RQ::RQServer::_getRequestQueue () {
	return &_request_queue;
}

std::queue <RQ::Response> *RQ::RQServer::_getResponseQueue () {
	return &_response_queue;
}

void RQ::RQServer::run () {
	active_threads.resize(_threads);

	for(int i=0; i<_threads; i++)
		active_threads[i] = std::thread(_threadCallback);

	for(int i=0; i<_threads; i++)
		active_threads[i].join();
}

void RQ::RQServer::_threadCallback () {
	while(!shut_down) {
	  if(! _request_queue.empty()) {
		RQ::Request req = _request_queue.pop();
		RQ::Response res;

		if(req.method == "camera-list")
			res = _camera_list(req);
		
		else if(req.method == "recording-list")
			res = _recording_list(req);
		
		else if(req.method == "playback")
			res = _playback(req);
		
		else if(req.method == "live-stream")
			res = _live_stream(req);
		
		else if(req.method == "sdk-playback")
			res = _sdk_playback(req);

		else { // Method not recognized
			crow::json::wvalue json_resp;
			json_resp["ok"]      = false;
			json_resp["code"]    = "request not recognized";
			json_resp["message"] = "Failed to find request";

			res.id   = req.id;
			res.body = crow::json::dump(json_resp);
			res.code = 400;
		}

		_response_queue.push(res);
	  }
	  std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void RQ::RQServer::_shutDown () {
	_shutdown = true;
}

/* ----------
    RQClient
   ---------- */

RQ::RQClient::RQClient (){}

RQ::ack RQ::RQClient::connect (RQServer qserver) {
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
			_response_queue->front()).id == req_id)
			return _response_queue->pop();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
