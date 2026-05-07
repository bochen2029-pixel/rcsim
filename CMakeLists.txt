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
    LANGUAGES C CXX ASM
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# §2.1, §15: strict IEEE-754 flags + warnings-as-errors apply ONLY to rcsim's own
# targets. Third-party dependencies (sleef, lemon, etc.) compile with their own
# warning posture; -Werror against unrelated diagnostics breaks their builds.
# Defined as variables here, applied via target_compile_options on rcsim targets.
if(MSVC)
    set(RCSIM_FP_FLAGS /fp:strict)
    set(RCSIM_WARN_FLAGS /W4 /WX)
else()
    set(RCSIM_FP_FLAGS
        -ffp-contract=off
        -fno-fast-math
        -fno-associative-math
    )
    set(RCSIM_WARN_FLAGS
        -Wall -Wextra -Werror -pedantic
    )
endif()

# §15: dependencies. Versions pinned at Phase 0 of v1.3 implementation per
# docs/PHASE_0_2_PLAN.md §4. SHA256 hashes are TODO at first download — implementer
# fills them after verifying the downloaded archive matches the upstream signature.
# Do not unpin without bumping schema_minor.
include(FetchContent)

FetchContent_Declare(blake3
    URL https://github.com/BLAKE3-team/BLAKE3/archive/refs/tags/1.5.4.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=... after first download
    SOURCE_SUBDIR c
)

FetchContent_Declare(pcg
    URL https://github.com/imneme/pcg-cpp/archive/428802d1a5634f96bcd0705fab379ff0113bcf13.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=...
)

FetchContent_Declare(sleef
    URL https://github.com/shibatch/sleef/archive/refs/tags/3.6.1.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=...
)

FetchContent_Declare(lemon
    URL https://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=...
    # §4.6 deterministic Network Simplex; BSD license.
)

FetchContent_Declare(ryml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG        v0.7.2
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES_RECURSE TRUE
    # TODO(phase 0): switch back to URL+SHA256 once ryml releases include c4core sources.
)

FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=...
)

# Arrow is required only for Phase 4+ (Parquet sink + rollup, §11.2). It pulls
# many system deps (jemalloc, snappy, lz4, zstd, brotli, ...). Phase 0–2 builds
# can disable it via -DRCSIM_ENABLE_ARROW=OFF.
option(RCSIM_ENABLE_ARROW "Enable Arrow/Parquet (Phase 4+); requires system deps" OFF)
if(RCSIM_ENABLE_ARROW)
    FetchContent_Declare(arrow
        URL https://archive.apache.org/dist/arrow/arrow-17.0.0/apache-arrow-17.0.0.tar.gz
        # TODO(phase 0): pin URL_HASH SHA256=...
        SOURCE_SUBDIR cpp
    )
endif()

FetchContent_Declare(catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
    # TODO(phase 0): pin URL_HASH SHA256=...
)

# Sleef build options — disable inline headers / SIMD-specific dispatches. We only
# use Sleef_exp_u10 (scalar, double-precision) per §4.5. CI test 07 enforces this.
set(BUILD_TESTS OFF CACHE BOOL "Sleef: skip tests")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static linkage")
set(SLEEF_BUILD_DFT OFF CACHE BOOL "No DFT")
set(SLEEF_BUILD_GNUABI_LIBS OFF CACHE BOOL "No GNUABI")
set(SLEEF_BUILD_QUAD OFF CACHE BOOL "No quad")
set(SLEEF_BUILD_SCALAR_LIB ON CACHE BOOL "Scalar Sleef_exp_u10 needed")

# Arrow build options — Parquet only; skip Compute, Flight, etc.
set(ARROW_PARQUET ON CACHE BOOL "Need Parquet writer")
set(ARROW_BUILD_STATIC ON CACHE BOOL "Static linkage")
set(ARROW_BUILD_SHARED OFF CACHE BOOL "Static linkage")
set(ARROW_BUILD_TESTS OFF CACHE BOOL "Skip Arrow tests")

FetchContent_MakeAvailable(blake3 pcg sleef lemon ryml nlohmann_json catch2)
if(RCSIM_ENABLE_ARROW)
    FetchContent_MakeAvailable(arrow)
endif()

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

    # geo/  (R-18 removed partition.cpp — adjacency + geojson cover spatial structure)
    src/geo/geojson.cpp
    src/geo/adjacency.cpp

    # testbed/
    src/testbed/harness.cpp
    src/testbed/sweep.cpp
    src/testbed/principal_policy.cpp           # abstract PrincipalPolicy (§11.3)
    src/testbed/scripted_heuristic_policy.cpp  # v1 default policy
)

target_include_directories(rcsim-core PUBLIC include)

# Strict FP + warning posture only on our own code (third-party deps use their own).
target_compile_options(rcsim-core PRIVATE ${RCSIM_FP_FLAGS} ${RCSIM_WARN_FLAGS})

target_link_libraries(rcsim-core PUBLIC
    blake3
    sleef
    lemon
    ryml::ryml
    nlohmann_json::nlohmann_json
)
# pcg-cpp is header-only; expose its include path explicitly.
target_include_directories(rcsim-core PUBLIC ${pcg_SOURCE_DIR}/include)
# LEMON headers live at <lemon_SOURCE>/lemon/*.h; the generated `config.h`
# lives in <lemon_BINARY>/lemon/config.h. Both must be on the include path.
target_include_directories(rcsim-core PUBLIC
    ${lemon_SOURCE_DIR}
    ${lemon_BINARY_DIR}
)

if(RCSIM_ENABLE_ARROW)
    target_link_libraries(rcsim-core PUBLIC arrow_static parquet_static)
    target_compile_definitions(rcsim-core PUBLIC RCSIM_HAS_ARROW=1)
endif()

# §15: executables — the three CLI drivers.
add_executable(rcsim-run     tools/rcsim_run.cpp)
add_executable(rcsim-testbed tools/rcsim_testbed.cpp)
add_executable(rcsim-replay  tools/rcsim_replay.cpp)

target_link_libraries(rcsim-run     PRIVATE rcsim-core)
target_link_libraries(rcsim-testbed PRIVATE rcsim-core)
target_link_libraries(rcsim-replay  PRIVATE rcsim-core)

target_compile_options(rcsim-run     PRIVATE ${RCSIM_FP_FLAGS} ${RCSIM_WARN_FLAGS})
target_compile_options(rcsim-testbed PRIVATE ${RCSIM_FP_FLAGS} ${RCSIM_WARN_FLAGS})
target_compile_options(rcsim-replay  PRIVATE ${RCSIM_FP_FLAGS} ${RCSIM_WARN_FLAGS})

# §16: tests.
enable_testing()
add_subdirectory(tests)
