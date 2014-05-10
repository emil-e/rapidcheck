#include <iostream>

#include "rapidcheck/Arbitrary.hpp"
#include "rapidcheck/Show.hpp"
#include "rapidcheck/Check.hpp"

using namespace rc;

template<typename Iterator>
void crappyReverse(Iterator begin, Iterator end)
{
    if ((end - begin) > 10)
        end = begin + 10;
    std::reverse(begin, end);
}

int main()
{
    check([](const std::vector<int> &vec) {
            auto expected = vec;
            auto actual = vec;
            std::reverse(expected.begin(), expected.end());
            crappyReverse(actual.begin(), actual.end());
            return expected == actual;
        });

    return 0;
}
