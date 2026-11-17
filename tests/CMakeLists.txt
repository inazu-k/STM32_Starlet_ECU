cmake_minimum_required(VERSION 3.22)
project(engine_sync_tests C)
enable_testing()

# テスト名と実行ファイル名を維持し、共通のコンパイル設定と登録をまとめる。
function(add_host_test name target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE ../Core/Inc)
    set_target_properties(${target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
    endif()
    add_test(NAME ${name} COMMAND ${target})
endfunction()

add_host_test(engine_sync engine_sync_tests
    engine_sync_test.c ../Core/Src/engine_sync.c)
add_host_test(engine_sync_service engine_sync_service_tests
    engine_sync_service_test.c ../Core/Src/engine_sync.c ../Core/Src/engine_sync_service.c)
add_host_test(ecu_model ecu_model_tests
    ecu_model_test.c ../Core/Src/ecu_model.c ../Core/Src/injection_plan.c ../Core/Src/engine_sync.c)
add_host_test(sensors sensor_tests sensor_test.c ../Core/Src/efi_calc.c)
add_host_test(ecu_regression ecu_regression_tests
    ecu_regression_test.c ../Core/Src/ecu_model.c ../Core/Src/injection_plan.c ../Core/Src/engine_sync.c)
add_host_test(model_boundary model_boundary_tests
    model_boundary_test.c ../Core/Src/ecu_model.c ../Core/Src/injection_plan.c)

# ADC試験では実機用main.hよりHALスタブを優先する。
target_include_directories(sensor_tests BEFORE PRIVATE stubs)
foreach(target ecu_model_tests ecu_regression_tests model_boundary_tests)
    target_link_libraries(${target} PRIVATE m)
endforeach()
