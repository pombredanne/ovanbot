// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_PLUGIN_KARMA_H_
#define OVANBOT_PLUGIN_KARMA_H_

#include <map>
#include <string>

#include "./plugin.h"

namespace ovanbot {
class KarmaPlugin : public Plugin {
 public:
  void HandlePrivmsg(const std::string &user,
                     const std::string &channel,
                     const std::string &msg);
 private:
  std::map<std::string, std::int32_t> karma_;
};
}

#endif  // OVANBOT_PLUGIN_KARMA_H_
