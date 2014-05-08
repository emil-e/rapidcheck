#include <iostream>

#include "rapidcheck/Arbitrary.hpp"

using namespace rc;

template<typename T, typename Alloc>
std::ostream &operator<<(std::ostream &os, const std::vector<T, Alloc> &vec)
{
    os << "[";
    if (!vec.empty()) {
        for (auto it = vec.begin(); it != (vec.end() - 1); it++)
            os << *it << ", ";
        os << vec.back();
    }
    os << "]";

    return os;
}

int main()
{
    auto vec = Arbitrary<std::vector<std::string>>()(10);
    std::cout << "int: " << vec << std::endl;
    return 0;
}
