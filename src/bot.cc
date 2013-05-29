// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include "./bot.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <re2/re2.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>

#include "./logfile.h"

using boost::asio::ip::tcp;

namespace {
// PM commands
re2::RE2 comm_join_re("^join #(.*)");

// IRC commands
const std::string user_ = "^:(\\S+)!\\S+ ";

// :evan_!evan@24.205.85.74 JOIN :#test
re2::RE2 join_re(user_ + "JOIN :(.+)");

// :evan_!evan@24.205.85.74 PART #test :lol
re2::RE2 part_re(user_ + "PART (\\S+) :(.*)");

// PING :irc.freenode.net
re2::RE2 ping_re("^PING :(.+)");

// :evan!evan@204.236.178.63 PRIVMSG #test :yay
re2::RE2 privmsg_re(user_ + "PRIVMSG (\\S+) :(.+)");

// :evan_!evan@24.205.85.74 QUIT :Quit: leaving
re2::RE2 quit_re(user_ + "QUIT :(.+)");

struct CheckRegexes {
  CheckRegexes() {
    assert(comm_join_re.ok());
    assert(join_re.ok());
    assert(part_re.ok());
    assert(ping_re.ok());
    assert(privmsg_re.ok());
    assert(quit_re.ok());
  }
};

CheckRegexes check_regexes_;

void TrimRight(std::string &s) {
  bool keep_going = true;
  while (!s.empty() && keep_going) {
    switch (s.back()) {
      case ' ':
      case '\r':
      case '\t':
      case '\n':
        s.erase(s.end() - 1);
        break;
      default:
        keep_going = false;
        break;
    }
  }
}
}

namespace ovanbot {

IRCRobot::IRCRobot(boost::asio::io_service &service,
                   const std::string &nick,
                   const std::string &password,
                   const std::string &owner)
    :io_service_(service),
     timer_(service),
     socket_(service),
     nick_(nick),
     password_(password),
     owner_(owner) {}


void IRCRobot::Connect(boost::asio::ip::tcp::resolver::iterator
                       endpoint_iterator) {
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
  socket_.lowest_layer().async_connect(
      endpoint,
      boost::bind(&IRCRobot::HandleConnect, this,
                  boost::asio::placeholders::error, ++endpoint_iterator));
}

void IRCRobot::HandleConnect(const boost::system::error_code& error,
                             tcp::resolver::iterator
                             endpoint_iterator) {
  if (!error) {
    if (password_.size()) {
      std::string s = "PASS ";
      s.append(password_);
      SendLine(s);
    }
    SendLine("NICK " + nick_);
    SendLine("USER ovanbot ovanbot ovanbot ovanbot");
    boost::asio::async_read_until(
        socket_, request_, '\n',
        boost::bind(
            &IRCRobot::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
  } else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {
    socket_.lowest_layer().close();
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
    socket_.lowest_layer().async_connect(
        endpoint,
        boost::bind(&IRCRobot::HandleConnect, this,
                    boost::asio::placeholders::error, ++endpoint_iterator));
  } else {
    std::cerr << "Connect failed: " << error;
    exit(1);
  }
}

void IRCRobot::SendLine(std::string msg) {
  std::cout << "W: " << msg << std::endl;

  // the message body has to have storage until the line is fully
  // sent; to do this, we make a copy of the message, and delete it later
  if (msg.empty() || msg.back() != '\n') {
    msg += '\n';
  }

  outgoing_.emplace_back(std::unique_ptr<char[]>(new char[msg.size()]));
  char *data_location = outgoing_.back().get();
  memcpy(data_location, msg.data(), msg.size());
  boost::asio::async_write(
      socket_,
      boost::asio::buffer(data_location, msg.size()),
      boost::bind(
          &IRCRobot::HandleWrite, this,
          data_location,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void IRCRobot::HandleRead(const boost::system::error_code &error,
                          size_t bytes_transferred) {
  std::string lines = extra_data_ + std::string(
      (std::istreambuf_iterator<char>(&request_)),
      std::istreambuf_iterator<char>());
  std::string::size_type offset = 0;
  while (true) {
    std::string::size_type pos = lines.find('\n', offset);
    if (pos == std::string::npos) {
      extra_data_ = lines.substr(offset);
      break;
    };
    std::string line = lines.substr(offset, pos - offset);
    TrimRight(line);
    HandleLine(line);
    offset = pos + 1;
    if (offset == lines.size()) {
      extra_data_.clear();
      break;
    }
  }

  boost::asio::async_read_until(
      socket_, request_, '\n',
      boost::bind(
          &IRCRobot::HandleRead, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

}

void IRCRobot::HandleLine(const std::string &line) {
  std::cout << "R: " << line << "\n";

  re2::StringPiece string1, string2, string3;
  if (RE2::FullMatch(line, ping_re, &string1)) {
    SendLine("PONG " + string1.as_string());
  } else if (RE2::FullMatch(line, privmsg_re, &string1, &string2, &string3)) {
    HandlePrivmsg(string1.as_string(),
                  string2.as_string(),
                  string3.as_string());
  } else if (RE2::FullMatch(line, join_re, &string1, &string2)) {
    HandleJoin(string1.as_string(), string2.as_string());
  } else if (RE2::FullMatch(line, quit_re, &string1, &string2)) {
    HandleQuit(string1.as_string(), string2.as_string());
  } else if (RE2::FullMatch(line, part_re, &string1, &string2, &string3)) {
    HandlePart(string1.as_string(), string2.as_string(), string3.as_string());
  }
}

void IRCRobot::HandlePrivmsg(const std::string &from_user,
                             const std::string &channel,
                             const std::string &msg) {
  if (channel.front() == '#') {
    Log(channel, from_user + ": " + msg);
  } else if (from_user == owner_ ){
    std::string s;
    if (RE2::FullMatch(msg, comm_join_re, &s)) {
      SendLine("JOIN #" + s);
    }
  }
}

void IRCRobot::HandleJoin(const std::string &user,
                          const std::string &channel) {
  Log(channel, "<--- " + user + " joins");
}

void IRCRobot::HandleQuit(const std::string &user,
                          const std::string &reason) {
}

void IRCRobot::HandlePart(const std::string &user,
                          const std::string &channel,
                          const std::string &reason) {
  Log(channel, "---> " + user + " parts");
}


void IRCRobot::HandleWrite(const char *outgoing_msg,
                           const boost::system::error_code& error,
                           size_t bytes_transferred) {
  assert(!outgoing_.empty() && outgoing_.front().get() == outgoing_msg);
  outgoing_.pop_front();
  if (error)  {
    std::cerr << "Write error: " << error;
  }
}
}
