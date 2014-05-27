#include <iostream>
#include <map>

#include <rapidcheck/rapidcheck.h>

using namespace rc;

template<typename Iterator>
void crappyReverse(Iterator begin, Iterator end)
{
    std::reverse(begin, end);
    if ((end - begin) > 40 && *(begin + 21) > 20)
        *(begin + 20) = 10;
}

template<typename Iterator>
void crappyReverse2(Iterator begin, Iterator end)
{
    for (auto it = begin; it != end; it++) {
        if (*it > 512 && *it < 1024)
            return;
    }
    std::reverse(begin, end);
}

template<typename Iterator>
void crappyReverse3(Iterator begin, Iterator end)
{
    for (auto it = begin; it != end; it++) {
        if (it->find("ab") != std::string::npos)
            return;
    }
    std::reverse(begin, end);
}

template<typename K, typename V>
size_t crappySize(const std::map<K, V> &m)
{
    return std::min<size_t>(20, m.size());
}

template<typename Collection>
size_t crappySize2(const Collection &c)
{
    if (c.size() <= 1)
        return c.size();

    for (auto it = ++c.begin(); it != c.end(); it++) {
        if (*it == c[0])
            return 2;
    }

    return c.size();
}

template<typename Collection>
bool prop_size(const Collection &c)
{
    return crappySize2(c) == c.size();
}

bool prop_size2(const std::vector<std::map<std::string, std::string>> &c)
{
    int count = 0;
    for (const auto &m : c) {
        for (const auto &p : m) {
            if (p.first.size() > 10)
                count++;
        }
    }

    return count < 5;
}

int main()
{
    check(prop_size2);
    return 0;
}
