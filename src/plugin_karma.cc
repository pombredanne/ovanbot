// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include "./plugin_karma.h"
#include "./protobuf/ovanbot.pb.h"

#include <fstream>
#include <sstream>
#include <queue>
#include <re2/re2.h>
#include <boost/lexical_cast.hpp>

namespace {
re2::RE2 dec_karma_re("^(.*)--\\s*$");
re2::RE2 inc_karma_re("^(.*)\\+\\+\\s*$");
re2::RE2 karma_query_re("^!karma (.*)$");


const std::string allowed_chars("abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "0123456789"
                                "'\":;,./?<>!@#$%^&*-_+=`~\\|");

std::string CleanString(const std::string &input) {
  std::stringstream ss;
  for (const auto &ch : input) {
    if (allowed_chars.find(ch) != std::string::npos) {
      ss << ch;
    }
  }
  return ss.str();
}
}

namespace ovanbot {
KarmaPlugin::KarmaPlugin() {
  std::ifstream karma_file (KarmaPath(), std::ios::binary);
  if (karma_file.good()) {
    KarmaMap karma_map;
    karma_map.ParseFromIstream(&karma_file);
    for (int i = 0; i < karma_map.karma_list_size(); i++) {
      const auto &k = karma_map.karma_list(i);
      karma_.insert(std::make_pair(k.entity(), k.karma()));
    }
  }
}

void KarmaPlugin::HandlePrivmsg(const std::string &user,
                                const std::string &channel,
                                const std::string &msg) {
  std::string target;
  if (channel.empty() || !channel.front() == '#') {
    // don't count private messages
    return;
  } else if (msg == "!karma" && !karma_.empty()) {
    std::priority_queue<std::pair<int, std::string> > queue;
    for (const auto &kv : karma_) {
      queue.push(std::make_pair(kv.second, kv.first));
    }

    std::stringstream ss;
    const std::size_t count = std::min(10ul, queue.size()) - 1;
    for (int i = 0; i < count; i++) {
      auto pair = queue.top();
      queue.pop();
      ss << pair.second << ": " << pair.first << ", ";
    }
    auto pair = queue.top();
    ss << pair.second << ": " << pair.first;
    robot_->SendPrivmsg(channel, ss.str());
    return;
  } else if (msg == "!karmareverse" && !karma_.empty()) {
    std::priority_queue<std::pair<int, std::string> > queue;
    for (const auto &kv : karma_) {
      queue.push(std::make_pair(-kv.second, kv.first));
    }

    std::stringstream ss;
    const std::size_t count = std::min(10ul, queue.size()) - 1;
    for (int i = 0; i < count; i++) {
      auto pair = queue.top();
      queue.pop();
      ss << pair.second << ": " << -pair.first << ", ";
    }
    auto pair = queue.top();
    ss << pair.second << ": " << pair.first;
    robot_->SendPrivmsg(channel, ss.str());
    return;
  } else if (RE2::FullMatch(msg, karma_query_re, &target)) {
    auto it = karma_.lower_bound(target);
    if (it != karma_.end() && it->first == target) {
      robot_->SendPrivmsg(
          channel, target + ": " + boost::lexical_cast<std::string>(it->second));
    }
  } else if (RE2::FullMatch(msg, inc_karma_re, &target)) {
    target = CleanString(target);
    if (target.empty()) {
      return;
    }
    auto it = karma_.lower_bound(target);
    if (it == karma_.end() || it->first != target) {
      karma_.insert(it, std::make_pair(target, 1));
    } else {
      it->second++;
    }
  } else if (RE2::FullMatch(msg, dec_karma_re, &target)) {
    target = CleanString(target);
    if (target.empty()) {
      return;
    }
    auto it = karma_.lower_bound(target);
    if (it == karma_.end() || it->first != target) {
      karma_.insert(it, std::make_pair(target, -1));
    } else {
      it->second--;
    }
  }
  Serialize();
}

void KarmaPlugin::Serialize() const {
  KarmaMap karma_map;
  for (const auto &pair : karma_) {
    KarmaMap::Karma *karma = karma_map.add_karma_list();
    karma->set_entity(pair.first);
    karma->set_karma(pair.second);
  }
  std::ofstream out(KarmaPath());
  karma_map.SerializeToOstream(&out);
}
}
