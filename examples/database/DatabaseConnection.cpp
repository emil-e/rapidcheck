#include "DatabaseConnection.h"

#include <map>

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
    if ((params.size() % 2) != 1) {
      return Message("error", {"Incorrect params"});
    }

    auto elements = params;
    std::string expectedHash = elements.front();
    elements.erase(begin(elements));
    std::hash<std::string> hasher;
    std::size_t hash = 0;
    std::vector<std::pair<std::string, std::string>> pairs;
    for (std::size_t i = 0; i < (elements.size() / 2); i++) {
      const auto key = elements[i * 2];
      const auto value = elements[(i * 2) + 1];
      pairs.emplace_back(key, value);
      hash ^= hasher(key);
      hash ^= hasher(value);
    }

    if (std::to_string(hash) == expectedHash) {
      for (const auto &p : pairs) {
        m_store[p.first] = p.second;
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
      return Message("ok", {it->second});
    } else {
      return Message("not_found");
    }
  }

  std::map<std::string, std::string> m_store;
};

std::unique_ptr<IDatabaseConnection> connectToDatabase(const std::string &id) {
  return std::unique_ptr<IDatabaseConnection>(new DatabaseConnection());
}
