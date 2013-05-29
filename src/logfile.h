// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_LOGFILE_H_
#define OVANBOT_LOGFILE_H_

#include <string>

namespace ovanbot {
void Log(const std::string &logger, const std::string &msg);
void FlushLogs();
}

#endif  // OVANBOT_LOGFILE_H_
