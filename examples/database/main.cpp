#include <rapidcheck.h>
#include <rapidcheck/state.h>

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
  void apply(DatabaseModel &s0) const override {
    RC_PRE(!s0.open);
    s0.open = true;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.open();
  }

  void show(std::ostream &os) const override {
    os << "Open";
  }
};

struct Close : public DbCommand {
  void apply(DatabaseModel &s0) const override {
    RC_PRE(!s0.inWriteBlock);
    RC_PRE(s0.open);
    s0.open = false;
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

  void apply(DatabaseModel &s0) const override {
    RC_PRE(s0.inWriteBlock);
    s0.data[user.username] = user;
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

  void apply(DatabaseModel &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(!s0.inWriteBlock);
    RC_PRE(s0.data.count(username) > 0);
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
  void apply(DatabaseModel &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(!s0.inWriteBlock);
    s0.inWriteBlock = true;
  }

  void run(const DatabaseModel &s0, Database &db) const override {
    db.beginWrite();
  }

  void show(std::ostream &os) const override {
    os << "BeginWrite";
  }
};

struct ExecuteWrite : public DbCommand {
  void apply(DatabaseModel &s0) const override {
    RC_PRE(s0.inWriteBlock);
    s0.inWriteBlock = false;
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
    state::check(
        s0,
        db,
        &state::gen::
            execOneOf<Open, Close, Put, BeginWrite, ExecuteWrite, Get>);
  });
  return 0;
}
