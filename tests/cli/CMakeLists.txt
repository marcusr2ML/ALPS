# Copyright Matthias Troyer and Synge Todo 2009-2013.
# SPDX-License-Identifier: MIT

alps_add_test(lattice2xml)
set_property(TEST lattice2xml PROPERTY LABELS "cli;compatibility")

find_package(Python 3.10 QUIET COMPONENTS Interpreter)
if(Python_Interpreter_FOUND)
  set(archive_arguments "$<TARGET_FILE:txt2archive>" "$<TARGET_FILE:xml2archive>")
  if(TARGET archive)
    list(APPEND archive_arguments "$<TARGET_FILE:archive>")
  endif()
  add_test(NAME archive_cli_contract
    COMMAND "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/archive_contract.py"
      ${archive_arguments})
  set_tests_properties(archive_cli_contract PROPERTIES TIMEOUT 60 LABELS "cli;compatibility")
  if(TARGET p2h5)
    add_test(NAME p2h5_cli
      COMMAND "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/check_p2h5.py" "$<TARGET_FILE:p2h5>")
    set_tests_properties(p2h5_cli PROPERTIES TIMEOUT 60 LABELS "cli")
  endif()
endif()
