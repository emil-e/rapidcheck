#pragma once

#include <string>
#include <tuple>
#include <type_traits>
#include <limits>

namespace rc {
namespace detail {

/// Disables copying
#define RC_DISABLE_COPY(Class)                                                 \
  Class(const Class &) = delete;                                               \
  Class &operator=(const Class &) = delete;

/// Disables moving
#define RC_DISABLE_MOVE(Class)                                                 \
  Class(Class &&) = delete;                                                    \
  Class &operator=(Class &&) = delete;

#define RC_GLUE2(a, b) a##b

/// Paste together the given arguments.
#define RC_GLUE(a, b) RC_GLUE2(a, b)

/// Unique identifier helper.
#define RC_UNIQUE(prefix) RC_GLUE(prefix, __LINE__)

/// Base case for `pushBackAll`
template <typename Collection>
void pushBackAll(Collection &/*collection*/) {
  // Base case
}

/// Appends the given items to the end of the given collection.
///
/// @param collection   The collection to append to.
/// @param item         The first item.
/// @param items        The rest of the items.
template <typename Collection, typename Item, typename... Items>
void pushBackAll(Collection &collection, Item &&item, Items &&... items) {
  collection.push_back(std::forward<Item>(item));
  pushBackAll(collection, std::forward<Items>(items)...);
}

/// Base case for `join`.
inline std::string join(const std::string &/*sep*/, const std::string str) {
  return str;
}

/// Joins the given strings using the given separator.
///
/// @param sep      The separator.
/// @param str      The first string.
/// @param strings  The rest of the strings.
template <typename... Strings>
std::string
join(const std::string &sep, const std::string &str, Strings... strings) {
  return str + sep + join(sep, strings...);
}

template <typename TupleT, typename... Types>
struct TupleTailHelper;

template <typename TupleT>
struct TupleTailHelper<TupleT> {
  static std::tuple<> tail(const TupleT &/*tuple*/) { return std::tuple<>(); }
};

template <typename TupleT, typename Type, typename... Types>
struct TupleTailHelper<TupleT, Type, Types...> {
  static std::tuple<Type, Types...> tail(const TupleT &tuple) {
    constexpr std::size_t tailHead =
        std::tuple_size<TupleT>::value - (sizeof...(Types) + 1);
    return std::tuple_cat(std::make_tuple(std::get<tailHead>(tuple)),
                          TupleTailHelper<TupleT, Types...>::tail(tuple));
  }
};

/// Returns a copy of the given tuple without the first element.
template <typename Type, typename... Types>
std::tuple<Types...> tupleTail(const std::tuple<Type, Types...> &tuple) {
  return TupleTailHelper<std::tuple<Type, Types...>, Types...>::tail(tuple);
}

/// Avalanching function.
/// Written in 2014 by Sebastiano Vigna (vigna@acm.org)
inline uint64_t avalanche(uint64_t x) {
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  return x ^= x >> 33;
}

/// Returns a bitmask of the given type with the lowest `nbits` bits set to 1
/// and the rest set to 0.
template <typename T>
constexpr T bitMask(int nbits) {
  using UT = typename std::make_unsigned<T>::type;
  using UTP = typename std::common_type<UT, unsigned>::type;
  // There are two pieces of undefined behavior we're avoiding here,
  //   1. Shifting past the width of a type (ex `<< 32` against an `int32_t`)
  //   2. Shifting a negative operand (which `~0` is for all signed types)
  // First we branch to avoid shifting the past the width of the type, then
  // (assuming we are shifting, and aren't just returning `~0`) we cast `0`
  // to an explicitly unsigned type before performing bitwise NOT and shift.
  // We're ensuring the target type is as big as unsigned, otherwise it will
  // be promoted to int before bitwise NOT, producing a negative value.
  return nbits < std::numeric_limits<UT>::digits ?
         ~T(~UTP(0) << nbits)                    :
         ~T(0);
}

/// Thrown by sign conversion functions on failure.
class SignException : public std::exception {
public:
  /// C-tor.
  ///
  /// @param msg  A message describing the sign error.
  SignException(const std::string &msg) : m_msg(msg) {}

  std::string message() const { return m_msg; }

  const char *what() const noexcept override {
    return m_msg.c_str();
  }


private:
  std::string m_msg;
};

/// Casts a value from a signed type to an unsigned type
/// Raises a SignException if the narrowing would change the value
template<typename NarrowFrom>
typename std::make_unsigned<NarrowFrom>::type makeUnsigned(NarrowFrom value) {
  static_assert(std::is_integral<NarrowFrom>::value);
  static_assert(std::is_signed<NarrowFrom>::value);

  if (value < 0) {
      throw SignException("Narrowing value below target range");
  }

  return static_cast<typename std::make_unsigned<NarrowFrom>::type>(value);
}

/// Casts a value from an unsigned type to a signed type
/// Raises a SignException if the narrowing would change the value
template<typename NarrowFrom>
typename std::make_signed<NarrowFrom>::type makeSigned(NarrowFrom value) {
  using DestType = typename std::make_signed<NarrowFrom>::type;

  static_assert(std::is_integral<NarrowFrom>::value);
  static_assert(std::is_unsigned<NarrowFrom>::value);

  if (value > static_cast<NarrowFrom>(std::numeric_limits<DestType>::max())) {
      throw SignException("Narrowing value above target range");
  }

  return static_cast<DestType>(value);
}

// TODO separate into header and implementation file

} // namespace detail
} // namespace rc
