#pragma once

#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/seq/Transform.h"

namespace rc {
namespace shrinkable {
namespace detail {

template <typename T, typename Mapper>
class MapShrinkable {
public:
  using U = Decay<typename std::result_of<Mapper(T)>::type>;

  template <typename MapperArg>
  MapShrinkable(Shrinkable<T> shrinkable, MapperArg &&mapper)
      : m_mapper(std::forward<MapperArg>(mapper))
      , m_shrinkable(std::move(shrinkable)) {}

  U value() const { return m_mapper(m_shrinkable.value()); }

  Seq<Shrinkable<U>> shrinks() const {
    auto mapper = m_mapper;
    return seq::map(m_shrinkable.shrinks(),
                    [=](Shrinkable<T> &&shrink) {
                      return shrinkable::map(std::move(shrink), mapper);
                    });
  }

private:
  Mapper m_mapper;
  Shrinkable<T> m_shrinkable;
};

template <typename T, typename Mapper>
class MapShrinksShrinkable {
public:
  template <typename MapperArg>
  MapShrinksShrinkable(Shrinkable<T> shrinkable, MapperArg &&mapper)
      : m_mapper(std::forward<MapperArg>(mapper))
      , m_shrinkable(std::move(shrinkable)) {}

  T value() const { return m_shrinkable.value(); }

  Seq<Shrinkable<T>> shrinks() const {
    return m_mapper(m_shrinkable.shrinks());
  }

private:
  Mapper m_mapper;
  Shrinkable<T> m_shrinkable;
};

template <typename T, typename Callable>
class MapcatShrinkable {
public:
  using U = typename std::result_of<Callable(T)>::type::ValueType;

  template <typename CallableArg>
  MapcatShrinkable(Shrinkable<T> shrinkable, CallableArg &&callable)
      : m_shrinkable(std::move(shrinkable))
      , m_callable(std::forward<CallableArg>(callable)) {}

  U value() const { return m_callable(m_shrinkable.value()).value(); }

  Seq<Shrinkable<U>> shrinks() const {
    const auto callable = m_callable;
    return seq::concat(seq::map(m_shrinkable.shrinks(),
                                [=](Shrinkable<T> &&s) {
                                  return shrinkable::mapcat(std::move(s),
                                                            callable);
                                }),
                       m_callable(m_shrinkable.value()).shrinks());
  }

private:
  Shrinkable<T> m_shrinkable;
  Callable m_callable;
};

} // namespace detail

template <typename T, typename Mapper>
Shrinkable<Decay<typename std::result_of<Mapper(T)>::type>>
map(Shrinkable<T> shrinkable, Mapper &&mapper) {
  using Impl = detail::MapShrinkable<T, Decay<Mapper>>;
  return makeShrinkable<Impl>(std::move(shrinkable),
                              std::forward<Mapper>(mapper));
}

template <typename T, typename Mapper>
Shrinkable<T> mapShrinks(Shrinkable<T> shrinkable, Mapper &&mapper) {
  using Impl = detail::MapShrinksShrinkable<T, Decay<Mapper>>;
  return makeShrinkable<Impl>(std::move(shrinkable),
                              std::forward<Mapper>(mapper));
}

template <typename T, typename Predicate>
Maybe<Shrinkable<T>> filter(Shrinkable<T> shrinkable, Predicate &&pred) {
  if (!pred(shrinkable.value())) {
    return Nothing;
  }

  return shrinkable::mapShrinks(
      std::move(shrinkable),
      [=](Seq<Shrinkable<T>> &&shrinks) {
        return seq::mapMaybe(std::move(shrinks),
                             [=](Shrinkable<T> &&shrink) {
                               return shrinkable::filter(std::move(shrink),
                                                         pred);
                             });
      });
}

template <typename T, typename Callable>
Shrinkable<typename std::result_of<Callable(T)>::type::ValueType>
mapcat(Shrinkable<T> shrinkable, Callable &&callable) {
  using Impl = detail::MapcatShrinkable<T, Decay<Callable>>;
  return makeShrinkable<Impl>(std::move(shrinkable),
                              std::forward<Callable>(callable));
}

template <typename T1, typename T2>
Shrinkable<std::pair<T1, T2>> pair(Shrinkable<T1> s1, Shrinkable<T2> s2) {
  return shrinkable::map(
      shrinkable::shrinkRecur(
          std::make_pair(s1, s2),
          [](const std::pair<Shrinkable<T1>, Shrinkable<T2>> &p) {
            return seq::concat(
                seq::map(p.first.shrinks(),
                         [=](Shrinkable<T1> &&s) {
                           return std::make_pair(std::move(s), p.second);
                         }),
                seq::map(p.second.shrinks(),
                         [=](Shrinkable<T2> &&s) {
                           return std::make_pair(p.first, std::move(s));
                         }));
          }),
      [](const std::pair<Shrinkable<T1>, Shrinkable<T2>> &p) {
        return std::make_pair(p.first.value(), p.second.value());
      });
}

} // namespace shrinkable
} // namespace rc
