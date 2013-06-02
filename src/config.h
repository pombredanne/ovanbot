// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef OVANBOT_CONFIG_H_
#define OVANBOT_COMFIG_H_

#include <string>

namespace ovanbot {
extern std::string working_dir;

inline std::string JoinPath(const std::string &left,
                            const std::string &right) {
  if (!left.empty()) {
    if (left.back() != '/') {
      return left + "/" + right;
    } else {
      return left + right;
    }
  } else {
    return right;
  }
}
}
#endif  // OVANBOT_CONFIG_H_
