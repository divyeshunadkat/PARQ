add_executable (parq PARQ.cpp)
target_link_libraries (parq ${Z3_LIBRARY} ${Boost_LIBRARIES} ${GMPXX_LIB} ${GMP_LIB} ${ARMADILLO_LIBRARIES})
install(TARGETS parq RUNTIME DESTINATION bin)
