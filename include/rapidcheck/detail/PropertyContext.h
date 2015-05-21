#pragma once

#include <string>

namespace rc {
namespace detail {

/// A `PropertyContext` is the hidden interface with which different actions in
/// properties communicate with the framework.
class PropertyContext {
public:
  /// Adds a tag to the current scope.
  virtual void addTag(std::string str) = 0;

  virtual ~PropertyContext() = default;
};

namespace param {

struct CurrentPropertyContext {
  using ValueType = PropertyContext *;
  static PropertyContext *defaultValue();
};

} // namespace param
} // namespace detail
} // namespace rc
