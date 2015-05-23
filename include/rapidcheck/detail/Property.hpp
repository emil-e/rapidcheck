#pragma once

#include "rapidcheck/detail/FunctionTraits.h"
#include "rapidcheck/gen/detail/ExecRaw.h"
#include "rapidcheck/detail/PropertyContext.h"

namespace rc {
namespace detail {

class WrapperContext : public PropertyContext {
public:
  explicit WrapperContext(std::vector<std::string> &tags);

  void addTag(std::string str) override;

private:
  Tags &m_tags;
};

std::ostream &operator<<(std::ostream &os, const CaseDescription &desc) {
  os << desc.result << std::endl;
  os << std::endl;
  for (const auto &p : desc.example) {
    os << p.first << ": " << p.second << std::endl;
  }
  os << std::endl;
  return os;
}

static inline CaseResult toCaseResultHelper(bool value) {
  return value ? CaseResult(CaseResult::Type::Success, "returned true")
               : CaseResult(CaseResult::Type::Failure, "returned false");
}

static inline CaseResult toCaseResultHelper(std::string value) {
  return value.empty()
      ? CaseResult(CaseResult::Type::Success, "empty string")
      : CaseResult(CaseResult::Type::Failure, std::move(value));
}

/// Helper class to convert different return types to `CaseResult`.
template <typename ReturnType>
struct CaseResultHelper {
  template <typename Callable, typename... Args>
  static CaseResult resultOf(const Callable &callable, Args &&... args) {
    return toCaseResultHelper(callable(std::forward<Args>(args)...));
  }
};

template <>
struct CaseResultHelper<void> {
  template <typename Callable, typename... Args>
  static CaseResult resultOf(const Callable &callable, Args &&... args) {
    callable(std::forward<Args>(args)...);
    return CaseResult(CaseResult::Type::Success, "no exceptions thrown");
  }
};

struct WrapperResult {
  CaseResult result;
  Tags tags;
};

template <typename Callable, typename Type = FunctionType<Callable>>
class PropertyWrapper;

template <typename Callable, typename ReturnType, typename... Args>
class PropertyWrapper<Callable, ReturnType(Args...)> {
public:
  template <typename Arg,
            typename = typename std::enable_if<
                !std::is_same<Decay<Arg>, PropertyWrapper>::value>::type>
  PropertyWrapper(Arg &&callable)
      : m_callable(std::forward<Arg>(callable)) {}

  WrapperResult operator()(Args &&... args) const {
    WrapperResult wrapperResult;
    WrapperContext handler(wrapperResult.tags);
    ImplicitParam<param::CurrentPropertyContext> letHandler(&handler);

    try {
      wrapperResult.result = CaseResultHelper<ReturnType>::resultOf(
          m_callable, static_cast<Args &&>(args)...);
    } catch (const CaseResult &result) {
      wrapperResult.result = result;
    } catch (const GenerationFailure &e) {
      wrapperResult.result = CaseResult(
          CaseResult::Type::Discard,
          std::string("Generation failed with message:\n") + e.what());
    } catch (const std::exception &e) {
      wrapperResult.result = CaseResult(
          CaseResult::Type::Failure,
          std::string("Exception thrown with message:\n") + e.what());
    } catch (const std::string &str) {
      wrapperResult.result = CaseResult(CaseResult::Type::Failure, str);
    } catch (...) {
      wrapperResult.result =
          CaseResult(CaseResult::Type::Failure, "Unknown object thrown");
    }

    return wrapperResult;
  }

private:
  Callable m_callable;
};

Gen<CaseDescription>
mapToCaseDescription(Gen<std::pair<WrapperResult, gen::detail::Recipe>> gen);

template <typename Callable>
Property toProperty(Callable &&callable) {
  using Wrapper = PropertyWrapper<Decay<Callable>>;
  return mapToCaseDescription(
      gen::detail::execRaw(Wrapper(std::forward<Callable>(callable))));
}

} // namespace detail
} // namespace rc

#include "Property.hpp"
