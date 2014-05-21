#include <iostream>

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

int main()
{
    check([](const std::vector<int> &vec, int foo) {
            auto expected(vec);
            auto actual(vec);
            std::reverse(expected.begin(), expected.end());
            crappyReverse2(actual.begin(), actual.end());
            return expected == actual;
        });

    return 0;
}
