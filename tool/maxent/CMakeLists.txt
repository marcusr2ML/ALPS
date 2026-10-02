# ALPS Project: https://alps.comp-phys.org/
# SPDX-License-Identifier: MIT
#
# Analytic continuation with Maxent (Levy, LeBlanc, Gull, CPC 215, 149 (2017)).
# The sources come from CQMP/Maxent; they use Eigen and carry their own
# parameter class (maxent_parameters.hpp) in place of ALPSCore's alps::params.

find_package(Eigen3 3.3 CONFIG QUIET)
if(NOT TARGET Eigen3::Eigen)
  # Eigen is header-only: fetch it when it is not installed
  include(FetchContent)
  # SOURCE_SUBDIR names a directory without a CMakeLists.txt, so only the
  # headers are downloaded and Eigen's own build is not added
  FetchContent_Declare(maxent_eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
    URL_HASH SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72
    SOURCE_SUBDIR headers-only)
  FetchContent_MakeAvailable(maxent_eigen)
  add_library(maxent_eigen INTERFACE)
  target_include_directories(maxent_eigen SYSTEM INTERFACE ${maxent_eigen_SOURCE_DIR})
  add_library(Eigen3::Eigen ALIAS maxent_eigen)
  message(STATUS "Maxent: using downloaded Eigen 3.4.0")
else()
  message(STATUS "Maxent: using Eigen ${Eigen3_VERSION}")
endif()

# the LAPACK SVD path of Maxent stays off; the kernel SVD uses Eigen
configure_file(maxent_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config/maxent_config.hpp)

add_library(maxent_core STATIC
  default_model.cpp
  maxent_backcont.cpp
  maxent_grid.cpp
  maxent_helper.cpp
  maxent_kernel.cpp
  maxent_parameters.cpp
  maxent_params.cpp
  maxent_simulation.cpp)
target_include_directories(maxent_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/config)
target_link_libraries(maxent_core PUBLIC alps Eigen3::Eigen)

add_executable(maxent maxent.cpp)
target_link_libraries(maxent maxent_core)
install(TARGETS maxent RUNTIME DESTINATION bin COMPONENT tools)

if(ALPS_BUILD_TESTS)
  add_executable(maxent_parameters_test maxent_parameters_test.cpp)
  target_link_libraries(maxent_parameters_test maxent_core)
  add_alps_test(maxent_parameters_test)

  # Regression test: linear-FREQUENCY_GRID runs must produce a finite
  # spectrum with the right sum rule.
  add_executable(maxent_linear_grid_numeric maxent_linear_grid_numeric.cpp)
  target_link_libraries(maxent_linear_grid_numeric maxent_core)
  add_alps_test(maxent_linear_grid_numeric)
endif()
