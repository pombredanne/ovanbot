// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include "./logfile.h"

#include <ctime>
#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <string>

namespace {
class Logger {
 public:
  Logger(const std::string &logname)
      :logname_(logname),
       logfile_(logname, std::ios::app) {
  }

  inline const std::string& FileName() const { return logname_; }

  void LogLine(const std::string &str) {
    std::time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    static char fmt_time[128];
    strftime(fmt_time, sizeof(fmt_time), "%F %T ", &timeinfo);

    // attempt to do the log with a single output operation, to
    // prevent interleaving if possible
    std::stringstream ss;
    ss << fmt_time;
    ss << str;
    if (str.empty() || str.back() != '\n') {
      ss << '\n';
    }
    logfile_ << ss.str();
    logfile_.flush();
  }

 private:
  std::string logname_;
  std::ofstream logfile_;
};

std::map<std::string, std::unique_ptr<Logger> > loggers_;
}

namespace ovanbot {
void Log(const std::string &logger, const std::string &msg) {
  // compute the true log name
  std::time_t now;
  time(&now);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  static char fmt_time[128];
  strftime(fmt_time, sizeof(fmt_time), "-%F", &timeinfo);

  std::string full_name = logger + std::string(fmt_time) + ".log";

  auto it = loggers_.lower_bound(logger);
  if (it == loggers_.end() || it->first != logger) {
    it = loggers_.insert(
        it, std::make_pair(logger, std::unique_ptr<Logger>(new Logger(full_name))));
  } else if (it->second->FileName() != full_name) {
    loggers_.erase(it);
    it = loggers_.insert(
        it, std::make_pair(logger, std::unique_ptr<Logger>(new Logger(full_name))));
  }
  it->second->LogLine(msg);
}

void FlushLogs() {
  loggers_.clear();
}
}
