#pragma once

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename... Ts>
using TypeList = ::rc::detail::TypeList<Ts...>;

template <typename Cmd, typename Args>
struct MakeCommand;

template <typename Cmd, typename... Args>
struct MakeCommand<Cmd, TypeList<Args...>> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const Args &... args) {
    return make(std::is_constructible<Cmd, Args...>(), args...);
  }

  static std::shared_ptr<const typename Cmd::CommandType>
  make(std::true_type, const Args &... args) {
    return std::make_shared<Cmd>(args...);
  }

  static std::shared_ptr<const typename Cmd::CommandType>
  make(std::false_type, const Args &... args) {
    return std::make_shared<Cmd>();
  }
};

template <typename Cmd, typename... Cmds>
class ExecOneOf {
private:
  using CmdSP = std::shared_ptr<const typename Cmd::CommandType>;

public:
  template <typename... Args>
  Gen<CmdSP> operator()(const Args &... args) const {
    using MakeFunc = CmdSP (*)(const Args &...);
    using ArgsList = TypeList<Args...>;
    static const MakeFunc makeFuncs[] = {&MakeCommand<Cmd, ArgsList>::make,
                                         &MakeCommand<Cmds, ArgsList>::make...};

    return [=](const Random &random, int size) {
      auto r = random;
      std::size_t n = r.split().next() % (sizeof...(Cmds) + 1);
      return rc::gen::exec([=] { return makeFuncs[n](args...); })(
          r, size); // TODO monadic bind
    };
  }
};

} // namespace detail

template <typename Cmd, typename... Cmds>
Gen<std::shared_ptr<const typename Cmd::CommandType>>
execOneOf(const typename Cmd::Model &state) {
  static detail::ExecOneOf<Cmd, Cmds...> execOneOfObject;
  return execOneOfObject(state);
}

template <typename Cmd, typename... Cmds>
detail::ExecOneOf<Cmd, Cmds...> execOneOfWithArgs() {
  return detail::ExecOneOf<Cmd, Cmds...>();
}

} // namespace gen
} // namespace state
} // namespace rc
