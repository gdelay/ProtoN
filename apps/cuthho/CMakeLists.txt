add_executable(cuthho_square cuthho_square.cpp)
target_link_libraries(cuthho_square ${LINK_LIBS})
install(TARGETS cuthho_square RUNTIME DESTINATION bin)
