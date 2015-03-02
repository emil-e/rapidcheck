#pragma once

#include <catch.hpp>

namespace rc {
namespace test {

//! Utility class to test copying, moving et.c.
struct Logger
{
public:
    Logger()
        : log{"default constructed"} {}

    Logger(std::string theId)
        : id(std::move(theId))
        , log{"constructed as " + id} {}

    Logger(const Logger &other)
        : id(other.id)
        , log(other.log)
    { log.emplace_back("copy constructed"); }

    Logger(Logger &&other)
        : id(std::move(other.id))
        , log(std::move(other.log))
    { log.emplace_back("move constructed"); }

    Logger &operator=(const Logger &rhs)
    {
        id = rhs.id;
        log = rhs.log;
        log.emplace_back("copy assigned");
        return *this;
    }

    Logger &operator=(Logger &&rhs)
    {
        id = std::move(rhs.id);
        log = std::move(rhs.log);
        log.emplace_back("move assigned");
        return *this;
    }

    template<typename ...Args>
    void requireExact(Args &&...args)
    {
        std::vector<std::string> expected {
            std::forward<Args>(args)...
        };
        REQUIRE(log == expected);
    }

    bool hasLogItem(const std::string &item)
    { return std::find(begin(log), end(log), item) != end(log); }

    int numberOf(const std::string &value) const
    { return std::count(begin(log), end(log), value); }

    virtual ~Logger() = default;

    std::string id;
    std::vector<std::string> log;
};


} // namespace test
} // namespace rc
