/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#if ENABLE_AOT == 1
#define F_FILE (uint8_t *)F_aot
#define F_SIZE F_aot_len
#include "wasm/F_aot.h"
#define G_FILE (uint8_t *)G_aot
#define G_SIZE G_aot_len
#include "wasm/G_aot.h"
#define MODEL_FILE (uint8_t *)model_aot
#define MODEL_SIZE model_aot_len
#include "wasm/model_aot.h"
#else
#define F_FILE F_wasm
#define F_SIZE F_wasm_len
#include "wasm/F_wasm.h"
#define G_FILE G_wasm
#define G_SIZE G_wasm_len
#include "wasm/G_wasm.h"
#define MODEL_FILE model_wasm
#define MODEL_SIZE model_wasm_len
#include "wasm/model_wasm.h"
#endif

#include "benchmark.h"
#include "debug.h"
#include "shell.h"
#include "wamr_env_thread.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm_c_api.h>
#include <wasm_export.h>

#include <stdalign.h>
#include <stddef.h>

#define WAMR_ENV_THREAD_STACK_SIZE 3100
#define ERROR_BUFFER_SIZE 128

// This might require tune RAM usage
#define ENABLE_MODEL_RESTORE_BEFORE_INFERENCE 1

const uint32_t stack_size = 1;
const uint32_t heap_size = 0;

char env_buffer0[8970];
char env_buffer1[107000];
char env_buffer2[8970];

/**
 * If you use a newer version of WAMR, you can use
 *
 * char env_buffer0[9970];
 * char env_buffer1[100000];
 * char env_buffer2[10000];
 */

wamr_env_thread_t saved_env_model_thread;
wamr_env_thread_number_t saved_env_model_number = -1;

#if ENABLE_MODEL_RESTORE_BEFORE_INFERENCE
char saving_model_buffer[107000];
#endif

mutex_t call_model_mutex = (mutex_t)MUTEX_INIT;

#define DWT_CYCCNT (unsigned int volatile *)0xE0001004
#define DWT_CTRL (unsigned int volatile *)0xE0001000
#define BENCH_start() printf("\n!BENCH_start\n")
#define BENCH_log(name, value_int, unit)                                                                               \
    printf("\n!BENCH_log{\"name\": \"%s\", \"value\": %d, \"unit\":\"%s\"}\n", name, value_int, unit)
#define BENCH_end() printf("\n!BENCH_end\n")

/*
 *
 * Contain a bug if input and output size do not match the size required in the model
 *
 *
 */
bool call_model(wasm_exec_env_t exec_env, char *input, unsigned int input_size, char *output, unsigned int output_size)
{
    unsigned int t1, t2;
    uint32_t argv[2];
    unsigned int input_address;
    char *native_input_address;
    unsigned int output_address;
    char *native_output_address;

    mutex_lock(&call_model_mutex);
    t1 = (unsigned int)*DWT_CYCCNT;
#if ENABLE_MODEL_RESTORE_BEFORE_INFERENCE
    wamr_env_thread_restored(ENV_1, &saved_env_model_thread, saving_model_buffer, sizeof(saving_model_buffer));
#endif

    t2 = (unsigned int)*DWT_CYCCNT;
    BENCH_log("restore_time", t2 - t1, "tick");
    printf("input %p %d\n", input, input_size);
    printf("call_model was called\n");
    int failed = wamr_env_thread_call_func_sub_env_with_args(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, 0, argv, true);
    if (failed)
    {
        printf("Model error: loading input\n");
        mutex_unlock(&call_model_mutex);
        return false;
    }
    input_address = argv[0];
    printf("input address: %d\n", input_address);
    failed = wamr_env_thread_call_func_sub_env_with_args(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_1, 0, argv, true);
    if (failed)
    {
        printf("Model error: loading output\n");
        mutex_unlock(&call_model_mutex);
        return false;
    }
    output_address = argv[0];
    printf("output address: %d\n", output_address);
    if (!wamr_env_thread_validate_app_addr(ENV_1, MODULE_INST_SLOT_0, input_address, input_size) ||
        !wamr_env_thread_validate_app_addr(ENV_1, MODULE_INST_SLOT_0, output_address, output_size))
    {
        printf("Model error: invalid model error\n");
        mutex_unlock(&call_model_mutex);
        return false;
    }
    native_input_address = wamr_env_thread_addr_app_to_native(ENV_1, MODULE_INST_SLOT_0, input_address);
    native_output_address = wamr_env_thread_addr_app_to_native(ENV_1, MODULE_INST_SLOT_0, output_address);
    memcpy(native_input_address, input, input_size);
    failed = wamr_env_thread_call_func_sub_env(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_2, true);
    if (failed)
    {
        printf("Model error: failed running model\n");
        mutex_unlock(&call_model_mutex);
        return false;
    }
    memcpy(output, native_output_address, output_size);
    mutex_unlock(&call_model_mutex);
    return true;
}

int main(void)
{
    ztimer_sleep(ZTIMER_SEC, 5);
    static NativeSymbol native_symbols_call_model[] = {
        {"call_model", &call_model, "(*~*~)i", NULL},
    };

    wamr_env_thread_init();
    DEBUG("save default ok\n");
    int failed = wamr_env_thread_init_env(ENV_0, env_buffer0, sizeof(env_buffer0), WAMR_ENV_THREAD_STACK_SIZE - 1,
                                          ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 0 failed\n");
    }
    failed = wamr_env_thread_init_env(ENV_1, env_buffer1, sizeof(env_buffer1), WAMR_ENV_THREAD_STACK_SIZE,
                                      ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 1 failed\n");
    }
    failed = wamr_env_thread_init_env(ENV_2, env_buffer2, sizeof(env_buffer2), WAMR_ENV_THREAD_STACK_SIZE + 1,
                                      ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 2 failed\n");
    }
    wamr_env_thread_register_natives(ENV_0, "env", native_symbols_call_model,
                                     sizeof(native_symbols_call_model) / sizeof(NativeSymbol));
    wamr_env_thread_register_natives(ENV_2, "env", native_symbols_call_model,
                                     sizeof(native_symbols_call_model) / sizeof(NativeSymbol));

    // ENV 0
    //      Module 0

    failed = wamr_env_thread_load_mod(ENV_0, F_FILE, F_SIZE, MODULE_SLOT_0);
    if (failed)
    {
        printf("Error loading module in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_mod_inst(ENV_0, MODULE_SLOT_0, MODULE_INST_SLOT_0, stack_size);
    if (failed)
    {
        printf("Error loading module in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_func(ENV_0, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "f");
    if (failed)
    {
        printf("Error loading function in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    DEBUG("Loaded module and function in env 0\n");

    // ENV 2
    //      Module 0

    failed = wamr_env_thread_load_mod(ENV_2, G_FILE, G_SIZE, MODULE_SLOT_0);
    if (failed)
    {
        printf("Error loading module in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_2));
    }
    failed = wamr_env_thread_load_mod_inst(ENV_2, MODULE_SLOT_0, MODULE_INST_SLOT_0, stack_size);
    if (failed)
    {
        printf("Error loading module in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_2));
    }
    failed = wamr_env_thread_load_func(ENV_2, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "g");
    if (failed)
    {
        printf("Error loading function in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_2));
    }
    DEBUG("Loaded module and function in env 0\n");

    // ENV 1
    //      Module 0

    failed = wamr_env_thread_load_mod(ENV_1, MODEL_FILE, MODEL_SIZE, MODULE_SLOT_0);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_mod_inst(ENV_1, MODULE_SLOT_0, MODULE_INST_SLOT_0, stack_size);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "get_input_address");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_1, "get_output_address");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_2, "run");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_3, "init");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    DEBUG("Loaded module and function in env 1\n");

    wamr_env_thread_call_func_plain_exec_with_args(ENV_1, MODULE_SLOT_0, FUNCTION_SLOT_3, 0, NULL);

#if ENABLE_MODEL_RESTORE_BEFORE_INFERENCE
    wamr_env_thread_save(ENV_1, &saved_env_model_thread, saving_model_buffer, sizeof(saving_model_buffer));
#endif
    wamr_env_thread_call_func(ENV_0, MODULE_SLOT_0, FUNCTION_SLOT_0);
    wamr_env_thread_call_func(ENV_2, MODULE_SLOT_0, FUNCTION_SLOT_0);
    DEBUG("\n==\n");
    printf("Containers launched\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    ps();
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
