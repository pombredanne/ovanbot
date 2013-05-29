// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_PLUGIN_H_
#define OVANBOT_PLUGIN_H_

#include <string>

namespace ovanbot {

class IRCRobot;

class Plugin {
 public:

  inline void set_robot(IRCRobot *robot) { robot_ = robot; }

  virtual ~Plugin() {}

  virtual void HandlePrivmsg(const std::string &user,
                             const std::string &channel,
                             const std::string &msg);

 protected:
  IRCRobot *robot_;

  inline bool IsChannel(const std::string &str) {
    return !str.empty() && str.front() == '#';
  }
};
}

#endif  // OVANBOT_PLUGIN_H_
