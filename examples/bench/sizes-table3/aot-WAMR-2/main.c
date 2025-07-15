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
#include "wasm/F_aot.h"
#if NB_CONTAINERS > 1
#include "wasm/G_aot.h"
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm_c_api.h>
#include <wasm_export.h>

#include <benchmark.h>
#include <stdalign.h>
#include <stddef.h>
#include <ztimer.h>

#define WAMR_ENV_THREAD_STACK_SIZE 4096
#define ERROR_BUFFER_SIZE 128

const uint32_t stack_size = 1;
const uint32_t heap_size = 0;

#if NB_CONTAINERS == 1
char env_buffer0[2710];
#elif NB_CONTAINERS == 2
char env_buffer0[5190];
#else
#error Unsupported NB of containers
#endif
char error_buffer[ERROR_BUFFER_SIZE];

int main(void)
{

    ztimer_sleep(ZTIMER_SEC, 5);
    RuntimeInitArgs init_args = {.mem_alloc_type = Alloc_With_Pool,
                                 .mem_alloc_option.pool.heap_buf = env_buffer0,
                                 .mem_alloc_option.pool.heap_size = sizeof(env_buffer0)};

    if (!wasm_runtime_full_init(&init_args))
    {
        printf("env init 0 failed\n");
        return 1;
    }
    wasm_module_t module = wasm_runtime_load((unsigned char *)F_aot, F_aot_len, error_buffer, sizeof(error_buffer));

    wasm_module_inst_t mod_inst = wasm_runtime_instantiate(module, stack_size, 0, error_buffer, sizeof(error_buffer));
    if (!mod_inst)
    {
        printf("WAMR error: %s\n", error_buffer);
        return 1;
    }

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(mod_inst, stack_size);
    if (!exec_env)
    {
        printf("WAMR env: Runtime failed to create exec env\n");
        return 1;
    }
    wasm_function_inst_t ffunc = wasm_runtime_lookup_function(mod_inst, "f");
    if (!ffunc)
    {
        printf("WAMR env: Runtime failed to find f function\n");
        return 1;
    }

#if NB_CONTAINERS > 1
    wasm_module_t module2 = wasm_runtime_load((unsigned char *)G_aot, G_aot_len, error_buffer, sizeof(error_buffer));

    wasm_module_inst_t mod_inst2 = wasm_runtime_instantiate(module2, stack_size, 0, error_buffer, sizeof(error_buffer));
    if (!mod_inst2)
    {
        printf("WAMR error: %s\n", error_buffer);
        return 1;
    }

    wasm_exec_env_t exec_env2 = wasm_runtime_create_exec_env(mod_inst2, stack_size);
    if (!exec_env2)
    {
        printf("WAMR env: Runtime failed to create exec env\n");
        return 1;
    }
    wasm_function_inst_t ffunc2 = wasm_runtime_lookup_function(mod_inst2, "g");
    if (!ffunc2)
    {
        printf("WAMR env: Runtime failed to find g function\n");
        return 1;
    }
#endif

    bool success = wasm_runtime_call_wasm(exec_env, ffunc, 0, NULL);
    if (!success)
    {
        printf("Error env 0\n");
        return 1;
    }

#if NB_CONTAINERS > 1
    success = wasm_runtime_call_wasm(exec_env2, ffunc2, 0, NULL);
    if (!success)
    {
        printf("Error env 1\n");
        return 1;
    }
#endif

    return 0;
}
