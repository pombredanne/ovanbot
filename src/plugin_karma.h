// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_PLUGIN_KARMA_H_
#define OVANBOT_PLUGIN_KARMA_H_

#include <map>
#include <string>

#include "./bot.h"
#include "./config.h"

namespace ovanbot {
class KarmaPlugin : public Plugin {
 public:
  KarmaPlugin();
  void HandlePrivmsg(const std::string &user,
                     const std::string &channel,
                     const std::string &msg);
 private:
  std::map<std::string, std::int32_t> karma_;

  void Serialize() const;
  inline std::string KarmaPath() const {
    return JoinPath(working_dir, "karma.pb");
  }
};
}

#endif  // OVANBOT_PLUGIN_KARMA_H_
