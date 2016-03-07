#pragma once

namespace rc {
namespace test {

struct NonCopyableModel {
  int value = 0;

  NonCopyableModel() = default;
  NonCopyableModel(const NonCopyableModel &) = delete;
  NonCopyableModel &operator=(const NonCopyableModel &) = delete;
  NonCopyableModel(NonCopyableModel &&) = default;
  NonCopyableModel &operator=(NonCopyableModel &&) = default;
};

using NonCopyableCmd = state::Command<NonCopyableModel, NonCopyableModel>;

struct NonCopyableInc : public NonCopyableCmd {
  void checkPreconditions(const NonCopyableModel &model) const override {
    RC_PRE(model.value < 20);
  }

  void apply(NonCopyableModel &model) const override { model.value++; }

  void run(const NonCopyableModel &s0, NonCopyableModel &sut) const override {
    sut.value++;
  }
};

struct NonCopyableDec : public NonCopyableCmd {
  void checkPreconditions(const NonCopyableModel &model) const override {
    RC_PRE(model.value > 0);
  }

  void apply(NonCopyableModel &model) const override { model.value--; }

  void run(const NonCopyableModel &s0, NonCopyableModel &sut) const override {
    sut.value--;
  }
};

inline NonCopyableModel initialNonCopyableModel() { return NonCopyableModel(); }

inline Gen<state::Commands<NonCopyableCmd>> genNonCopyableCommands() {
  return state::gen::commands(
      &initialNonCopyableModel,
      [](const NonCopyableModel &model) {
        return state::gen::execOneOfWithArgs<NonCopyableInc, NonCopyableDec>()(
            model.value);
      });
}

} // namespace test
} // namespace rc
