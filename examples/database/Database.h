#pragma once

#include <string>
#include <vector>
#include <map>

class IDatabaseConnection;

class Database {
public:
  explicit Database(std::unique_ptr<IDatabaseConnection> connection);

  void open();
  void close();
  void beginWrite();
  void executeWrite();
  void put(const std::string &key, const std::string &value);

  bool get(const std::string &key, std::string &value);

private:
  bool m_open;
  std::unique_ptr<IDatabaseConnection> m_connection;
  std::map<std::string, std::string> m_cache;
  bool m_hasBlock;
  std::vector<std::pair<std::string, std::string>> m_queue;
  std::size_t m_hash;
};
