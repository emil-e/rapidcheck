#include "rapidcheck/Assertions.h"

namespace rc {
namespace detail {

std::string makeDescriptionMessage(const std::string &file,
                                   int line,
                                   const std::string &description) {
  return file + ":" + std::to_string(line) + ":\n" + description;
}

std::string makeExpressionMessage(const std::string &file,
                                  int line,
                                  const std::string &assertion,
                                  const std::string &expansion) {
  return makeDescriptionMessage(file, line, assertion) +
      "\n"
      "\n"
      "Expands to:\n" +
      expansion;
}

std::string makeUnthrownExceptionMessage(const std::string &file,
                                         int line,
                                         const std::string &assertion) {
  return makeDescriptionMessage(file, line, assertion) +
      "\n"
      "\n"
      "No exception was thrown.";
}

std::string makeWrongExceptionMessage(const std::string &file,
                                      int line,
                                      const std::string &assertion,
                                      const std::string &expected) {
  return makeDescriptionMessage(file, line, assertion) +
      "\n"
      "\n"
      "Thrown exception did not match " +
      expected + ".";
}

} // namespace detail
} // namespace rc
