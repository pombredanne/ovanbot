// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_PLUGIN_OWNER_H_
#define OVANBOT_PLUGIN_OWNER_H_

#include <string>

#include "./bot.h"

namespace ovanbot {
class OwnerPlugin : public Plugin {
 public:
  void HandlePrivmsg(const std::string &user,
                     const std::string &channel,
                     const std::string &msg);
};
}

#endif  // OVANBOT_PLUGIN_OWNER_H_
