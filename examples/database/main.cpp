#include <rapidcheck.h>

#include "Database.h"
#include "DatabaseConnection.h"
#include "Generators.h"

using namespace rc;

struct DatabaseModel {
  bool open = false;
  bool inWriteBlock = false;
  std::map<std::string, User> data;
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
  User user = *gen::arbitrary<User>();

  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.inWriteBlock);
    auto s1 = s0;
    s1.data[user.username] = user;
    return s1;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.put(user);
  }

  void show(std::ostream &os) const override {
    os << "Put(" << user << ")";
  }
};

struct Get : public DbCommand {
  std::string username;

  explicit Get(const DatabaseModel &s0) {
    username = (*gen::elementOf(s0.data)).second.username;
  }

  DatabaseModel nextState(const DatabaseModel &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(!s0.inWriteBlock);
    RC_PRE(s0.data.count(username) > 0);
    return s0;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    User user;
    RC_ASSERT(db.get(username, user));
    RC_ASSERT(user == s0.data.at(username));
  }

  void show(std::ostream &os) const override {
    os << "Get(" << toString(username) << ")";
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
                                    Get>);
  });
  return 0;
}
