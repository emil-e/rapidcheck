#pragma once

#include <catch.hpp>

namespace rc {
namespace test {

//! Utility class to test copying, moving et.c.
struct Logger
{
public:
    Logger() noexcept
    {
        try {
            log.push_back("default constructed");
        } catch(...) {}
    }

    Logger(std::string theId) noexcept
    {
        try {
            id = std::move(theId);
            log.push_back("constructed as " + id);
        } catch(...) {}
    }

    Logger(const Logger &other) noexcept
    {
        try {
            id = other.id;
            log = other.log;
            log.emplace_back("copy constructed");
        } catch(...) {}
    }

    Logger(Logger &&other) noexcept
    {
        try {
            id = std::move(other.id);
            log = std::move(other.log);
            log.emplace_back("move constructed");
        } catch(...) {}
    }

    Logger &operator=(const Logger &rhs) noexcept
    {
        try {
            id = rhs.id;
            log = rhs.log;
            log.emplace_back("copy assigned");
        } catch(...) {}
        return *this;
    }

    Logger &operator=(Logger &&rhs) noexcept
    {
        try {
            id = std::move(rhs.id);
            log = std::move(rhs.log);
            log.emplace_back("move assigned");
        } catch(...) {}
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
    {
        return std::count_if(
            begin(log), end(log),
            [&](const std::string &s) {
                return s.find(value) != std::string::npos;
            });
    }

    bool operator==(const Logger &rhs) const
    { return (id == rhs.id) && (log == rhs.log); }

    virtual ~Logger() noexcept = default;

    std::string id;
    std::vector<std::string> log;
};


} // namespace test
} // namespace rc
