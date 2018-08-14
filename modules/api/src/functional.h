#include "crow/crow_all.h"
#include "hw_ivs.h"
#include "spdlog/spdlog.h"

auto console = spdlog::stdout_color_mt("thread process");

std::shared_ptr<vms::hwivs::HuaweiIVS> _login(
    const crow::json::rvalue body) {
  const std::string ip       = body["ip"].s();
  const std::string username = body["username"].s();
  const std::string password = body["password"].s();
  const std::string vendor   = body["vendor"].s();

  try {
    auto vms = std::make_shared<vms::hwivs::HuaweiIVS>("./log");
    vms->login(ip, 9900, username, password);
    return vms;
    // return session.login(ip, username, password, vendor);
  } catch (std::exception) {
    throw;
  }
}

RQ::Response _camera_list(RQ::Request req) {
  RQ::Response res;
  res.id = req.id;

  crow::json::wvalue json_resp;
  json_resp["ok"] = false;

  try {
    auto data = crow::json::load(req.body);
    auto vms  = _login(data);
    
    auto cameras = vms->camera_list(data["max_list"].i());

    json_resp["ok"]   = true;
    json_resp["code"] = "camera/success";

    for (unsigned int i = 0; i < cameras.size(); i++) {
      json_resp["camera"][i]["ip"]   = cameras.at(i).ip();
      json_resp["camera"][i]["name"] = cameras.at(i).name();
      json_resp["camera"][i]["code"] = cameras.at(i).code();
    }

  } catch (std::invalid_argument err) {
    console->error(err.what());
    json_resp["code"]    = "camera/invalid-argument";
    json_resp["message"] = "Argument is invalid.";
    res.code = 400;
  } catch (std::runtime_error err) {
    console->error(err.what());
    json_resp["code"]    = "camera/failed";
    json_resp["message"] = "Failed to find camera.";
    res.code = 400;
  }

  res.body = crow::json::dump(json_resp);
  return res;
}

RQ::Response _recording_list(RQ::Request req) {
  RQ::Response res;
  res.id = req.id;

  crow::json::wvalue json_resp;
  json_resp["ok"] = false;

  try {
    auto data = crow::json::load(req.body);
    auto vms  = _login(data);

    std::vector<vms::Record> records =
        vms->recording_list(data["camera_code"].s(), data["start_time"].s(),
                            data["end_time"].s(), data["max_list"].i());

    json_resp["ok"]   = true;
    json_resp["code"] = "recording/success";

    for (unsigned int i = 0; i < records.size(); i++) {
      json_resp["records"][i]["start_time"] = records.at(i).start_time;
      json_resp["records"][i]["end_time"]   = records.at(i).end_time;
    }
  } catch (std::invalid_argument err) {
    console->error(err.what());
    json_resp["code"]    = "recording/invalid-argument";
    json_resp["message"] = "Argument is invalid.";
    res.code = 400;
  } catch (std::runtime_error err) {
    console->error(err.what());
    json_resp["code"]    = "recording/failed";
    json_resp["message"] = "Failed to find recording.";
    res.code = 400;
  }

  res.body = crow::json::dump(json_resp);
  return res;
}

RQ::Response _playback(RQ::Request req) {
  RQ::Response res;
  res.id = req.id;

  crow::json::wvalue json_resp;
  json_resp["ok"] = false;

  try {
    auto data = crow::json::load(req.body);
    auto vms  = _login(data);

    std::string rtsp_url =
        vms->playback(data["camera_code"].s(), data["nvr_code"].s(),
                      data["start_time"].s(), data["end_time"].s());

    json_resp["ok"]   = true;
    json_resp["code"] = "recording/success";
    json_resp["url"]  = rtsp_url;
  } catch (std::invalid_argument err) {
    console->error(err.what());
    json_resp["code"]    = "recording/invalid-argument";
    json_resp["message"] = "Argument is invalid.";
    res.code = 400;
  } catch (std::runtime_error err) {
    console->error(err.what());
    json_resp["code"]    = "recording/failed";
    json_resp["message"] = "Failed to find recording.";
    res.code = 400;
  }

  res.body = crow::json::dump(json_resp);
  return res;
}

RQ::Response _live_stream(RQ::Request req) {
  RQ::Response res;
  res.id = req.id;

  crow::json::wvalue json_resp;
  json_resp["ok"] = false;

  try {
    auto data = crow::json::load(req.body);
    auto vms  = _login(data);

    std::string transport = "udp";
    if (data.has("transport") && data["transport"].s() == "tcp") {
      transport = "tcp";
    }

    std::string rtsp_url = vms->live_stream(data["camera_code"].s(),
                                            data["nvr_code"].s(), transport);

    json_resp["ok"]   = true;
    json_resp["code"] = "live-stream/success";
    json_resp["url"]  = rtsp_url;
  } catch (std::invalid_argument err) {
    console->error(err.what());
    json_resp["code"]    = "live-stream/invalid-argument";
    json_resp["message"] = "Argument is invalid.";
    res.code = 400;
  } catch (std::runtime_error err) {
    console->error(err.what());
    json_resp["code"]    = "live-stream/failed";
    json_resp["message"] = "Failed to find live stream.";
    res.code = 400;
  }

  res.body = crow::json::dump(json_resp);
  return res;
}
