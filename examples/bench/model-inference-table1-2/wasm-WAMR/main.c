/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "debug.h"
#include "shell.h"
#include "wasm/model_wasm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm_c_api.h>
#include <wasm_export.h>

#include <stdalign.h>
#include <stddef.h>

#define WAMR_ENV_THREAD_STACK_SIZE 4096
#define ERROR_BUFFER_SIZE 128

const uint32_t stack_size = 1024;
const uint32_t heap_size = 0;

#include "model_input.h"

char env_buffer0[98000];
char error_buffer[ERROR_BUFFER_SIZE];

void *get_address(wasm_function_inst_t func, unsigned int size, wasm_module_inst_t moduleinst, wasm_exec_env_t exec_env)
{
    unsigned int argv[2];
    unsigned int module_address;
    bool success = wasm_runtime_call_wasm(exec_env, func, 0, (uint32_t *)argv);
    if (!success)
    {
        printf("Failed to get value\n");
        return NULL;
    }
    module_address = argv[0];
    printf("value address: %d\n", module_address);
    if (!wasm_runtime_validate_app_addr(moduleinst, module_address, size))
    {
        printf("Model error: invalid model error\n");
        return false;
    }
    return wasm_runtime_addr_app_to_native(moduleinst, module_address);
}

#define BENCH_start() printf("\n!BENCH_start\n")
#define BENCH_log(name, value_int, unit)                                                                               \
    printf("\n!BENCH_log{\"name\": \"%s\", \"value\": %d, \"unit\":\"%s\"}\n", name, value_int, unit)
#define BENCH_end() printf("\n!BENCH_end\n")
#include <benchmark.h>
#include <ztimer.h>

void log_wamr_memory(void)
{
    mem_alloc_info_t mem_alloc_info;

    wasm_runtime_get_mem_alloc_info(&mem_alloc_info);
    printf("highmark_size: %ld, total_free_size  %ld, total_size %ld\n", mem_alloc_info.highmark_size,
           mem_alloc_info.total_free_size, mem_alloc_info.total_size);
}

int main(void)
{
    unsigned int t1;
    unsigned int t2;
    BENCH_start();

    ztimer_sleep(ZTIMER_SEC, 5);
    printf("Start \n");
    RuntimeInitArgs init_args = {.mem_alloc_type = Alloc_With_Pool,
                                 .mem_alloc_option.pool.heap_buf = env_buffer0,
                                 .mem_alloc_option.pool.heap_size = sizeof(env_buffer0)};

    if (!wasm_runtime_full_init(&init_args))
    {
        printf("env init 0 failed\n");
        return 1;
    }
    log_wamr_memory();
    wasm_module_t module =
        wasm_runtime_load((unsigned char *)model_wasm, model_wasm_len, error_buffer, sizeof(error_buffer));
    log_wamr_memory();
    printf("Loaded\n");
    printf("Module p %p %s\n", module, error_buffer);
    // ENV 0
    //      Module 0

    wasm_module_inst_t mod_inst = wasm_runtime_instantiate(module, stack_size, 0, error_buffer, sizeof(error_buffer));
    if (!mod_inst)
    {
        printf("WAMR error: %s\n", error_buffer);
        return 1;
    }
    printf("Instantiated\n");
    log_wamr_memory();
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(mod_inst, stack_size);
    DEBUG("create exec env\n");
    if (!exec_env)
    {
        printf("WAMR env: Runtime failed to create exec env\n");
        return 1;
    }
    log_wamr_memory();
    wasm_function_inst_t funcinit = wasm_runtime_lookup_function(mod_inst, "init");
    if (!funcinit)
    {
        printf("WAMR env: Runtime failed to find init function\n");
        return 1;
    }
    printf("Run\n");
    wasm_function_inst_t funcrun = wasm_runtime_lookup_function(mod_inst, "run");
    if (!funcrun)
    {
        printf("WAMR env: Runtime failed to find run function\n");
        return 1;
    }
    wasm_function_inst_t funcinput = wasm_runtime_lookup_function(mod_inst, "get_input_address");
    if (!funcinput)
    {
        printf("WAMR env: Runtime failed to find init function\n");
        return 1;
    }
    wasm_function_inst_t funcoutput = wasm_runtime_lookup_function(mod_inst, "get_output_address");
    if (!funcoutput)
    {
        printf("WAMR env: Runtime failed to find init function\n");
        return 1;
    }
    DEBUG("Loaded module and function in env 0\n");
    bool success = wasm_runtime_call_wasm(exec_env, funcinit, 0, NULL);

    char *input_addr = get_address(funcinput, sizeof(input), mod_inst, exec_env);
    signed char *output_addr = get_address(funcoutput, 100, mod_inst, exec_env);
    memcpy(input_addr, input, sizeof(input));

    // success = wasm_runtime_call_wasm(exec_env, funcrun, 0, NULL);
    BENCHMARK_FUNC("run model", 20, wasm_runtime_call_wasm(exec_env, funcrun, 0, NULL));
    if (!success)
    {
        printf("Error when calling function %s\n", wasm_runtime_get_exception(mod_inst));
    }

    for (int i = 0; i < 12; i++)
    {
        printf("%d %d\n", i, output_addr[i]);
    }
    printf("\nFinish\n");

    // wasm_runtime_dump_mem_consumption(exec_env);
    log_wamr_memory();
    BENCH_end();

    return 0;
}
