/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
/**
 * @defgroup    wamr_env WAMR env
 * @brief       A library to use WAMR with multiple instances
 *
 * @{
 *
 * @file
 * @details     This library manages the usage of WAMR with multiple instances. This allows to have dedicated
 *              WAMR instances for containers. This allows to have more control in container access to native
 *              function, container memory usage (container cannot use more than the buffer dedicated to their)
 *              instance and also separate containers memory usage. 
 *
 * @author      Bastien BUIL <bastien.buil@orange.com>
 */
#ifndef WAMR_ENV_HEADER
#define WAMR_ENV_HEADER

#include "static_context_switcher.h"
#include <stddef.h>
#include <wasm_export.h>
#include <stdio.h>

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

    static_context_switcher_env_t multi_env;

    char *error_buf;
    uint32_t error_buf_size;

} wamr_env_t;

/**
 * @brief Return if the given instance is loaded
 *
 * This function is automatically called by wamr_env_call_func_with_args
 *
 * @param[in] env pointer to the environment structure of the function
 * @param[in] mod_inst_slot slot of the module instance to check
 * @return 1 if module is loaded, 0 elsewhere
 */
static inline bool wamr_env_is_instance_loaded(wamr_env_t *env, unsigned int mod_inst_slot)
{
    if (mod_inst_slot >= WAMR_ENV_MODULE_INSTANCE_SLOT_COUNT)
    {
        printf("Invalid module instance slot\n");
        return false;
    }
    if (env->mod_inst[mod_inst_slot] == NULL)
    {
        printf("Module instance not loaded\n");
        return false;
    }
    return true;
}

/**
 * @brief Return if the given function is loaded
 *
 * This function is automatically called by wamr_env_call_func_with_args
 *
 * @param[in] env pointer to the environment structure of the function
 * @param[in] mod_inst_slot slot of the module instance with the function to check
 * @param[in] fct_slot slot of the function to check
 * @return 1 if function is loaded, 0 elsewhere
 */
static inline int wamr_env_is_func_loaded(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot)
{
    if (fct_slot >= WAMR_ENV_FUNCTION_SLOT_COUNT)
    {
        printf("Invalid function slot\n");
        return 0;
    }

    if (!wamr_env_is_instance_loaded(env, mod_inst_slot))
    {
        printf("Invalid module instance slot\n");
        return 0;
    }

    if (env->func[fct_slot] == NULL)
    {
        printf("Function not loaded\n");
        return 0;
    }
    return 1;
}

/**
 * @brief Swap to the environment and call the function loaded in the given environment at the given slot
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
 *
 * @param[in] env pointer to the environment structure of the function
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
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
 *
 * @param[in] env pointer to the environment structure containing the module
 * @param[in] mod_inst_slot indice of the slot in which the module instance containing the function is loaded
 * @param[in] fct_slot indice of the slot in which the function must be loaded
 * @param[in] fct_name name of the function to load
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_load_func(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int fct_slot, char *fct_name);

/**
 * @brief Swap to the correct environment and load a module instance in a given module instance slot of a given wasm
 * module in a given environment
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
 *
 * @param[in] env pointer to the environment structure containing the module
 * @param[in] mod_slot indice of the slot in which the module containing the function is loaded
 * @param[in] mod_inst_slot indice of the slot in which the module instance will be loaded
 * @param[in] stack_size Stack size of the WAMR exec env (it is the size of the operand stack managed by WAMR and
 * containg the values and stack frame during execution)
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_load_mod_inst(wamr_env_t *env, unsigned int mod_slot, unsigned int mod_inst_slot, int stack_size);

/**
 * @brief Swap to the correct environment and load a wasm module in a given module slot in a given environment
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
 *
 * @param[in] env pointer to the environment structure in which the module should be loaded
 * @param[in] code code of the wasm module to load. The code may be modified by WAMR and should be loaded as long as the
 * module is used
 * @param[in] code_size size of the code of the wasm module to load
 * @param[in] mod_slot module slot in which the module should be loaded
 * @return 0 in case of success, another value elsewhere
 */
int wamr_env_load_mod(wamr_env_t *env, uint8_t *code, int code_size, unsigned int mod_slot);

/**
 * @brief Swap and init the given environment. If the environment is already initialized, reset the environment
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer(). Becarefull, wamr_env_get_error_buffer()
 * might return NULL if the error buffer failed to be allocated
 *
 * @param[in] env pointer to the environment structure to initialize. The structure should be allocated but should not be initialized
 * @param[in] buffer buffer dedicated to the environment
 * @param[in] buffer_size size of the given buffer
 * @param[in] error_buffer_size size to allocate for the error buffer
 * @return 0 in case of success, other values elsewhere
 */
int wamr_env_init_env(wamr_env_t *env, char *buffer, uint32_t buffer_size, uint32_t error_buffer_size);

/**
 * @brief Initialize the wamr_env library. This call is needed before using the library and using the WAMR library
 */
void wamr_env_init(void);

/**
 * @brief Unload the current environment, restore WAMR statics to their default state
 */
void wamr_env_unload_env(void);

/**
 * @brief Swap to the given environment
 *
 * @param[in] env pointer to the environment structure to swap to
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
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
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
 * If the returns is not 0, you can get error string with wamr_env_get_error_buffer()
 *
 * @param[in] env indice of environment in which the module must be registered
 * @param[in] module_name name to be given to the module
 * @param[in] mod_slot Module slot in which the module is already loaded
 * @return 0 in case of success, other value elsewhere
 */
int wamr_env_register_module(wamr_env_t *env, const char *module_name, unsigned int mod_slot);

/**
 * @brief Get the error buffer of wamr_env.
 * The error buffer is allocated, unless the wamr_env is not initialized, so you should only check
 * if the returns value is not NULL after calling wamr_env_init_env()
 *
 * @return error buffer containing string explaining the error or NULL if the env is not initialized.
 */
char *wamr_env_get_error_buffer(wamr_env_t *env);

bool wamr_env_validate_app_addr(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset,
                                unsigned int size);

void *wamr_env_addr_app_to_native(wamr_env_t *env, unsigned int mod_inst_slot, unsigned int app_offset);

uint32_t wamr_env_addr_native_to_app(wamr_env_t *env, unsigned int mod_inst_slot, void *native_address);

void wamr_env_print(wamr_env_t *env);
void wamr_env_print_info(void);

#endif

/** @} */
