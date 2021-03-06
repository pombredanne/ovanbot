// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include "./plugin_logging.h"

#include <ctime>
#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <string>

namespace ovanbot {
Logger::Logger(const std::string &logname)
    :logname_(logname),
     logfile_(logname, std::ios::app) {}

void Logger::Rotate(const std::string &name) {
#if 0
  // for newer compilers
  std::ofstream new_file(name, std::ios::app);
  logfile_.swap(new_file);
#else
  logfile_.close();
  logfile_.open(name, std::ios::app);
#endif
}


void Logger::LogLine(const std::string &str) {
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

void LoggingPlugin::Log(const std::string &logger, const std::string &msg) {
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
#if 0
    it = loggers_.emplace_hint(it, {logger, full_name});
#else
    auto p = loggers_.insert(std::make_pair(
        logger, std::unique_ptr<Logger>(new Logger(full_name))));
    it = p.first;
#endif
  } else if (it->second->FileName() != full_name) {
    it->second->Rotate(full_name);
  }
  it->second->LogLine(msg);
}

void LoggingPlugin::HandlePrivmsg(const std::string &user,
                                  const std::string &channel,
                                  const std::string &msg) {
  if (IsChannel(channel)) {
    Log(channel, user + ": " + msg);
  }
}
}
