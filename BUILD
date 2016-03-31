
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
  copts = ["-Wall", "-O3"],
)

cc_binary(
  name = "main",
  srcs = ["src/release/main.cpp"],
  copts = ["-Wall", "-O3", "-Isrc"],
  linkopts = ["-pthread", "-lm"],
  deps = [":packing"],
)
