#! /usr/bin/python
##
# @file bazel-scan.py
# @brief Scan src directory and create BUILD file for bazel
# @author Yu Peng (ypeng@cs.hku.hk)
# @version 1.0.0
# @date 2016-03-13

import os
import re

COMMON = """
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
"""

TEMPLATE = """
cc_binary(
  name = "%s",
  srcs = ["%s"],
  copts = ["-Wall", "-O3", "-Isrc"],
  linkopts = ["-pthread", "-lm"],
  deps = [":packing"],
)
"""

if __name__ == "__main__":
  fp = open('BUILD', 'w')
  fp.write(COMMON)
  paths = ["src/release"]
  for path in paths:
    for file in os.listdir(path):
      if re.search("\.cpp$", file):
        basename = os.path.basename(file).rpartition('.')[0]
        fp.write(TEMPLATE % (basename, path + "/" + file))




