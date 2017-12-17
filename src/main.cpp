#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>
#include <fstream>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }


// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main(int argc, char *argv[])
{
  uWS::Hub h;

  PID pid_s;
  PID pid_t;
  bool train;

  // check if training flag is sent
  train = argc > 0;
    
  // TODO: Initialize the pid variable.
  // pid.Init(0.9, 3.0, 0.5); // 0.3 throttle 

  // pid.Init(1.3, 3.0, 0.5, "steer_metrics.csv");
  // pid_s.Init(1.3, 3.0, 0.5, "steer_metrics.csv");
  // pid_s.Init(1.4, 1.9, 0.4, "steer_metrics.csv");
  // pid_s.Init(0.3, 0.3, 0.05, "steer_metrics.csv");
  pid_s.Init(0.246, 0.057, 0.01, "steer_metrics.csv");
  pid_t.Init(0.017, 0.001, 0.001, "throttle_metrics.csv");

  h.onMessage([&pid_s, &pid_t, train](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          

          pid_s.UpdateError(cte, train);            
          steer_value = pid_s.TotalError();          

          const double REFERENCE_SPEED = 0.8;
          pid_t.UpdateError((speed - REFERENCE_SPEED) * cte, train);
          double throttle = REFERENCE_SPEED - pid_t.TotalError();
                                       

          // use hyperbolic tangent to keep the steer in [-1, 1] range
          // http://mathworld.wolfram.com/HyperbolicTangent.html          
          // if the steer value is out of range then normalize it to the range
          if (steer_value > 1 || steer_value < -1) {
            steer_value = tanh(steer_value);                    
          }

          // print every few runs
          if (pid_s.run_ % 500 == 0) {
            // DEBUG
            // print the epoch and run
            std::cout << "#Epoch: " << pid_s.epoch_ << " #Run: " << pid_s.run_ << std::endl;
            std::cout << "CTE: " << cte << " Steering Value: " << steer_value << " Throttle Value: " << throttle << std::endl;
          }
          

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          // std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {    
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
