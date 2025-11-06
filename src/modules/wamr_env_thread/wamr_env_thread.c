/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "wamr_env_thread.h"
#ifndef BOARD_NATIVE
#include "irq_arch.h"
#endif
#include "wamr_env.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define WAMR_ENV_DEBUG 0
#define WAMR_ENV_DEBUG_MUTEX 0

wamr_env_thread_t env_threads[ENV_AMOUNT];
ztimer_t env_timers[ENV_AMOUNT]; // Cannot be in env_thread as ztimer_base_t copy might break list !

wamr_env_thread_number_t wamr_env_thread_get_env_from_pid(kernel_pid_t pid)
{
    for (int i = 0; i < ENV_AMOUNT; i++)
    {
        if (env_threads[i].env_state >= ENV_RUNNING && env_threads[i].thread_pid == pid)
        {
            return i;
        }
    }
    return -1;
}

kernel_pid_t wamr_env_thread_get_pid(wamr_env_thread_number_t env)
{
    if (env_threads[env].env_state == ENV_UNINITIALIZED)
    {
        return -1;
    }
    return env_threads[env].thread_pid;
}

void wamr_env_thread_swap(wamr_env_thread_number_t env)
{
#ifndef BOARD_NATIVE
    unsigned state = irq_disable();
#endif
    if (env_threads[env].env_state == ENV_UNINITIALIZED)
    {
        printf("Swapping to unintialized thread %" PRIu32 "\n", env);
#ifndef BOARD_NATIVE
        irq_restore(state);
#endif
        return;
    }
    if (env_threads[env].env_state == ENV_IN_SUB_ENV)
    {
        printf("Swapping to environment: running another environment (Sub env state): env %" PRIu32 "\n", env);
    }
    wamr_env_swap(&env_threads[env].env);
#ifndef BOARD_NATIVE
    irq_restore(state);
#endif
}

static void _wamr_env_thread_sched_callback(kernel_pid_t active, kernel_pid_t next)
{
#if WAMR_ENV_DEBUG == 1
    // printf("Changing thread %d to %d\n", active, next);
#endif
    wamr_env_thread_number_t env = wamr_env_thread_get_env_from_pid(next);
    if (env != -1)
    {
        wamr_env_thread_swap(env);
    }
#if WAMR_ENV_DEBUG == 1
    // printf("No thread matched with pid=%d\n", next);
#endif
}

int _wamr_env_thread_clear(wamr_env_thread_number_t env, uint32_t thread_stack_size, char *buffer, uint32_t buffer_size)
{
    env_threads[env].thread_stack = wasm_runtime_malloc(thread_stack_size);
    env_threads[env].thread_stack_size = thread_stack_size;
    env_threads[env].thread_pid = -1;
    env_threads[env].env_state = ENV_INITIALIZED;
    env_threads[env].buffer = buffer;
    env_threads[env].buffer_size = buffer_size;
    return env_threads[env].thread_stack == NULL;
}

/**
 * Issue is this initialization is context switched ?
 */
int wamr_env_thread_init_env(wamr_env_thread_number_t env, char *buffer, uint32_t buffer_size,
                             uint32_t thread_stack_size, uint32_t error_buffer_size)
{
#if WAMR_ENV_DEBUG == 1
    printf("init env wamr_env\n");
#endif
    int failed = wamr_env_init_env(&env_threads[env].env, buffer, buffer_size, error_buffer_size);
#if WAMR_ENV_DEBUG == 1
    printf("init env wamr_env done\n");
#endif
    int failed2 = _wamr_env_thread_clear(env, thread_stack_size, buffer, buffer_size);
#if WAMR_ENV_DEBUG == 1
    printf("init env wamr_env cleared\n");
#endif
    return failed || failed2;
}

static void _callback_unlock_mutex(void *arg)
{
#if WAMR_ENV_DEBUG_MUTEX == 1
    printf("unlock mutex\n");
#endif
    mutex_t *mutex = (mutex_t *)arg;
    mutex_unlock(mutex);
}

void wamr_env_thread_init(void)
{
    for (int i = 0; i < ENV_AMOUNT; i++)
    {
        env_threads[i].env_state = ENV_UNINITIALIZED;
        env_threads[i].mutex = (mutex_t)MUTEX_INIT;
        env_timers[i] = (ztimer_t){
            .callback = _callback_unlock_mutex,
            .arg = (void *)&env_threads[i].mutex,
        };
    }
    wamr_env_init();
    sched_register_cb(&_wamr_env_thread_sched_callback);
}

static const uint32_t max_slot_in_options = 256;

static void *_start_thread_func(void *args)
{
    /*uint32_t *args_uint = args;
    wamr_env_number_t env = args_uint[0];
    int fct_slot = args_uint[1];*/
    uint32_t args_n = (uint32_t)args;
    wamr_env_thread_number_t env = args_n % max_slot_in_options;
    unsigned int mod_inst_slot = (args_n / max_slot_in_options) % max_slot_in_options;
    unsigned int fct_slot = (args_n / (max_slot_in_options * max_slot_in_options)) % max_slot_in_options;
#if WAMR_ENV_DEBUG == 1
    printf("New thread pid=%d env=%ld inst=%d func=%d\n", thread_getpid(), env, mod_inst_slot, fct_slot);
#endif
    env_threads[env].thread_pid = thread_getpid();
    env_threads[env].env_state = ENV_RUNNING;
    wamr_env_call_func(&env_threads[env].env, mod_inst_slot, fct_slot);

#if WAMR_ENV_DEBUG == 1
    printf("End of thread pid=%d\n", thread_getpid());
#endif
    env_threads[env].thread_pid = -1;
    env_threads[env].env_state = ENV_STOPPED;
    return NULL;
}

int wamr_env_thread_call_func(wamr_env_thread_number_t env, unsigned int mod_inst_slot, unsigned int fct_slot)
{
    uint32_t args = env + max_slot_in_options * mod_inst_slot + (max_slot_in_options * max_slot_in_options) * fct_slot;
#if WAMR_ENV_DEBUG == 1
    printf("Call %ld %d\n", env, fct_slot);
#endif
    kernel_pid_t pid = thread_create(
        env_threads[env].thread_stack, env_threads[env].thread_stack_size,
        // THREAD_PRIORITY_IDLE - 1, THREAD_CREATE_WOUT_YIELD, &_start_thread_func, (void*)args, "wamr env thread");
        THREAD_PRIORITY_IDLE - 1, THREAD_CREATE_WOUT_YIELD, &_start_thread_func, (void *)args, "wamr env thread");

#if WAMR_ENV_DEBUG == 1
    printf("env call func env=%ld pid=%d\n", env, pid);
#endif
    return 0;
}

int wamr_env_thread_call_func_sub_env_with_args(wamr_env_thread_number_t new_env, unsigned int mod_inst_slot,
                                                unsigned int fct_slot, unsigned int argc, uint32_t *argv, bool wait)
{
    kernel_pid_t pid = thread_getpid();
    if (pid == -1)
    {
        return 1;
    }
    wamr_env_thread_number_t current_env = wamr_env_thread_get_env_from_pid(pid);
    env_threads[current_env].env_state = ENV_IN_SUB_ENV;

    while (env_threads[new_env].env_state >= ENV_RUNNING)
    {
        if (!wait)
        {
            printf("Cannot run sub env function if env is already running\n");
            return 1;
        }
        thread_yield();
    }

    int failed = wamr_env_thread_call_func_plain_exec_with_args(new_env, mod_inst_slot, fct_slot, argc, argv);

    env_threads[current_env].env_state = ENV_RUNNING;
    wamr_env_thread_swap(current_env);
    return failed;
}

int wamr_env_thread_call_func_plain_exec_with_args(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                                   unsigned int fct_slot, unsigned int argc, uint32_t *argv)
{
    env_threads[env].thread_pid = thread_getpid();
    env_threads[env].env_state = ENV_RUNNING;
    wamr_env_thread_swap(env);

    bool success = wamr_env_is_func_loaded(&env_threads[env].env, mod_inst_slot, fct_slot);
    if (!success)
    {
        return !success;
    }

    success = wasm_runtime_call_wasm(env_threads[env].env.exec_env[mod_inst_slot], env_threads[env].env.func[fct_slot],
                                     argc, (uint32_t *)argv);
    if (!success)
    {
        printf("wamr_env_thread_call_func_plain_exec_with_args: Error when calling function %s\n",
               wasm_runtime_get_exception(env_threads[env].env.mod_inst[mod_inst_slot]));
    }
    env_threads[env].thread_pid = -1;
    env_threads[env].env_state = ENV_STOPPED;
    return !success;
}

int wamr_env_thread_call_func_sub_env(wamr_env_thread_number_t new_env, unsigned int mod_inst_slot,
                                      unsigned int fct_slot, bool wait)
{
    return wamr_env_thread_call_func_sub_env_with_args(new_env, mod_inst_slot, fct_slot, 0, NULL, wait);
}

bool wamr_env_thread_is_env_paused(wamr_env_thread_number_t env)
{
    return env_threads[env].env_state == ENV_PAUSED;
}

void wamr_env_thread_pause(wamr_env_thread_number_t env)
{
    if (env_threads[env].env_state == ENV_RUNNING)
    {
        thread_t *thread = thread_get(env_threads[env].thread_pid);
        env_threads[env].env_state = ENV_PAUSED;
        sched_set_status(thread, STATUS_SLEEPING);
    }
}
void wamr_env_thread_resume(wamr_env_thread_number_t env)
{
    if (env_threads[env].env_state == ENV_PAUSED)
    {
        thread_t *thread = thread_get(env_threads[env].thread_pid);
        env_threads[env].env_state = ENV_RUNNING;
        sched_set_status(thread, STATUS_PENDING);
    }
}

void wamr_env_thread_save(wamr_env_thread_number_t env, wamr_env_thread_t *saving_env_thread, char *saving_buffer,
                          uint32_t saving_buffer_size)
{
    if (env_threads[env].buffer_size > saving_buffer_size)
    {
        printf("Saving buffer too small\n");
        return;
    }
    if (env_threads[env].env_state != ENV_PAUSED && env_threads[env].env_state != ENV_STOPPED)
    {
        printf("Env should be paused before saving !\n");
        return;
    }
#ifndef BOARD_NATIVE
    unsigned state = irq_disable();
#endif
    memcpy(saving_env_thread, &env_threads[env], sizeof(wamr_env_thread_t));
    memcpy(saving_buffer, env_threads[env].buffer, env_threads[env].buffer_size);
#ifndef BOARD_NATIVE
    irq_restore(state);
#endif
}

void wamr_env_thread_restore(wamr_env_thread_number_t env, wamr_env_thread_t *saved_env_thread, char *saved_buffer,
                             uint32_t saved_buffer_size)
{
    if (env_threads[env].buffer_size > saved_buffer_size)
    {
        printf("Saved buffer too small\n");
        return;
    }
#ifndef BOARD_NATIVE
    unsigned state = irq_disable();
#endif
    if (env_threads[env].env_state >= ENV_RUNNING && env_threads[env].thread_pid != -1)
    {
        thread_t *thread = thread_get(env_threads[env].thread_pid);
        sched_set_status(thread, STATUS_SLEEPING);
    }
    memcpy(&env_threads[env], saved_env_thread, sizeof(wamr_env_thread_t));
    memcpy(env_threads[env].buffer, saved_buffer, env_threads[env].buffer_size);
    if (env_threads[env].env_state == ENV_PAUSED)
    {
        wamr_env_thread_pause(env);
    }
#ifndef BOARD_NATIVE
    irq_restore(state);
#endif
}

void wamr_env_thread_sleep_time(ztimer_clock_t *clock, uint32_t duration)
{
    wamr_env_thread_number_t env = wamr_env_thread_get_env_from_pid(thread_getpid());

    if (env == -1)
    {
        return;
    }
#ifndef BOARD_NATIVE
    assert(!irq_is_in());
#endif
    env_threads[env].mutex = (mutex_t)MUTEX_INIT_LOCKED;

    /* correct board / MCU specific overhead */
    if (duration > clock->adjust_sleep)
    {
        duration -= clock->adjust_sleep;
    }
    else
    {
        duration = 0;
    }

#if WAMR_ENV_DEBUG_MUTEX == 1
    printf("Set ztimer %ld\n", env);
#endif
    ztimer_set(clock, &env_timers[env], duration);
#if WAMR_ENV_DEBUG_MUTEX == 1
    printf("Mutex lock %ld\n", env);
#endif
    mutex_lock(&env_threads[env].mutex);
#if WAMR_ENV_DEBUG_MUTEX == 1
    printf("Mutex unlock %ld\n", env);
#endif
    if (wamr_env_thread_is_env_paused(env))
    {
#if WAMR_ENV_DEBUG_MUTEX == 1
        printf("Mutex thread pause %ld\n", env);
#endif
        thread_sleep();
    }
}

/**
 * Facade function
 */

int wamr_env_thread_register_natives(wamr_env_thread_number_t env, const char *module_name,
                                     NativeSymbol *native_symbols, uint32_t n_native_symbols)
{
    return wamr_env_register_natives(&env_threads[env].env, module_name, native_symbols, n_native_symbols);
}

int wamr_env_thread_register_module(wamr_env_thread_number_t env, const char *module_name, unsigned int mod_slot)
{
    return wamr_env_register_module(&env_threads[env].env, module_name, mod_slot);
}

int wamr_env_thread_load_func(wamr_env_thread_number_t env, unsigned int mod_inst_slot, unsigned int fct_slot,
                              char *fct_name)
{
    return wamr_env_load_func(&env_threads[env].env, mod_inst_slot, fct_slot, fct_name);
}

int wamr_env_thread_load_mod_inst(wamr_env_thread_number_t env, unsigned int mod_slot, unsigned int mod_inst_slot,
                                  int stack_size)
{
    return wamr_env_load_mod_inst(&env_threads[env].env, mod_slot, mod_inst_slot, stack_size);
}

int wamr_env_thread_load_mod(wamr_env_thread_number_t env, uint8_t *code, int code_size, unsigned int mod_slot)
{
    return wamr_env_load_mod(&env_threads[env].env, code, code_size, mod_slot);
}

bool wamr_env_thread_validate_app_addr(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                       unsigned int app_offset, unsigned int size)
{
    if (!wamr_env_is_instance_loaded(&env_threads[env].env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_thread_swap(env);
    return wasm_runtime_validate_app_addr(env_threads[env].env.mod_inst[mod_inst_slot], app_offset, size);
}

void *wamr_env_thread_addr_app_to_native(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                         unsigned int app_offset)
{
    if (!wamr_env_is_instance_loaded(&env_threads[env].env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_thread_swap(env);
    return wasm_runtime_addr_app_to_native(env_threads[env].env.mod_inst[mod_inst_slot], app_offset);
}

uint32_t wamr_env_thread_addr_native_to_app(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                            void *native_address)
{
    if (!wamr_env_is_instance_loaded(&env_threads[env].env, mod_inst_slot))
    {
        return false;
    }
    wamr_env_thread_swap(env);
    return wasm_runtime_addr_native_to_app(env_threads[env].env.mod_inst[mod_inst_slot], native_address);
}

int wamr_env_thread_load_mod_preserve_code(wamr_env_thread_number_t env, uint8_t *code, int code_size,
                                           unsigned int mod_slot)
{
    wamr_env_thread_swap(env);
    uint8_t *new_code = wasm_runtime_malloc(code_size);
    if (code == NULL)
    {
        snprintf(env_threads[env].env.error_buf, env_threads[env].env.error_buf_size,
                 "WAMR env Error: trying to load function on not loaded module\n");
        return 1;
    }
    memcpy(new_code, code, code_size);
    return wamr_env_load_mod(&env_threads[env].env, new_code, code_size, mod_slot);
}

char *wamr_env_thread_get_error_buffer(wamr_env_thread_number_t env)
{
    return env_threads[env].env.error_buf;
}
bool wamr_env_thread_is_env_initialized(wamr_env_thread_number_t env)
{
    return env_threads[env].env_state != ENV_UNINITIALIZED;
}

void wamr_env_thread_print(wamr_env_thread_number_t env)
{
    printf("wamr env thread:\n");
    printf("\tthread stack %p, thread stack size: %" PRIu32 "\n", env_threads[env].thread_stack,
           env_threads[env].thread_stack_size);
    printf("\tenv_state %d\n", env_threads[env].env_state);
    wamr_env_print(&env_threads[env].env);
}
void wamr_env_thread_print_info(void)
{
    wamr_env_print_info();
}