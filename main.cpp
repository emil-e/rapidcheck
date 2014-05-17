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
    int foob = 0;
    check([&foob](const std::vector<int> &vec, int foo) {
            foob++;
            if (foob > 50)
                return false;
            else
                return true;
        });

    return 0;
}
