/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#ifndef WAMR_ENV_THREAD_HEADER
#define WAMR_ENV_THREAD_HEADER

#include "wamr_env.h"
#include <ps.h>
#include <sched.h>
#include <thread.h>
#include <thread_config.h>
#include <ztimer.h>

#define ENV_AMOUNT 3

#define ENV_0 0
#define ENV_1 1
#define ENV_2 2

typedef int8_t wamr_env_thread_env_state;
#define ENV_UNINITIALIZED 0
#define ENV_INITIALIZED 2
#define ENV_STOPPED 10
#define ENV_IN_SUB_ENV 12
#define ENV_RUNNING 15 // All state >= ENV_RUNNING means the env run or is sleep, waiting state
#define ENV_PAUSED 20

/**
 * @brief Structure to store the state of a wamr_env_thread environment.
 */
typedef struct wamr_env_thread_s
{
    char *buffer;
    uint32_t buffer_size;
    char *thread_stack;
    uint32_t thread_stack_size;
    wamr_env_t env;

    kernel_pid_t thread_pid;
    wamr_env_thread_env_state env_state;
    mutex_t mutex;
} wamr_env_thread_t;

/**
 * @brief Indice of WAMR environment
 */
typedef int32_t wamr_env_thread_number_t;

/**
 * @brief Get the environment associated with a thread
 *
 * @param[in] pid Pid of the thread associated to the environment
 * @return Indice of the environment
 */
wamr_env_thread_number_t wamr_env_thread_get_env_from_pid(kernel_pid_t pid);

/**
 * @brief Check if environment is paused
 *
 * @param[in] env environment to check
 * @return 1 if the environment is paused, 0 elsewhere
 */
bool wamr_env_thread_is_env_paused(wamr_env_thread_number_t env);

/**
 * @brief Swap and init the given environment. If the environment is already initialized, reset the environment
 *
 * @param[in] env indice of the environment to load
 * @param[in] buffer Buffer with which the environment should be initialized
 * @param[in] buffer_size The size of the given buffer
 * @param[in] thread_stack_size The size that should be allocated from the buffer for the OS native thread stack
 * @param[in] error_buffer_size The size that should be allocated from the buffer for the error buffer. Recommanded
 * value: 128
 * @return 0 in case of success, other values elsewhere
 */
int wamr_env_thread_init_env(wamr_env_thread_number_t env, char *buffer, uint32_t buffer_size,
                             uint32_t thread_stack_size, uint32_t error_buffer_size);

/**
 * @brief Init the wamr_env_thread library. This call also init the wamr_env library used underneath
 */
void wamr_env_thread_init(void);

/**
 * @brief Create a new thread and call the function loaded in the given environment at the given slot.
 * When the thread will be running, a swap will be automatically done to put the correct environment.
 *
 * @param[in] env indice of the environment of the function
 * @param[in] mod_inst_slot slot of the module instance to run
 * @param[in] fct_slot slot of the function to run
 * @return 0 in case of success, other values elsewhere
 */
int wamr_env_thread_call_func(wamr_env_thread_number_t env, unsigned int mod_inst_slot, unsigned int fct_slot);

/**
 * @brief From a thread associated to an environment, call a function of another environment. This function can be
 * called from native function.
 * @param[in] new_env indice of the environment of the function to call
 * @param[in] mod_inst_slot slot of the module instance to run
 * @param[in] fct_slot slot of the function to run
 * @param[in] argc number of argument to pass the the function
 * @param[in,out] argv argument to pass the function. If the function return any value, argv is updated with the return
 * values.
 * @param[in] wait_if_running number of argument to pass the the function
 */
int wamr_env_thread_call_func_sub_env_with_args(wamr_env_thread_number_t new_env, unsigned int mod_inst_slot,
                                                unsigned int fct_slot, unsigned int argc, uint32_t *argv,
                                                bool wait_if_running);

int wamr_env_thread_call_func_sub_env(wamr_env_thread_number_t new_env, unsigned int mod_inst_slot,
                                      unsigned int fct_slot, bool wait_if_running);

int wamr_env_thread_call_func_plain_exec_with_args(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                                   unsigned int fct_slot, unsigned int argc, uint32_t *argv);

int wamr_env_thread_call_func_with_args(wamr_env_thread_number_t env, unsigned int fct_slot, unsigned int argc,
                                        uint32_t *argv);

/**
 * @brief Make the thread, associated with the given environment, sleep
 *
 * @param[in] env indice of the environment to pause
 */
void wamr_env_thread_pause(wamr_env_thread_number_t env);

/**
 * @brief Make the thread, associated with the given environment, resume. The thread is put in pending state.
 *
 * @param[in] env indice of the environment to resume
 */
void wamr_env_thread_resume(wamr_env_thread_number_t env);

/**
 * @brief
 *
 * @param[in] env
 * @param[out] saving_env
 * @param[out] saving_env_thread
 */
void wamr_env_thread_save(wamr_env_thread_number_t env, wamr_env_thread_t *saving_env_thread, char *saving_buffer,
                          uint32_t saving_buffer_size);

/**
 * @brief
 *
 * @param[in] env
 * @param[in] saving_env
 * @param[in] saving_env_thread
 */
void wamr_env_thread_restore(wamr_env_thread_number_t env, wamr_env_thread_t *saved_env_thread, char *saved_buffer,
                              uint32_t saved_buffer_size);

/**
 * @brief Sleep the environment of the current thread for the given duration
 *
 * @param[in] clock z_timer clock
 * @param[in] duration duraction in the unit of the clock
 */
void wamr_env_thread_sleep_time(ztimer_clock_t *clock, uint32_t duration); // Sleep the env of the current thread

/*
 * These functions are direct call to the wamr env functions
 */

/**
 * @brief Swap and register native functions callable in the environment
 * Note: WASM runtime will not allocate memory to clone the native symbols, so user must ensure the array can be used
 * forever
 *
 * @param[in] env indice of environment in which native functions must be registered
 * @param[in] module_name name of the module of the native functions (if you do not know the good value, use "env")
 * @param[in] native_symbols Array of native_symbols
 * @param[in] n_native_symbols Size of the array of native_symbols
 * @return 0 in case of success, other value elsewhere
 */
int wamr_env_thread_register_natives(wamr_env_thread_number_t env, const char *module_name,
                                     NativeSymbol *native_symbols, uint32_t n_native_symbols);

/**
 * @brief Swap and register module, to be callable by other modules
 *
 * @param[in] env indice of environment in which the module must be registered
 * @param[in] module_name name to be given to the module
 * @param[in] mod_slot Module slot in which the module is already loaded
 * @return 0 in case of success, other value elsewhere
 */
int wamr_env_thread_register_module(wamr_env_thread_number_t env, const char *module_name, unsigned int mod_slot);

/**
 * @brief Swap to the correct environment and load a function of a instance module in a given function slot of a given
 * wasm module in a given environment
 *
 * @param[in] env indice of the environment containing the module
 * @param[in] mod_inst_slot indice of the slot in which the module instance containing the function is loaded
 * @param[in] fct_slot indice of the slot in which the function must be loaded
 * @param[in] fct_name name of the function to load
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_thread_load_func(wamr_env_thread_number_t env, unsigned int mod_inst_slot, unsigned int fct_slot,
                              char *fct_name);

/**
 * @brief Swap to the correct environment and load a module instance in a given module instance slot of a given wasm
 * module in a given environment
 *
 * @param[in] env indice of the environment containing the module
 * @param[in] mod_slot indice of the slot in which the module containing the function is loaded
 * @param[in] mod_inst_slot indice of the slot in which the module instance will be loaded
 * @param[in] stack_size Stack size of the WAMR exec env (it is the size of the operand stack managed by WAMR and
 * containg the values and stack frame during execution)
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_thread_load_mod_inst(wamr_env_thread_number_t env, unsigned int mod_slot, unsigned int mod_inst_slot,
                                  int stack_size);

/**
 * @brief Swap to the correct environment and load a wasm module in a given module slot in a given environment
 *
 * @param[in] env indice of the environment in which the module should be loaded
 * @param[in] code code of the wasm module to load. The code may be modified by WAMR and should be loaded as long as the
 * module is used
 * @param[in] code_size size of the code of the wasm module to load
 * @param[in] mod_slot module slot in which the module should be loaded
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_thread_load_mod(wamr_env_thread_number_t env, uint8_t *code, int code_size, unsigned int mod_slot);

int wamr_env_thread_load_mod_preserve_code(wamr_env_thread_number_t env, uint8_t *code, int code_size,
                                           unsigned int mod_slot);

void wamr_env_thread_swap(wamr_env_thread_number_t env);
char *wamr_env_thread_get_error_buffer(wamr_env_thread_number_t env);
kernel_pid_t wamr_env_thread_get_pid(wamr_env_thread_number_t env);
bool wamr_env_thread_is_env_initialized(wamr_env_thread_number_t env);

bool wamr_env_thread_validate_app_addr(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                       unsigned int app_offset, unsigned int size);
void *wamr_env_thread_addr_app_to_native(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                         unsigned int app_offset);
uint32_t wamr_env_thread_addr_native_to_app(wamr_env_thread_number_t env, unsigned int mod_inst_slot,
                                            void *native_address);

void wamr_env_thread_print(wamr_env_thread_number_t env);
void wamr_env_thread_print_info(void);
#endif