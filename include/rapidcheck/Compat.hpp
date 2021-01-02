#pragma once

#include <type_traits>

namespace rc {
namespace compat {

#if __cplusplus <= 201703L
template <typename Fn, typename ...Args>
using return_type = typename std::result_of<Fn(Args...)>;
#else
template <typename Fn, typename ...Args>
using return_type = typename std::invoke_result<Fn,Args...>;
#endif

}
}
