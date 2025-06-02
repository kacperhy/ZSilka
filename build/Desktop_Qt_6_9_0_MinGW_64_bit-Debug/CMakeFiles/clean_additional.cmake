# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ZarzadzanieSilownia_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ZarzadzanieSilownia_autogen.dir\\ParseCache.txt"
  "ZarzadzanieSilownia_autogen"
  )
endif()
