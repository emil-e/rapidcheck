#include "DatabaseConnection.h"

#include <map>

#include "User.h"

Message::Message(std::string t, std::vector<std::string> p)
    : type(std::move(t))
    , params(std::move(p)) {}

class DatabaseConnection : public IDatabaseConnection {
public:
  Message sendMessage(const Message &msg) override {
    if (msg.type == "put") {
      return doPut(msg.params);
    } else if (msg.type == "get") {
      return doGet(msg.params);
    } else {
      return Message("error", {"Unknown command"});
    }
  }

private:
  Message doPut(const std::vector<std::string> &params) {
    if (params.size() < 1) {
      return Message("error", {"Incorrect params"});
    }

    std::string expectedHash = params.front();
    std::hash<User> hasher;
    std::size_t hash = 0;
    std::vector<User> users;
    for (auto it = begin(params) + 1; it != end(params);) {
      User user;
      const auto next = deserialize(it, end(params), user);
      if (next == it) {
        return Message("error", {"Invalid user data"});
      }
      hash ^= hasher(user);
      users.push_back(std::move(user));
      it = next;
    }

    if (std::to_string(hash) == expectedHash) {
      for (const auto &user : users) {
        m_store[user.username] = user;
      }
    }
    return Message("ok");
  }

  Message doGet(const std::vector<std::string> &params) {
    if (params.size() != 1) {
      return Message("error", {"Incorrect params"});
    }

    const auto it = m_store.find(params[0]);
    if (it != m_store.end()) {
      Message msg("ok");
      serialize(it->second, msg.params);
      return msg;
    } else {
      return Message("not_found");
    }
  }

  std::map<std::string, User> m_store;
};

std::unique_ptr<IDatabaseConnection> connectToDatabase(const std::string &id) {
  return std::unique_ptr<IDatabaseConnection>(new DatabaseConnection());
}
