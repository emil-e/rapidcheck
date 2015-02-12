#pragma once

#define RC_SIGNED_INTEGRAL_TYPES                \
    char,                                       \
    short,                                      \
    int,                                        \
    long,                                       \
    long long

#define RC_UNSIGNED_INTEGRAL_TYPES              \
    unsigned char,                              \
    unsigned short,                             \
    unsigned int,                               \
    unsigned long,                              \
    unsigned long long

#define RC_INTEGRAL_TYPES RC_SIGNED_INTEGRAL_TYPES, RC_UNSIGNED_INTEGRAL_TYPES

#define RC_REAL_TYPES double, float

#define RC_SIGNED_TYPES RC_REAL_TYPES, RC_SIGNED_INTEGRAL_TYPES

#define RC_NUMERIC_TYPES RC_REAL_TYPES, RC_INTEGRAL_TYPES

#define RC_SEQUENCE_CONTAINERS(T)               \
    std::vector<T>,                             \
    std::deque<T>,                              \
    std::forward_list<T>,                       \
    std::list<T>                                \

#define RC_ORDERED_CONTAINERS(T)                \
    RC_SEQUENCE_CONTAINERS(T),                  \
    std::set<T>,                                \
    std::map<T, T>,                             \
    std::multiset<T>,                           \
    std::multimap<T, T>

#define RC_UNORDERED_CONTAINERS(T)              \
    std::unordered_set<T>,                      \
    std::unordered_map<T, T>,                   \
    std::unordered_multiset<T>,                 \
    std::unordered_multimap<T, T>

#define RC_GENERIC_CONTAINERS(T)                            \
    RC_ORDERED_CONTAINERS(T),                               \
    RC_UNORDERED_CONTAINERS(T)

#define RC_STRING_TYPES std::string, std::wstring
