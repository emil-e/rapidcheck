#include <iostream>

#include "rapidcheck/Arbitrary.hpp"
#include "rapidcheck/Show.hpp"
#include "rapidcheck/Check.hpp"

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
    check([](const std::vector<std::vector<int>> &vec) {
            // auto expected = vec;
            // auto actual = vec;
            // std::reverse(expected.begin(), expected.end());
            // crappyReverse(actual.begin(), actual.end());
            // return expected == actual;
            return false;
        });

    return 0;
}
