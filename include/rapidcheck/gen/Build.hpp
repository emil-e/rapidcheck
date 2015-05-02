#pragma once

#include "rapidcheck/detail/ApplyTuple.h"

namespace rc {
namespace gen {
namespace detail {

template <typename T>
class Lens;

// Member variables
template <typename Type, typename T>
class Lens<T(Type::*)> {
public:
  typedef T(Type::*MemberPtr);
  typedef Type Target;
  typedef T ValueType;

  Lens(MemberPtr ptr)
      : m_ptr(ptr) {}

  void set(Target &obj, T &&arg) const { obj.*m_ptr = std::move(arg); }

private:
  MemberPtr m_ptr;
};

// Member functions with single argument
template <typename Type, typename Ret, typename T>
class Lens<Ret (Type::*)(T)> {
public:
  typedef Ret (Type::*MemberPtr)(T);
  typedef Type Target;
  typedef Decay<T> ValueType;

  Lens(MemberPtr ptr)
      : m_ptr(ptr) {}

  void set(Target &obj, ValueType &&arg) const { (obj.*m_ptr)(std::move(arg)); }

private:
  MemberPtr m_ptr;
};

// Member functions with multiple arguments
template <typename Type, typename Ret, typename T1, typename T2, typename... Ts>
class Lens<Ret (Type::*)(T1, T2, Ts...)> {
public:
  typedef Ret (Type::*MemberPtr)(T1, T2, Ts...);
  typedef Type Target;
  typedef std::tuple<Decay<T1>, Decay<T2>, Decay<Ts>...> ValueType;

  Lens(MemberPtr ptr)
      : m_ptr(ptr) {}

  void set(Target &obj, ValueType &&arg) const {
    rc::detail::applyTuple(
        std::move(arg),
        [&](Decay<T1> &&arg1, Decay<T2> &&arg2, Decay<Ts> &&... args) {
          (obj.*m_ptr)(std::move(arg1), std::move(arg2), std::move(args)...);
        });
  }

private:
  MemberPtr m_ptr;
};

template <typename Member>
struct Binding {
  using LensT = Lens<Member>;
  using Target = typename LensT::Target;
  using ValueType = typename LensT::ValueType;
  using GenT = Gen<ValueType>;

  Binding(LensT &&l, GenT &&g)
      : lens(std::move(l))
      , gen(std::move(g)) {}

  LensT lens;
  GenT gen;
};

} // namespace detail

template <typename T, typename... Args>
Gen<T> construct(Gen<Args>... gens) {
  return gen::map(gen::tuple(std::move(gens)...),
                  [](std::tuple<Args...> &&argsTuple) {
                    return rc::detail::applyTuple(
                        std::move(argsTuple),
                        [](Args &&... args) { return T(std::move(args)...); });
                  });
}

template <typename T, typename Arg, typename... Args>
Gen<T> construct() {
  return gen::construct<T>(gen::arbitrary<Arg>(), gen::arbitrary<Args>()...);
}

template <typename T, typename... Args>
Gen<std::unique_ptr<T>> makeUnique(Gen<Args>... gens) {
  return gen::map(gen::tuple(std::move(gens)...),
                  [](std::tuple<Args...> &&argsTuple) {
                    return rc::detail::applyTuple(
                        std::move(argsTuple),
                        [](Args &&... args) {
                          return std::unique_ptr<T>(new T(std::move(args)...));
                        });
                  });
}

template <typename Member>
detail::Binding<Member> set(Member member,
                            typename detail::Binding<Member>::GenT gen) {
  return detail::Binding<Member>(detail::Lens<Member>(member), std::move(gen));
}

template <typename Member>
detail::Binding<Member> set(Member member) {
  using T = typename detail::Binding<Member>::ValueType;
  return set(member, gen::arbitrary<T>());
}

template <typename T, typename... Members>
Gen<T> build(Gen<T> gen, const detail::Binding<Members> &... bs) {
  using Tuple = std::tuple<T, typename detail::Binding<Members>::ValueType...>;
  return gen::map(
      gen::tuple(std::move(gen), std::move(bs.gen)...),
      [=](Tuple &&tuple) {
        return rc::detail::applyTuple(
            std::move(tuple),
            [&](T &&obj,
                typename detail::Binding<Members>::ValueType &&... vs) {
              // We use the comma operator to give the expression a
              // return type other than void so it can be expanded
              // in an initializer list.
              auto dummy{(bs.lens.set(obj, std::move(vs)), 0)...};
              return std::move(obj);
            });
      });
}

template <typename T, typename... Members>
Gen<T> build(const detail::Binding<Members> &... bs) {
  return build<T>(fn::constant(shrinkable::lambda([] { return T(); })), bs...);
}

} // namespace gen
} // namespace rc
