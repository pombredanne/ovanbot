// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

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

using boost::asio::ip::tcp;

namespace {
re2::RE2 join_re("^join #(.*)");
re2::RE2 ping_re("^PING :(.+)");
re2::RE2 privmsg_re("^:(\\S+)!\\S+ PRIVMSG (\\S+) :(.+)");

struct CheckRegexes {
  CheckRegexes() {
    assert(join_re.ok());
    assert(ping_re.ok());
    assert(privmsg_re.ok());
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

  char *allocated_buf = new char[msg.size()];
  memcpy(allocated_buf, msg.data(), msg.size());
  outgoing_.push_back(allocated_buf);
  boost::asio::async_write(
      socket_,
      boost::asio::buffer(allocated_buf, msg.size()),
      boost::bind(
          &IRCRobot::HandleWrite, this,
          allocated_buf,
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
  }
}

void IRCRobot::HandlePrivmsg(const std::string &from_user,
                             const std::string &channel,
                             const std::string &msg) {
  if (channel.front() == '#') {
    // Log messages to channels (not PMs from users)
    std::time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);
    static char fmt_time[128];
    strftime(fmt_time, sizeof(fmt_time), "%F %T", timeinfo);

    std::ofstream logfile(channel + ".log", std::ios::app);
    logfile << fmt_time
            << " " << from_user << ": " << msg << "\n";
  } else if (from_user == owner_ ){
    std::string s;
    if (RE2::FullMatch(msg, join_re, &s)) {
      SendLine("JOIN #" + s);
    }
  }
}

void IRCRobot::HandleWrite(const char *outgoing_msg,
                           const boost::system::error_code& error,
                           size_t bytes_transferred) {
  assert(!outgoing_.empty() && outgoing_.front() == outgoing_msg);
  delete outgoing_.front();
  outgoing_.erase(outgoing_.begin());
  if (error)  {
    std::cerr << "Write error: " << error;
  }
}
}
