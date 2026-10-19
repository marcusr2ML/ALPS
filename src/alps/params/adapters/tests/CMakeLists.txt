add_executable(parameter_input parameter_input.cpp)
target_link_libraries(parameter_input PRIVATE ALPS::alps)
alps_add_gtest(parameter_input COMPONENT params)
