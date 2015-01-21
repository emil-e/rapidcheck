#pragma once

namespace rc {
namespace detail {
namespace ansi {

enum Attr
{
    AttrNone = 0,
    AttrBold = 1,
    AttrUnderscore = 4,
    AttrBlink = 5,
    AttrReverseVideo = 7,
    AttrConcealed = 8,
    FgBlack = 30,
    FgRed = 31,
    FgGreen = 32,
    FgYellow = 33,
    FgBlue = 34,
    FgMagenta = 35,
    FgCyan = 36,
    FgWhite = 37,
    BgBlack = 40,
    BgRed = 41,
    BgGreen = 42,
    BgYellow = 43,
    BgBlue = 44,
    BgMagenta = 45,
    BgCyan = 46,
    BgWhite = 47
};

//! Set the given text attributes.
template<typename ...Attrs>
std::string attr(Attrs ...attrs)
{
    return "\e[" + join(";", std::to_string(attrs)...) + "m";
}

//! Move cursor up `n` lines.
std::string cursorUp(int n);

//! Move cursor down `n` lines.
std::string cursorDown(int n);

//! Erase line escape sequence.
constexpr const char *eraseLine = "\e[K";
//! Save cursor position.
constexpr const char *cursorSave = "\e[s";
//! Restore cursor position.
constexpr const char *cursorRestore = "\e[u";
//! Move cursor to beginning of line.
constexpr char cursorHome = '\r';
//! Resets all settings.
constexpr const char *reset = "\ec";

} // namespace ansi
} // namespace detail
} // namespace rc
