#include <rapidcheck.h>

#include "Database.h"
#include "DatabaseConnection.h"

using namespace rc;

struct DatabaseModel {
  bool open = false;
  bool inWriteBlock = false;
  std::map<std::string, std::string> data;
};

using DbCommand = state::Command<DatabaseModel, Database>;

struct Open : public DbCommand {
  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(!s0.open);
    auto s1 = s0;
    s1.open = true;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.open();
  }

  void show(std::ostream &os) const override {
    os << "Open";
  }
};

struct Close : public DbCommand {
  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(!s0.inWriteBlock);
    RC_PRE(s0.open);
    auto s1 = s0;
    s1.open = false;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.close();
  }

  void show(std::ostream &os) const override {
    os << "Close";
  }
};

struct Put : public DbCommand {
  std::string key = *gen::arbitrary<std::string>();
  std::string value = *gen::arbitrary<std::string>();

  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.inWriteBlock);
    auto s1 = s0;
    s1.data[key] = value;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.put(key, value);
  }

  void show(std::ostream &os) const override {
    os << "Put(" << toString(key) << ", " << toString(value) << ")";
  }
};

struct GetExisting : public DbCommand {
  std::string key;

  explicit GetExisting(const DatabaseModel &s0) {
    std::vector<std::string> keys;
    for (const auto &p : s0.data) {
      keys.push_back(p.first);
    }

    key = *gen::elementOf(keys);
  }

  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(!s0.inWriteBlock);
    RC_PRE(s0.data.count(key) > 0);
    return s0;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    std::string value;
    RC_ASSERT(db.get(key, value));
    RC_ASSERT(value == s0.data.at(key));
  }

  void show(std::ostream &os) const override {
    os << "GetExisting(" << toString(key) << ")";
  }
};

struct BeginWrite : public DbCommand {
  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(!s0.inWriteBlock);
    auto s1 = s0;
    s1.inWriteBlock = true;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.beginWrite();
  }

  void show(std::ostream &os) const override {
    os << "BeginWrite";
  }
};

struct ExecuteWrite : public DbCommand {
  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.inWriteBlock);
    auto s1 = s0;
    s1.inWriteBlock = false;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.executeWrite();
  }

  void show(std::ostream &os) const override {
    os << "ExecuteWrite";
  }
};

int main() {
  check([] {
    DatabaseModel s0;
    Database db(connectToDatabase("localhost"));
    state::check(s0,
                 db,
                 &state::anyCommand<Open,
                                    Close,
                                    Put,
                                    BeginWrite,
                                    ExecuteWrite,
                                    GetExisting>);
  });
  return 0;
}
