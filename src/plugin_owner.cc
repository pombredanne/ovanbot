// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include <re2/re2.h>

#include "./bot.h"
#include "./plugin_owner.h"

namespace {
re2::RE2 join_re("^join #(.*)$");
}

namespace ovanbot {
void OwnerPlugin::HandlePrivmsg(const std::string &user,
                                const std::string &channel,
                                const std::string &msg) {
  if (!IsChannel(channel) && user == robot_->owner()) {
    std::string target;
    if (RE2::FullMatch(msg, join_re, &target)) {
      robot_->SendLine("JOIN #" + target);
    }
  }
}
}
