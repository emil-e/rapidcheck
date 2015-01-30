#include "rapidcheck/detail/Configuration.h"

#include <random>

namespace rc {
namespace detail {

namespace {

Configuration loadConfiguration()
{
    Configuration config;
    std::random_device device;
    config.seed = (static_cast<uint64_t>(device()) << 32) | device();
    // TODO rapidcheck logging framework ftw
    std::cerr << "Using seed " << config.seed << std::endl;
    return config;
}

} // namespace

const Configuration &defaultConfiguration()
{
    static Configuration config = loadConfiguration();
    return config;
}

} // namespace detail
} // namespace rc
