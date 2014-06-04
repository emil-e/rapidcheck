#include "rapidcheck/Check.h"

#include "ConsoleDelegate.h"

namespace rc {

void rapidcheck(int argc, const char * const *argv)
{
    using namespace detail;
    ConsoleDelegate delegate(std::cout);
    TestSuite::defaultSuite().run(delegate);
}

} // namespace rc
