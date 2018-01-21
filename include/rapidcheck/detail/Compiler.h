#pragma once

#if defined(_MSC_VER)
#if _CPPUNWIND || _HAS_EXCEPTIONS
#define RC_EXCEPTIONS_ENABLED 1
#else
#define RC_EXCEPTIONS_ENABLED 0
#endif
#elif defined(__GNUC__)
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
#define RC_EXCEPTIONS_ENABLED 1
#else
#define RC_EXCEPTIONS_ENABLED 0
#endif
#elif defined(__clang__)
#if __has_feature(cxx_exceptions)
#define RC_EXCEPTIONS_ENABLED 1
#else
#define RC_EXCEPTIONS_ENABLED 0
#endif
#endif

#if RC_EXCEPTIONS_ENABLED
#define RC_THROW_EXCEPTION(Exception, ...) throw Exception(__VA_ARGS__)
#define RC_TRY try
#define RC_CATCH(Exception, ...) catch(Exception) { __VA_ARGS__ }
#else
#define RC_THROW_EXCEPTION(Exception, ...) std::abort()
#define RC_TRY
#define RC_CATCH(Exception, ...)
#endif
