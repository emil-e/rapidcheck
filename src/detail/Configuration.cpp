#include "rapidcheck/detail/Configuration.h"

#include <random>
#include <cstdlib>

#include "MapParser.h"

namespace rc {
namespace detail {

std::ostream &operator<<(std::ostream &os, const Configuration &config) {
  os << configToString(config);
  return os;
}

bool operator==(const Configuration &c1, const Configuration &c2) {
  return (c1.seed == c2.seed) && (c1.maxSuccess == c2.maxSuccess) &&
      (c1.maxSize == c2.maxSize) && (c1.maxDiscardRatio == c2.maxDiscardRatio);
}

bool operator!=(const Configuration &c1, const Configuration &c2) {
  return !(c1 == c2);
}

ConfigurationException::ConfigurationException(std::string msg)
    : m_msg(std::move(msg)) {}

const char *ConfigurationException::what() const noexcept {
  return m_msg.c_str();
}

namespace {

// Returns false only on invalid format, not on missing key
template <typename T, typename Validator>
void loadParam(const std::map<std::string, std::string> &map,
               const std::string &key,
               T &dest,
               std::string failMsg,
               const Validator &validate) {
  auto it = map.find(key);
  if (it == end(map)) {
    return;
  }

  std::istringstream in(it->second);
  T value;
  in >> value;
  if (in.fail() || !validate(value)) {
    throw ConfigurationException(std::move(failMsg));
  }
  dest = value;
}

template <typename Validator>
bool loadParam(const std::map<std::string, std::string> &map,
               const std::string &key,
               std::string &dest,
               const std::string &failMsg,
               const Validator &validate = [](const std::string &) {
                 return true;
               }) {
  auto it = map.find(key);
  if (it != end(map)) {
    dest = it->second;
  }
  return true;
}

template <typename T>
bool isNonNegative(T x) {
  return x >= 0;
}

template <typename T>
bool anything(const T &) {
  return true;
}

Configuration configFromMap(const std::map<std::string, std::string> &map,
                            const Configuration &defaults) {
  Configuration config(defaults);

  loadParam(map,
            "seed",
            config.seed,
            "'seed' must be a valid integer",
            anything<uint64_t>);

  loadParam(map,
            "max_success",
            config.maxSuccess,
            "'max_success' must be a valid non-negative integer",
            isNonNegative<int>);

  loadParam(map,
            "max_size",
            config.maxSize,
            "'max_size' must be a valid non-negative integer",
            isNonNegative<int>);

  loadParam(map,
            "max_discard_ratio",
            config.maxDiscardRatio,
            "'max_discard_ratio' must be a valid non-negative integer",
            isNonNegative<int>);

  return config;
}

std::map<std::string, std::string> mapFromConfig(const Configuration &config) {
  return {{"seed", std::to_string(config.seed)},
          {"max_success", std::to_string(config.maxSuccess)},
          {"max_size", std::to_string(config.maxSize)},
          {"max_discard_ratio", std::to_string(config.maxDiscardRatio)}};
}

std::map<std::string, std::string>
mapDifference(const std::map<std::string, std::string> &lhs,
              const std::map<std::string, std::string> &rhs) {
  std::map<std::string, std::string> result;
  for (const auto &pair : lhs) {
    auto it = rhs.find(pair.first);
    if ((it == end(rhs)) || (pair.second != it->second)) {
      result.insert(pair);
    }
  }

  return result;
}

} // namespace

Configuration configFromString(const std::string &str,
                               const Configuration &defaults) {
  try {
    return configFromMap(parseMap(str), defaults);
  } catch (const ParseException &e) {
    throw ConfigurationException(
        std::string("Failed to parse configuration string -- ") + e.what());
  }
}

std::string configToString(const Configuration &config) {
  return mapToString(mapFromConfig(config));
}

std::string configToMinimalString(const Configuration &config) {
  auto defaults = mapFromConfig(Configuration());
  // Remove keys that we always want to specify
  defaults.erase("seed");
  return mapToString(mapDifference(mapFromConfig(config), defaults));
}

namespace {

Configuration loadConfiguration() {
  Configuration config;
  // Default to random seed
  std::random_device device;
  config.seed = (static_cast<uint64_t>(device()) << 32) | device();

  auto params = std::getenv("RC_PARAMS");
  if (params != nullptr) {
    config = configFromString(params, config);
  }

  // TODO rapidcheck logging framework ftw
  std::cerr << "Using configuration: " << configToMinimalString(config)
            << std::endl;
  return config;
}

} // namespace

const Configuration &defaultConfiguration() {
  static Configuration config = loadConfiguration();
  return config;
}

} // namespace detail
} // namespace rc
