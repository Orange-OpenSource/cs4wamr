/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm_c_api.h>
#include <wasm_export.h>

#if ENABLE_AOT == 1
#define F_FILE (uint8_t *)F_aot
#define F_SIZE F_aot_len
#include "wasm/F_aot.h"
#define G_FILE (uint8_t *)G_aot
#define G_SIZE G_aot_len
#include "wasm/G_aot.h"
#define H_FILE (uint8_t *)H_aot
#define H_SIZE H_aot_len
#include "wasm/H_aot.h"
#else
#define F_FILE F_wasm
#define F_SIZE F_wasm_len
#include "wasm/F_wasm.h"
#define G_FILE G_wasm
#define G_SIZE G_wasm_len
#include "wasm/G_wasm.h"
#define H_FILE H_wasm
#define H_SIZE H_wasm_len
#include "wasm/H_wasm.h"
#endif

#include "native_functions.h"
#include "ps.h"
#include "shell.h"
#include "wamr_env_thread.h"
#include "ztimer.h"

#define WAMR_ENV_BUFFER_SIZE 12000
#define WAMR_ENV_THREAD_STACK_SIZE 2048
#define ERROR_BUFFER_SIZE 248

uint32_t stack_size = 1024, heap_size = 0;
char error_buf[128];

char env_buffer0[20000];
char env_buffer1[20000];

char *env_buffer[ENV_AMOUNT] = {env_buffer0, env_buffer1};

int main(void)
{
    static NativeSymbol native_symbols_f[] = {
        {"call", &call_f, "()", NULL},
        {"wait", &wait, "(i)", NULL},
        {"nothing", &nothing, "()", NULL},
    };

    static NativeSymbol native_symbols_g[] = {
        {"call", &call_g, "()", NULL},
        {"wait", &wait, "(i)", NULL},
        {"nothing", &nothing, "()", NULL},
    };

    //
    // Init environment
    //

    wamr_env_thread_init();
    int failed = wamr_env_thread_init_env(ENV_0, env_buffer[0], sizeof(env_buffer0), WAMR_ENV_THREAD_STACK_SIZE,
                                          ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 0 failed\n");
    }
    failed = wamr_env_thread_init_env(ENV_1, env_buffer[1], sizeof(env_buffer1), WAMR_ENV_THREAD_STACK_SIZE,
                                      ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 1 failed\n");
    }

    wamr_env_thread_register_natives(ENV_0, "env", native_symbols_f, sizeof(native_symbols_f) / sizeof(NativeSymbol));
    wamr_env_thread_register_natives(ENV_1, "env", native_symbols_g, sizeof(native_symbols_g) / sizeof(NativeSymbol));

    //
    // Load modules
    //

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
        printf("Error loading module instance in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_func(ENV_0, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "f");
    if (failed)
    {
        printf("Error loading function in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }

    // ENV 1
    //      Module 1

    failed = wamr_env_thread_load_mod(ENV_1, H_FILE, H_SIZE, MODULE_SLOT_1);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    wamr_env_thread_register_module(ENV_1, "H", MODULE_SLOT_1);

    //      Module 0

    failed = wamr_env_thread_load_mod(ENV_1, G_FILE, G_SIZE, MODULE_SLOT_0);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }
    failed = wamr_env_thread_load_mod_inst(ENV_1, MODULE_SLOT_0, MODULE_INST_SLOT_0, stack_size);
    if (failed)
    {
        printf("Error loading module instance in env 0: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "g");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_1));
    }

    wamr_env_thread_call_func(ENV_0, MODULE_INST_SLOT_0, FUNCTION_SLOT_0);
    wamr_env_thread_call_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_0);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    ps();
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
