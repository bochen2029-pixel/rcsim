# tests/CMakeLists.txt — test-suite configuration.
#
# §16.1 determinism, §16.2 property, §16.3 retrodiction.
# All test stubs carry [pending] Catch2 tag so they can be excluded from default CI run
# until implemented (see CLAUDE.md §3 Task 14).

# TODO: At first implementation session, enable Catch2 linkage via:
# find_package(Catch2 3 REQUIRED)
# include(CTest)
# include(Catch)

# Iterate test subdirectories and declare each stub file as a test target.
set(RCSIM_TEST_CATEGORIES
    determinism
    property
    retrodiction
    unit
)

foreach(category ${RCSIM_TEST_CATEGORIES})
    file(GLOB test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${category}/*.cpp")
    foreach(test_source ${test_sources})
        get_filename_component(test_name ${test_source} NAME_WE)
        set(target_name "rcsim-test-${category}-${test_name}")

        add_executable(${target_name} ${test_source})
        target_link_libraries(${target_name} PRIVATE rcsim-core)
        # TODO: At first implementation session, after Catch2 is available via
        # FetchContent_MakeAvailable in the top-level CMakeLists.txt:
        # target_link_libraries(${target_name} PRIVATE Catch2::Catch2WithMain)
        # catch_discover_tests(${target_name})

        # Until Catch2 is wired, register a placeholder ctest entry so the test matrix
        # is visible in ctest -N. The executable will fail to link until Catch2 is enabled,
        # which is expected and gated at Phase 0 per CLAUDE.md §1.2.
        add_test(NAME ${target_name} COMMAND ${target_name})

        # Mark as pending so CI can filter them out during early phases.
        set_tests_properties(${target_name} PROPERTIES LABELS "pending;${category}")
    endforeach()
endforeach()
