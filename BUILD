
cc_library(
  name = "packing",
  srcs = glob(
    [
      "src/common/*.cpp",
      "src/algorithm/*.cpp"
    ]
  ),
  hdrs = glob(
    [
      "src/common/*.h",
      "src/algorithm/*.h"
    ]
  ),
  includes = ["src"],
  copts = ["-Wall", "-O3", "-std=c++0x"],
)

cc_binary(
  name = "solve_sa",
  srcs = ["src/release/solve_sa.cpp"],
  copts = ["-Wall", "-O3", "-Isrc", "-std=c++0x"],
  linkopts = ["-pthread", "-lm"],
  deps = [":packing"],
)

cc_binary(
  name = "tree_search",
  srcs = ["src/release/tree_search.cpp"],
  copts = ["-Wall", "-O3", "-Isrc", "-std=c++0x"],
  linkopts = ["-pthread", "-lm"],
  deps = [":packing"],
)

cc_binary(
  name = "mento_carlo",
  srcs = ["src/release/mento_carlo.cpp"],
  copts = ["-Wall", "-O3", "-Isrc", "-std=c++0x"],
  linkopts = ["-pthread", "-lm"],
  deps = [":packing"],
)
