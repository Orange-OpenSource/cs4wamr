/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#ifndef WAMR_ENV_HEADER
#define WAMR_ENV_HEADER

#include "multi_static.h"
#include <wasm_export.h>

#define WAMR_ENV_FUNCTION_SLOT_COUNT 5
#define WAMR_ENV_MODULE_SLOT_COUNT 3
#define WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT 3

#define FUNCTION_SLOT_0 0
#define FUNCTION_SLOT_1 1
#define FUNCTION_SLOT_2 2
#define FUNCTION_SLOT_3 3
#define FUNCTION_SLOT_4 4
#define MODULE_SLOT_0 0
#define MODULE_SLOT_1 1
#define MODULE_SLOT_2 2
#define MODULE_INST_SLOT_0 0
#define MODULE_INST_SLOT_1 1
#define MODULE_INST_SLOT_2 2

/**
 * @brief Structure to store the state of a wamr_env environment.
 */
typedef struct wamr_env_s
{
    wasm_module_inst_t mod_inst[WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT];
    wasm_exec_env_t exec_env[WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT];

    wasm_function_inst_t func[WAMR_ENV_FUNCTION_SLOT_COUNT];

    wasm_module_t mod[WAMR_ENV_MODULE_SLOT_COUNT];
    uint8_t *code[WAMR_ENV_MODULE_SLOT_COUNT];
    int code_size[WAMR_ENV_MODULE_SLOT_COUNT];

    multi_static_env_t multi_env;

    char *error_buf;
    uint32_t error_buf_size;

} wamr_env_t;

/**
 * @brief Swap to the environment and call the function loaded in the given environment at the given slot
 *
 * @param[in] env indice of the environment of the function
 * @param[in] mod_inst_slot slot of the module instance to run
 * @param[in] fct_slot slot of the function to run
 * @return 0 in case of success, other values elsewhere
 */
int wamr_env_call_func(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot);

int wamr_env_call_func_with_args(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot, unsigned int argc,
                                 uint32_t *argv);

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
int wamr_env_load_func(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot, char *fct_name);

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
int wamr_env_load_mod_inst(wamr_env_t *env, unsigned int mod_slot, unsigned int mod_inst_slot, int stack_size);

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
int wamr_env_load_mod(wamr_env_t *env, uint8_t *code, int code_size, unsigned int mod_slot);

/**
 * @brief Swap and init the given environment. If the environment is already initialized, reset the environment
 *
 * @param[in] env indice of the environment to load
 * @return 0 in case of success, other values elsewhere
 */
int wamr_env_init_env(wamr_env_t *env, char *buffer, uint32_t buffer_size, uint32_t error_buffer_size);

/**
 * @brief Initialize the wamr_env library. This call is needed before using the library and using the WAMR library
 */
void wamr_env_init(void);

/**
 * @brief Swap to the given environment
 *
 * @param[in] env indice of the environment to swap to
 */
void wamr_env_swap(wamr_env_t *env);

/*
 *  Note: WASM runtime will not allocate memory to clone the native symbol, so user must ensure the array can be used
 * forever
 */

/**
 * @brief Register native functions callable in the environment
 * Note: WASM runtime will not allocate memory to clone the native symbols, so user must ensure the array can be used
 * forever
 *
 * @param[in] env indice of environment in which native functions must be registered
 * @param[in] module_name name of the module of the native functions (if you do not know the good value, use "env")
 * @param[in] native_symbols Array of native_symbols
 * @param[in] n_native_symbols Size of the array of native_symbols
 * @return 0 in case of success, other value elsewhere
 */
int wamr_env_register_natives(wamr_env_t *env, const char *module_name, NativeSymbol *native_symbols,
                              uint32_t n_native_symbols);

/**
 * @brief Swap and register module, to be callable by other modules
 *
 * @param[in] env indice of environment in which the module must be registered
 * @param[in] module_name name to be given to the module
 * @param[in] mod_slot Module slot in which the module is already loaded
 * @return 0 in case of success, other value elsewhere
 */
int wamr_env_register_module(wamr_env_t *env, const char *module_name, unsigned int mod_slot);

char *wamr_env_get_error_buffer(wamr_env_t *env);
bool wamr_env_validate_app_addr(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset,
                                unsigned int size);

void *wamr_env_addr_app_to_native(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset);

uint32_t wamr_env_addr_native_to_app(wamr_env_t *env, unsigned int mod_inst_slot, void *native_address);

void wamr_env_print(wamr_env_t *env);
void wamr_env_print_info(void);

#endif