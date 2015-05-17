#include "Database.h"

#include <iostream>

#include "DatabaseConnection.h"

Database::Database(std::unique_ptr<IDatabaseConnection> connection)
    : m_open(false)
    , m_connection(std::move(connection))
    , m_hasBlock(false)
    , m_hash(0) {}

void Database::open() {
  if (m_open) {
    throw std::runtime_error("Already open");
  }

  m_open = true;
}

void Database::close() {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }

  m_cache.clear();
  m_open = false;
}

void Database::beginWrite() {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (m_hasBlock) {
    throw std::runtime_error("Already called beginWrite");
  }

  m_queue.clear();
  m_hasBlock = true;
}

void Database::executeWrite() {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (!m_hasBlock) {
    throw std::runtime_error("beginWrite wasn't called");
  }

  auto msg = Message("put");
  msg.params.push_back(std::to_string(m_hash));
  for (const auto &user : m_queue) {
    serialize(user, msg.params);
  }

  const auto response = m_connection->sendMessage(msg);
  if (response.type == "ok") {
    for (const auto &user : m_queue) {
      m_cache.emplace(user.username, user);
    }
    m_hasBlock = false;
  } else {
    throw std::runtime_error("Invalid response");
  }
}

void Database::put(User user) {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (!m_hasBlock) {
    throw std::runtime_error("beginWrite wasn't called");
  }

  m_hash ^= std::hash<User>()(user);
  m_queue.push_back(std::move(user));
}

bool Database::get(const std::string &username, User &user) {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (m_hasBlock) {
    throw std::runtime_error("Currently in write");
  }

  const auto it = m_cache.find(username);
  if (it != m_cache.end()) {
    user = it->second;
    return true;
  }

  const auto response = m_connection->sendMessage(Message("get", {username}));
  if (response.type == "ok") {
    const auto it =
        deserialize(begin(response.params), end(response.params), user);
    if (it == begin(response.params)) {
      throw std::runtime_error("Invalid response");
    }
    return true;
  } else if (response.type == "not_found") {
    return false;
  } else {
    throw std::runtime_error("Invalid response");
  }
}
