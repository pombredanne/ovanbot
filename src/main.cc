// -*- C++ -*-
// Copyright 2013, Evan Klitzke <evan@eklitzke.org>

#include <stdio.h>
#include <syslog.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>

#include "./bot.h"

namespace po = boost::program_options;
using boost::asio::ip::resolver_query_base;

ovanbot::IRCRobot *bot;

int main(int argc, char **argv) {
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "produce help message")
      ("host,H", po::value<std::string>(), "host to connect to")
      ("port,p", po::value<std::uint16_t>()->default_value(6667),
       "port to connect to")
      ("password", po::value<std::string>()->default_value(""),
       "connect password")
      ("nick,n", po::value<std::string>()->default_value("ovanbot"),
       "bot nickname")
      ("owner", po::value<std::string>()->default_value("evan"),
       "owner of the bot")
      ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc;
    return 0;
  }

  if (!vm.count("host")) {
    std::cout << desc;
    std::cout << "\nmissing --host\n";
    return 0;
  }

  boost::asio::io_service io_service;

  // initiate the name resolution
  const std::string host = vm["host"].as<std::string>();
  const std::uint16_t port = vm["port"].as<std::uint16_t>();
  const std::string str_port = boost::lexical_cast<std::string>(port);
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(
      host, str_port.c_str(), resolver_query_base::numeric_service);
  boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

  const std::string nick = vm["nick"].as<std::string>();
  const std::string owner = vm["owner"].as<std::string>();
  const std::string pass = vm["password"].as<std::string>();
  ovanbot::IRCRobot robot(io_service, nick, pass, owner);

  robot.Connect(iterator);
  io_service.run();

  return 0;
}
