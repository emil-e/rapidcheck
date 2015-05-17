#pragma once

#include <string>
#include <vector>
#include <memory>

struct Message {
  Message() = default;
  Message(std::string t,
          std::vector<std::string> p = std::vector<std::string>());

  std::string type;
  std::vector<std::string> params;
};

class IDatabaseConnection {
public:
  virtual Message sendMessage(const Message &msg) = 0;
  virtual ~IDatabaseConnection() = default;
};

std::unique_ptr<IDatabaseConnection> connectToDatabase(const std::string &id);
