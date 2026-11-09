find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE TEST_SOURCES
    "core/*.cpp"
    "test_main.cpp"
)

if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_SOURCE_DIR}/src
    )
    include(CTest)
    include(Catch)
    catch_discover_tests(unit_tests)
else()
    message(STATUS "test_native: Directory initialized, awaiting test suites (TDD red phase).")
    # Add dummy target so cmake/ninja builds cleanly in CI when no tests are added yet
    add_custom_target(unit_tests
        COMMAND ${CMAKE_COMMAND} -E echo "test_native: No tests defined yet. Ready for TDD."
    )
endif()
