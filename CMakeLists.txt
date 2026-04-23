# CMakeLists.txt — rcsim scaffold
#
# DESIGN_v1.3.md §15 (build system) + §15.1 (CI matrix).
# This file configures — it does NOT require FetchContent URLs to resolve.
# All FetchContent Declare stanzas are URL_PLACEHOLDER; fill in at first implementation
# session (phase 0 gate: rcsim-run --help works, which does not require URL resolution yet).

cmake_minimum_required(VERSION 3.25)
project(rcsim
    VERSION 1.3.0
    DESCRIPTION "Deterrence-Extended with Civilizational Economics — deterministic headless simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# §2.1, §15: strict IEEE-754 flags — foundation of determinism contract.
if(MSVC)
    add_compile_options(/fp:strict /W4 /WX)
else()
    add_compile_options(
        -ffp-contract=off
        -fno-fast-math
        -fno-associative-math
        -Wall -Wextra -Werror -pedantic
    )
endif()

# §15: dependencies — declared with placeholder URLs per CLAUDE.md Task 15 instruction.
#   DO NOT fill these URLs in the scaffold. They are pinned at first implementation session
#   so that the version hash is decided deliberately by the implementer, not by auto-scaffold.
include(FetchContent)

FetchContent_Declare(blake3
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — pin BLAKE3 C release
    URL ""
)

FetchContent_Declare(pcg
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — pin pcg-cpp commit
    URL ""
)

FetchContent_Declare(sleef
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — pin sleef release with Sleef_exp_u10 scalar
    URL ""
)

FetchContent_Declare(lemon
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — §4.6 deterministic Network Simplex (BSD)
    URL ""
)

FetchContent_Declare(ryml
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — rapidyaml for scenario YAML (§12)
    URL ""
)

FetchContent_Declare(nlohmann_json
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — for GeoJSON parsing (§18)
    URL ""
)

FetchContent_Declare(arrow
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — Apache Arrow / Parquet (§11.2)
    URL ""
)

FetchContent_Declare(catch2
    # TODO: URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION — pin Catch2 v3 release
    URL ""
)

# NOTE: FetchContent_MakeAvailable is intentionally NOT called here. The scaffold is
# configuration-only per CLAUDE.md §1.2 "Do NOT attempt to compile". At first implementation
# session, fill URLs above and uncomment the following line:
#
# FetchContent_MakeAvailable(blake3 pcg sleef lemon ryml nlohmann_json arrow catch2)

# §15: rcsim-core static library.
#
# Sources enumerated to match §15 CMakeLists.txt list and §18 directory layout.
# NOTE: CMakeLists.txt in §15 of the spec lists "observer/attribution_probabilistic.cpp"
# but the scaffold uses "observer/attribution.cpp" to match "observer/attribution.hpp"
# per CLAUDE.md Task 12 instruction ("matching .cpp"). Rename at first implementation
# session if the original name is preferred.
add_library(rcsim-core STATIC
    # core/
    src/core/tick.cpp
    src/core/fixed_point.cpp
    src/core/rng.cpp
    src/core/hash.cpp
    src/core/tree_reduce.cpp
    src/core/transcendentals.cpp               # polynomial_exp_bitexact + sleef fallback
    src/core/transcendentals_polynomial.cpp    # Padé [6/6] approximation
    src/core/canonicalize.cpp                  # signed-zero sweep + NaN/Inf trap + Kahan residual
    src/core/integer_network_flow.cpp          # replaces FP LP (§4.6)

    # state/
    src/state/world_state.cpp
    src/state/territory.cpp
    src/state/principal.cpp
    src/state/global_state.cpp
    src/state/pending.cpp
    src/state/canonicalization_state.cpp

    # dynamics/
    src/dynamics/algebraics.cpp
    src/dynamics/ode_system.cpp
    src/dynamics/integrator.cpp
    src/dynamics/coupling.cpp

    # action/
    src/action/action.cpp
    src/action/queue.cpp
    src/action/effect.cpp
    src/action/action_wire.cpp
    src/action/validate_epistemic.cpp          # fog-of-war validation (§5.4)
    src/action/apply_with_fizzle.cpp           # ground-truth application (§5.4)

    # observer/
    src/observer/measurement.cpp
    src/observer/distortion.cpp
    src/observer/asymmetric_telemetry.cpp
    src/observer/attribution.cpp               # probabilistic RNG-seeded logistic (§6.4)
    src/observer/cascade_stability.cpp         # legitimacy_S bounds (§6.4.1)
    src/observer/operator_policies.cpp

    # political/
    src/political/clock.cpp
    src/political/regime.cpp
    src/political/response_latency.cpp
    src/political/endogenous_transitions.cpp

    # io/
    src/io/sync_log.cpp
    src/io/replay.cpp
    src/io/scenario_yaml.cpp
    src/io/parquet_sink.cpp
    src/io/parquet_rollup.cpp
    src/io/ipc_multisubscriber.cpp             # §14 multi-subscriber IPC

    # geo/
    src/geo/geojson.cpp
    src/geo/partition.cpp
    src/geo/adjacency.cpp

    # testbed/
    src/testbed/harness.cpp
    src/testbed/sweep.cpp
    src/testbed/principal_policy.cpp           # abstract PrincipalPolicy (§11.3)
    src/testbed/scripted_heuristic_policy.cpp  # v1 default policy
)

target_include_directories(rcsim-core PUBLIC include)

# TODO: At first implementation session, after FetchContent_MakeAvailable is enabled, link:
# target_link_libraries(rcsim-core PUBLIC
#     blake3 pcg sleef lemon ryml nlohmann_json::nlohmann_json arrow_static parquet_static)

# §15: executables — the three CLI drivers.
add_executable(rcsim-run     tools/rcsim_run.cpp)
add_executable(rcsim-testbed tools/rcsim_testbed.cpp)
add_executable(rcsim-replay  tools/rcsim_replay.cpp)

target_link_libraries(rcsim-run     PRIVATE rcsim-core)
target_link_libraries(rcsim-testbed PRIVATE rcsim-core)
target_link_libraries(rcsim-replay  PRIVATE rcsim-core)

# §16: tests.
enable_testing()
add_subdirectory(tests)
