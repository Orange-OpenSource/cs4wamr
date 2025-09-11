/*
 * Software Name : CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#include "multi_static.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t default_env_values[MAX_NUM_EXTERNAL_VALUES][MAX_STATIC_VALUE_SIZE];
multi_static_static_values_t static_values[MAX_NUM_EXTERNAL_VALUES] = EXTERNAL_VALUES_DEFAULT;
uint32_t static_values_count = MAX_NUM_EXTERNAL_VALUES;
static multi_static_env_t *current_env = NULL;

static void _multi_static_reset_static_values(void)
{
    if (static_values_count == MAX_NUM_EXTERNAL_VALUES && static_values[0].ptr == static_values[0].size &&
        static_values[0].ptr == 1)
    {
        printf("Multi-static not initialized. \nPlease use the elf reader to inject static variable location\n");
    }
    for (unsigned int i = 0; i < static_values_count; i++)
    {
        memcpy((void *)static_values[i].ptr, default_env_values[i], static_values[i].size);
    }
}

static void _multi_static_load_static_values(multi_static_env_t *env)
{
    for (unsigned int i = 0; i < static_values_count; i++)
    {
        memcpy((void *)static_values[i].ptr, env->static_values[i], env->static_values_size[i]);
    }
}

static void _multi_static_save_static_values(multi_static_env_t *env)
{
    for (unsigned int i = 0; i < static_values_count; i++)
    {
        memcpy(env->static_values[i], (void *)static_values[i].ptr, env->static_values_size[i]);
    }
}

void multi_static_save_default_static_values(void)
{
    for (unsigned int i = 0; i < static_values_count; i++)
    {
        memcpy(default_env_values[i], (void *)static_values[i].ptr, static_values[i].size);
    }
}

void multi_static_swap(multi_static_env_t *env)
{
    if (current_env == env)
    {
        return;
    }
    if (current_env != NULL)
    {
        _multi_static_save_static_values(current_env);
    }
    _multi_static_load_static_values(env);
    current_env = env;
}

void multi_static_reset_env(void)
{
    if (current_env != NULL)
    {
        _multi_static_save_static_values(current_env);
    }
    _multi_static_reset_static_values();
}

int multi_static_init_env(multi_static_env_t *env, void *(*malloc_func)(unsigned int))
{
    for (uint32_t i = 0; i < static_values_count; i++)
    {
        env->static_values[i] = malloc_func(static_values[i].size);
        if (env->static_values[i] == NULL)
        {
            return 1;
        }
        env->static_values_size[i] = static_values[i].size;
    }

    current_env = env;
    return 0;
}

void multi_static_print_env(multi_static_env_t *env)
{
    printf("multi_static_env: %p\n", env);
    for (uint32_t i = 0; i < static_values_count; i++)
    {
        printf("\tstatic value: %p, static value size: %lu\n", env->static_values[i], env->static_values_size[i]);
    }
}
void multi_static_print_info(void)
{
    printf("multi_static_env info:\n");
    printf("\tcurrent env %p, &current env %p, static_values_count %lu\n", current_env, &current_env,
           static_values_count);
    for (uint32_t i = 0; i < static_values_count; i++)
    {
        printf("\tstatic value: %p, static value size: %lu\n", (void *)static_values[i].ptr, static_values[i].size);
    }
}