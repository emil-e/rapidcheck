#pragma once

#include <string>

#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

/// A `PropertyContext` is the hidden interface with which different actions in
/// properties communicate with the framework.
class PropertyContext {
public:
  /// Reports a result.
  virtual void reportResult(const CaseResult &result) = 0;

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
