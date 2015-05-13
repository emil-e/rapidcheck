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
  for (const auto &p : m_queue) {
    msg.params.push_back(p.first);
    msg.params.push_back(p.second);
  }

  const auto response = m_connection->sendMessage(msg);
  if (response.type == "ok") {
    for (const auto &p : m_queue) {
      m_cache[p.first] = p.second;
      // m_cache.insert(p);
    }
    m_hasBlock = false;
  } else {
    throw std::runtime_error("Invalid response");
  }
}

void Database::put(const std::string &key, const std::string &value) {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (!m_hasBlock) {
    throw std::runtime_error("beginWrite wasn't called");
  }

  m_queue.emplace_back(key, value);
  std::hash<std::string> hasher;
  m_hash ^= hasher(key);
  m_hash ^= hasher(value);
}

bool Database::get(const std::string &key, std::string &value) {
  if (!m_open) {
    throw std::runtime_error("Not open");
  }
  if (m_hasBlock) {
    throw std::runtime_error("Currently in write");
  }

  const auto it = m_cache.find(key);
  if (it != m_cache.end()) {
    value = it->second;
    return true;
  }

  const auto response = m_connection->sendMessage(Message("get", {key}));
  if ((response.type == "ok") && (response.params.size() == 1)) {
    value = response.params[0];
    return true;
  } else if (response.type == "not_found") {
    return false;
  } else {
    throw std::runtime_error("Invalid response");
  }
}
