// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#ifndef BOT_H_
#define BOT_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <list>
#include <memory>
#include <string>

#include "./plugin.h"

using boost::asio::ip::tcp;

namespace ovanbot {

class Plugin;

class IRCRobot {
 public:
  explicit IRCRobot(boost::asio::io_service &service,
                    const std::string &nick = "ovanbot",
                    const std::string &password = "",
                    const std::string &owner = "evan");

  void Connect(boost::asio::ip::tcp::resolver::iterator);
  bool Authenticate(const std::string &, const std::string &,
                    const std::string &);
  void AddChannel(const std::string &);

  inline const std::string& owner() const { return owner_; }

  void SendLine(std::string);

 private:
  boost::asio::io_service &io_service_;
  boost::asio::deadline_timer timer_;
  boost::asio::ip::tcp::socket socket_;

  const std::string nick_;
  const std::string password_;
  const std::string owner_;

  std::vector<std::unique_ptr<Plugin> > plugins_;

  // storage for outgoing messages
  std::list<std::unique_ptr<char[]> > outgoing_;
  std::string extra_data_;
  boost::asio::streambuf request_;

  void HandleConnect(const boost::system::error_code&, tcp::resolver::iterator);
  void HandleHandshake(const boost::system::error_code&);
  void HandleWrite(const char *, const boost::system::error_code&, size_t);
  void HandleTimeout(const boost::system::error_code&);
  void HandleRead(const boost::system::error_code &, size_t);
  void HandleLine(const std::string &line);
};
}

#endif  // BOT_H_
