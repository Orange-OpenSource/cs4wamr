/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#include "wamr_env.h"
#include "multi_static.h"
#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wasm_export.h>

#define ENABLE_DEBUG 0

void wamr_env_swap(wamr_env_t *env)
{
    return multi_static_swap(&env->multi_env);
}

void wamr_env_init(void)
{
    multi_static_save_default_static_values();
}

static int _wamr_env_init_struct(wamr_env_t *env, uint32_t error_buffer_size)
{
    for (int i = 0; i < WAMR_ENV_MODULE_SLOT_COUNT; i++)
    {
        env->mod[i] = NULL;
        env->code[i] = NULL;
        env->code_size[i] = 0;
    }
    for (int i = 0; i < WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT; i++)
    {
        env->mod_inst[i] = NULL;
        env->exec_env[i] = NULL;
    }
    for (int i = 0; i < WAMR_ENV_FUNCTION_SLOT_COUNT; i++)
    {
        env->func[i] = NULL;
    }
    env->error_buf = wasm_runtime_malloc(error_buffer_size);
    env->error_buf_size = error_buffer_size;

    return env->error_buf == NULL;
}

int wamr_env_init_env(wamr_env_t *env, char *buffer, uint32_t buffer_size, uint32_t error_buffer_size)
{
    multi_static_reset_env();

    RuntimeInitArgs init_args = {.mem_alloc_type = Alloc_With_Pool,
                                 .mem_alloc_option.pool.heap_buf = buffer,
                                 .mem_alloc_option.pool.heap_size = buffer_size};

    if (!wasm_runtime_full_init(&init_args))
    {
        return 1;
    }

    int failed = _wamr_env_init_struct(env, error_buffer_size);
    if (failed)
    {
        return 1;
    }
    failed = multi_static_init_env(&env->multi_env, &wasm_runtime_malloc);
    if (failed)
    {
        return 1;
    }
    return 0;
}

void wamr_env_unload_env()
{
    multi_static_reset_env();
}

int wamr_env_load_mod(wamr_env_t *env, uint8_t *code, int code_size, unsigned int mod_slot)
{
    wamr_env_swap(env);

    env->code[mod_slot] = code;
    env->code_size[mod_slot] = code_size;
    env->mod[mod_slot] =
        wasm_runtime_load(env->code[mod_slot], env->code_size[mod_slot], env->error_buf, env->error_buf_size);
    if (!env->mod[mod_slot])
    {
        return 1;
    }

    return 0;
}

int wamr_env_load_mod_inst(wamr_env_t *env, unsigned int mod_slot, unsigned int mod_inst_slot, int stack_size)
{
    wamr_env_swap(env);
    if (mod_inst_slot >= WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT)
    {
        snprintf(env->error_buf, env->error_buf_size, "Invalid module instance slot\n");
        return 1;
    }
    if (mod_slot >= WAMR_ENV_MODULE_SLOT_COUNT || !env->mod[mod_slot])
    {
        snprintf(env->error_buf, env->error_buf_size, "WAMR env Error: trying to load function on not loaded module\n");
        return 1;
    }

    env->mod_inst[mod_inst_slot] =
        wasm_runtime_instantiate(env->mod[mod_slot], stack_size, 0, env->error_buf, env->error_buf_size);
    if (!env->mod_inst[mod_inst_slot])
    {
        return 1;
    }

    env->exec_env[mod_inst_slot] = wasm_runtime_create_exec_env(env->mod_inst[mod_inst_slot], stack_size);
    DEBUG("create exec env\n");
    if (!env->exec_env[mod_inst_slot])
    {
        snprintf(env->error_buf, env->error_buf_size, "WAMR env: Runtime failed to create exec env\n");
        return 1;
    }
    return 0;
}

int wamr_env_load_func(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot, char *fct_name)
{
    wamr_env_swap(env);
    if (fct_slot >= WAMR_ENV_FUNCTION_SLOT_COUNT)
    {
        printf("Invalid function slot\n");
        return 1;
    }
    if (mod_inst_slot >= WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT || !env->mod_inst[mod_inst_slot])
    {
        snprintf(env->error_buf, env->error_buf_size,
                 "WAMR env Error: trying to load function on not loaded module instance\n");
        return 1;
    }
    env->func[fct_slot] = wasm_runtime_lookup_function(env->mod_inst[mod_inst_slot], fct_name);
    if (!env->func[fct_slot])
    {
        snprintf(env->error_buf, env->error_buf_size, "WAMR env: Runtime failed to find function\n");
        return 1;
    }

    DEBUG("lookup function\n");
    return 0;
}

int wamr_env_call_func(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot)
{
    return wamr_env_call_func_with_args(env, mod_inst_slot, fct_slot, 0, NULL);
}

int wamr_env_call_func_with_args(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot, unsigned int argc,
                                 uint32_t *argv)
{
    wamr_env_swap(env);

    bool success = wamr_env_is_func_loaded(env, mod_inst_slot, fct_slot);
    if (!success)
    {
        return !success;
    }

    success = wasm_runtime_call_wasm(env->exec_env[mod_inst_slot], env->func[fct_slot], argc, (uint32_t *)argv);
    if (!success)
    {
        printf("Error when calling function %s\n", wasm_runtime_get_exception(env->mod_inst[mod_inst_slot]));
    }

    return !success;
}

int wamr_env_register_natives(wamr_env_t *env, const char *module_name, NativeSymbol *native_symbols,
                              uint32_t n_native_symbols)
{
    wamr_env_swap(env);

    wasm_runtime_register_natives(module_name, native_symbols, n_native_symbols);
    return 0;
}

int wamr_env_register_module(wamr_env_t *env, const char *module_name, unsigned int mod_slot)
{
    wamr_env_swap(env);
    if (mod_slot >= WAMR_ENV_MODULE_SLOT_COUNT || !env->mod[mod_slot])
    {
        printf("Error: trying to load function on not loaded module\n");
        return 1;
    }
    bool success = wasm_runtime_register_module(module_name, env->mod[mod_slot], env->error_buf, env->error_buf_size);
    if (!success)
    {
        printf("Error when registering module %s\n", env->error_buf);
    }
    return !success;
}

bool wamr_env_validate_app_addr(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset, unsigned int size)
{
    if (!wamr_env_is_instance_loaded(env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_swap(env);
    return wasm_runtime_validate_app_addr(env->mod_inst[mod_inst_slot], app_offset, size);
}

void *wamr_env_addr_app_to_native(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset)
{
    if (!wamr_env_is_instance_loaded(env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_swap(env);
    return wasm_runtime_addr_app_to_native(env->mod_inst[mod_inst_slot], app_offset);
}

uint32_t wamr_env_addr_native_to_app(wamr_env_t *env, unsigned int mod_inst_slot, void *native_address)
{
    if (!wamr_env_is_instance_loaded(env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_swap(env);
    return wasm_runtime_addr_native_to_app(env->mod_inst[mod_inst_slot], native_address);
}

char *wamr_env_get_error_buffer(wamr_env_t *env)
{
    return env->error_buf;
}

void wamr_env_print(wamr_env_t *env)
{
    multi_static_print_env(&env->multi_env);
}

void wamr_env_print_info(void)
{
    multi_static_print_info();
}