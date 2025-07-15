/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#ifndef MULTI_STATIC_HEADER
#define MULTI_STATIC_HEADER

#include <stdint.h>

#ifndef MAX_STATIC_VALUE_SIZE
#define MAX_STATIC_VALUE_SIZE 128
#endif
#define MAX_NUM_EXTERNAL_VALUES 4
#define EXTERNAL_VALUES_DEFAULT                                                                                        \
    {                                                                                                                  \
        {                                                                                                              \
            1, 1                                                                                                       \
        }                                                                                                              \
    } // Prevent value from being in bss

/**
 * Environment structure for the variable on the state of an instance of the target library
 */
typedef struct multi_static_env_s
{
    uint8_t *static_values[MAX_NUM_EXTERNAL_VALUES];
    uint32_t static_values_size[MAX_NUM_EXTERNAL_VALUES]; // Can possibly be remove
} multi_static_env_t;

/**
 * @brief Structure containing the address and the size of some static values
 */
typedef struct multi_static_external_values_s
{
    uint32_t ptr;
    uint32_t size;
} multi_static_static_values_t;

/**
 * @brief Function to save the default state of the target library. The function must be run before using the
 * multi-static library
 */
void multi_static_save_default_static_values(void);

/**
 * @brief Reset the target library to its default state
 */
void multi_static_reset_env(void);

/**
 * @brief Swap the environment of the target library to the given environment
 * @param[in] env Pointer to the structure of the environment to swap to
 */
void multi_static_swap(multi_static_env_t *env);

/**
 * @brief Initialize a new environment for the target library. The environment structure is used by the swapping
 * mechanism.
 * @param[out] env Structure of the newly created environment.
 * @param[in] malloc_func Malloc function used to initialize the environment
 * @return 0 in case of success, 1 elsewhere
 */
int multi_static_init_env(multi_static_env_t *env, void *(*malloc_func)(unsigned int));

/**
 * @brief Print pointers information of the given environment. It notably prints what are the saved buffer from the
 * target library.
 * @param[in] env Env to print information from
 */
void multi_static_print_env(multi_static_env_t *env);

/**
 * @brief Print information on the static values of the target library
 */
void multi_static_print_info(void);

#endif