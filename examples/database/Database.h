#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "User.h"

class IDatabaseConnection;

class Database {
public:
  explicit Database(std::unique_ptr<IDatabaseConnection> connection);

  void open();
  void close();
  void beginWrite();
  void executeWrite();
  void put(User user);
  bool get(const std::string &username, User &user);

private:
  bool m_open;
  std::unique_ptr<IDatabaseConnection> m_connection;
  std::map<std::string, User> m_cache;
  bool m_hasBlock;
  std::vector<User> m_queue;
  std::size_t m_hash;
};
