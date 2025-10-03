/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
/**
 * @defgroup    static_context_switcher CS Multi Static
 * @brief       A library to manage context switching for code using static variable
 *
 * @{
 *
 * @file
 * @details     This library can be used with the elf reader tool to switch between different instances of 
 *              an application/library using static variables to store its state. The mecanism works by 
 *              saving the static variables to store the instance, then recopying back the static variables
 *              to reload the instance.
 *
 * @author      Bastien BUIL <bastien.buil@orange.com>
 */

#ifndef STATIC_CONTEXT_SWITCHER_HEADER
#define STATIC_CONTEXT_SWITCHER_HEADER

#include <stdint.h>

#ifndef STATIC_CONTEXT_SWITCHER_MAX_STATIC_SIZE
#define STATIC_CONTEXT_SWITCHER_MAX_STATIC_SIZE 128
#endif

#ifndef STATIC_CONTEXT_SWITCHER_MAX_NUMBER_STATIC
#define STATIC_CONTEXT_SWITCHER_MAX_NUMBER_STATIC 4
#endif

/**
 * Environment structure for the variable on the state of an instance of the target library
 */
typedef struct static_context_switcher_env_s
{
    uint8_t *static_values[STATIC_CONTEXT_SWITCHER_MAX_NUMBER_STATIC];
    uint32_t static_values_size[STATIC_CONTEXT_SWITCHER_MAX_NUMBER_STATIC]; // Can possibly be remove
} static_context_switcher_env_t;

/**
 * @brief Structure containing the address and the size of some static values
 */
typedef struct static_context_switcher_external_values_s
{
    uint32_t ptr;
    uint32_t size;
} static_context_switcher_static_values_t;

/**
 * @brief Function to save the default state of the target library. The function must be run before using the
 * multi-static library
 */
void static_context_switcher_save_default_static_values(void);

/**
 * @brief Reset the target library to its default state
 */
void static_context_switcher_reset_env(void);

/**
 * @brief Swap the environment of the target library to the given environment
 * @param[in] env Pointer to the structure of the environment to swap to
 */
void static_context_switcher_swap(static_context_switcher_env_t *env);

/**
 * @brief Initialize a new environment for the target library. The environment structure is used by the swapping
 * mechanism.
 * @param[out] env Structure of the newly created environment.
 * @param[in] malloc_func Malloc function used to initialize the environment
 * @return 0 in case of success, 1 elsewhere
 */
int static_context_switcher_init_env(static_context_switcher_env_t *env, void *(*malloc_func)(unsigned int));

/**
 * @brief Print pointers information of the given environment. It notably prints what are the saved buffer from the
 * target library.
 * @param[in] env Env to print information from
 */
void static_context_switcher_print_env(static_context_switcher_env_t *env);

/**
 * @brief Print information on the static values of the target library
 */
void static_context_switcher_print_info(void);

#endif

/** @} */
