/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#include "benchmark.h"
#include "debug.h"
#include "shell.h"
#include "wamr_env_thread.h"
#include "wasm/F_aot.h"
#if NB_CONTAINERS > 1
#include "wasm/G_aot.h"
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm_c_api.h>
#include <wasm_export.h>

#include <stdalign.h>
#include <stddef.h>

#define WAMR_ENV_THREAD_STACK_SIZE 1024
#define ERROR_BUFFER_SIZE 128

const uint32_t stack_size = 1;
const uint32_t heap_size = 0;

char env_buffer0[3980];
#if NB_CONTAINERS > 1
char env_buffer1[3980];
#endif

int main(void)
{
    ztimer_sleep(ZTIMER_SEC, 5);
    wamr_env_thread_init();
    DEBUG("save default ok\n");
    int failed = wamr_env_thread_init_env(ENV_0, env_buffer0, sizeof(env_buffer0), WAMR_ENV_THREAD_STACK_SIZE,
                                          ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 0 failed\n");
    }
#if NB_CONTAINERS > 1
    failed = wamr_env_thread_init_env(ENV_1, env_buffer1, sizeof(env_buffer1), WAMR_ENV_THREAD_STACK_SIZE,
                                      ERROR_BUFFER_SIZE);
    if (failed)
    {
        printf("env init 1 failed\n");
    }
#endif
    failed = wamr_env_thread_load_mod(ENV_0, (unsigned char *)F_aot, F_aot_len, MODULE_SLOT_0);
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

#if NB_CONTAINERS > 1
    failed = wamr_env_thread_load_mod(ENV_1, (unsigned char *)G_aot, G_aot_len, MODULE_SLOT_0);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_mod_inst(ENV_1, MODULE_SLOT_0, MODULE_INST_SLOT_0, stack_size);
    if (failed)
    {
        printf("Error loading module in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
    failed = wamr_env_thread_load_func(ENV_1, MODULE_INST_SLOT_0, FUNCTION_SLOT_0, "g");
    if (failed)
    {
        printf("Error loading function in env 1: %s\n", wamr_env_thread_get_error_buffer(ENV_0));
    }
#endif
    wamr_env_thread_call_func(ENV_0, MODULE_SLOT_0, FUNCTION_SLOT_0);
#if NB_CONTAINERS > 1
    wamr_env_thread_call_func(ENV_1, MODULE_SLOT_0, FUNCTION_SLOT_0);
#endif
    DEBUG("\n==\n");

    return 0;
}
