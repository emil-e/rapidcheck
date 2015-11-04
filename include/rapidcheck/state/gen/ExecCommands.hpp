#pragma once

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmd,
          typename =
              typename std::is_constructible<Cmd, typename Cmd::Model &&>::type>
struct MakeCommand;

template <typename Cmd>
struct MakeCommand<Cmd, std::true_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::Model &state) {
    return std::make_shared<Cmd>(state);
  }
};

template <typename Cmd>
struct MakeCommand<Cmd, std::false_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::Model &state) {
    return std::make_shared<Cmd>();
  }
  static std::shared_ptr<const typename Cmd::CommandType>
  make() {
    return std::make_shared<Cmd>();
  }
};

} // namespace detail

template <typename Cmd, typename... Cmds>
Gen<std::shared_ptr<const typename Cmd::CommandType>>
execOneOf(const typename Cmd::Model &state) {
  using CmdSP = std::shared_ptr<const typename Cmd::CommandType>;
  using State = typename Cmd::Model;
  using MakeFunc = CmdSP (*)(const State &);
  static const MakeFunc makeFuncs[] = {&detail::MakeCommand<Cmd>::make,
                                       &detail::MakeCommand<Cmds>::make...};

  return [=](const Random &random, int size) {
    auto r = random;
    std::size_t n = r.split().next() % (sizeof...(Cmds) + 1);
    return rc::gen::exec([=] { return makeFuncs[n](state); })(
        r, size); // TODO monadic bind
  };
}

} // namespace gen
} // namespace state
} // namespace rc
