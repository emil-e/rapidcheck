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

int main()
{
    check([](const std::vector<int> &vec, int foo) {
            auto expected(vec);
            auto actual(vec);
            std::reverse(expected.begin(), expected.end());
            crappyReverse(actual.begin(), actual.end());
            return expected == actual;
        });

    return 0;
}
