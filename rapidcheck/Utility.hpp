#pragma once

//! Disables copying
#define RC_DISABLE_COPY(Class)                          \
    Class(const Class &) = delete;                \
    Class &operator=(const Class &) = delete;

//! Disables moving
#define RC_DISABLE_MOVE(Class)                  \
    Class(Class &&) = delete;             \
    Class &operator=(Class &&) = delete;
