#pragma once

namespace rc {
namespace gen {
namespace detail {

template <typename Container>
class ElementOfGen {
public:
  typedef typename Container::value_type T;

  template <typename Arg>
  ElementOfGen(Arg &&arg)
      : m_container(std::forward<Arg>(arg)) {}

  Shrinkable<T> operator()(const Random &random, int size) const {
    const auto start = begin(m_container);
    const auto containerSize = end(m_container) - start;
    if (containerSize == 0) {
      throw GenerationFailure("Cannot pick element from empty container.");
    }
    const auto i = Random(random).next() % containerSize;
    return shrinkable::just(*(start + i));
  }

private:
  Container m_container;
};

template <typename T>
class OneOfGen {
public:
  template <typename... Ts>
  OneOfGen(Gen<Ts>... gens)
      : m_gens{std::move(gens)...} {}

  Shrinkable<T> operator()(const Random &random, int size) const {
    Random r(random);
    const auto i = r.split().next() % m_gens.size();
    return m_gens[i](r, size);
  }

private:
  std::vector<Gen<T>> m_gens;
};

} // namespace detail

template <typename Container>
Gen<typename Decay<Container>::value_type> elementOf(Container &&container) {
  return detail::ElementOfGen<Decay<Container>>(
      std::forward<Container>(container));
}

template <typename T, typename... Ts>
Gen<Decay<T>> element(T &&element, Ts &&... elements) {
  using Vector = std::vector<Decay<T>>;
  return detail::ElementOfGen<Vector>(
      Vector{std::forward<T>(element), std::forward<Ts>(elements)...});
}

template <typename T, typename... Ts>
Gen<T> oneOf(Gen<T> gen, Gen<Ts>... gens) {
  return detail::OneOfGen<T>(std::move(gen), std::move(gens)...);
}

} // namespace gen
} // namespace rc
