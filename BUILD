package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rapidcheck",
    strip_include_prefix =
        "include",
    includes = [
        "include",
    ],
    hdrs = glob([
        "include/rapidcheck/**/*.h",
        "include/rapidcheck/**/*.hpp",
    ]),
    srcs = glob([
        "src/**/*.cpp",
        "src/**/*.h",
    ]),
)
