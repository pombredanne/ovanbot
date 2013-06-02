// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_PLUGIN_LOGGING_H_
#define OVANBOT_PLUGIN_LOGGING_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "./plugin.h"

namespace ovanbot {
class Logger {
 public:
  Logger() = delete;
  Logger(const std::string &logname);
  Logger(const Logger &other) = delete;

  inline const std::string& FileName() const { return logname_; }
  void LogLine(const std::string &str);
  void Rotate(const std::string &name);

 private:
  std::string logname_;
  std::ofstream logfile_;
};

class Logger;

class LoggingPlugin : public Plugin {
  void HandlePrivmsg(const std::string &user,
                     const std::string &channel,
                     const std::string &msg);

  void Log(const std::string &logger, const std::string &msg);
  std::map<std::string, std::unique_ptr<Logger> > loggers_;
};
}
#endif  // OVANBOT_PLUGIN_LOGGING_H_
