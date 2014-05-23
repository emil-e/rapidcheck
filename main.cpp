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

int main()
{
    check([](const std::map<std::string, std::string> &m) {
            return crappySize(m) == m.size();
        });

    return 0;
}
