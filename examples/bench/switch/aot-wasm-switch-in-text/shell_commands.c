/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "shell.h"
#include "wamr_env_thread.h"
#include <stdint.h>
#include <stdlib.h>

#define MAX_WAMR_ENV_BUFFER_SIZE 0

/*wamr_env_thread_t saved_env_thread;
wamr_env_thread_number_t saved_env_number = -1;
char saving_buffer[MAX_WAMR_ENV_BUFFER_SIZE];
*/

wamr_env_thread_number_t _args_to_env(int argc, char **argv)
{
    wamr_env_thread_number_t env = 0;
    if (argc == 2)
    {
        env = strtol(argv[1], NULL, 10);
    }
    return env;
}

int make_thread_sleep(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    wamr_env_thread_pause(env);
    printf("sleeping env %ld\n", env);
    return 0;
}

int make_thread_resume(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    wamr_env_thread_resume(env);
    printf("resuming env  %ld\n", env);
    return 0;
}

/*
int save_env(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    wamr_env_thread_save(env, &saved_env_thread, saving_buffer, sizeof(saving_buffer));
    saved_env_number = env;
    printf("saving env  %ld\n", env);
    return 0;
}

int load_env(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    if (saved_env_number != env)
    {
        printf("Env slot do not match for loading !\n");
        return 1;
    }
    wamr_env_thread_restored(env, &saved_env_thread, saving_buffer, sizeof(saving_buffer));
    printf("loading env  %ld\n", env);
    return 0;
}

int fast_save(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    wamr_env_thread_pause(env);
    wamr_env_thread_save(env, &saved_env_thread, saving_buffer, sizeof(saving_buffer));
    wamr_env_thread_resume(env);
    saved_env_number = env;
    printf("fast saving env  %ld\n", env);
    return 0;
}

int fast_load(int argc, char **argv)
{
    wamr_env_thread_number_t env = _args_to_env(argc, argv);
    if (saved_env_number != env)
    {
        printf("Env slot do not match for loading !\n");
        return 1;
    }
    wamr_env_thread_restored(env, &saved_env_thread, saving_buffer, sizeof(saving_buffer));
    wamr_env_thread_resume(env);
    printf("fast loading env  %ld\n", env);
    return 0;
}
*/

void wamr_env_prints_stats(void)
{
    mem_alloc_info_t mem_alloc_info;
    for (int i = 0; i < ENV_AMOUNT; i++)
    {
        if (!wamr_env_thread_is_env_initialized(i))
        {
            printf("env: %d, uninitialized\n", i);
            continue;
        }
        wamr_env_thread_swap(i);
        wamr_env_thread_print_info();
        wasm_runtime_get_mem_alloc_info(&mem_alloc_info);
        kernel_pid_t pid = wamr_env_thread_get_pid(i);
        wamr_env_thread_print(i);
        printf("env: %d, pid: %d, highmark_size: %ld, total_free_size  %ld, total_size %ld\n\n\n", i, pid,
               mem_alloc_info.highmark_size, mem_alloc_info.total_free_size, mem_alloc_info.total_size);
    }
}

int wamr_buffer_stat(int argc, char **argv)
{
    wamr_env_prints_stats();
    return 0;
}

SHELL_COMMAND(pause, "Make env X sleep", make_thread_sleep);
SHELL_COMMAND(resume, "Make env X resume", make_thread_resume);
/*SHELL_COMMAND(save, "Make env X save", save_env);
SHELL_COMMAND(fast_save, "Make env X pause, save and resume", fast_save);
SHELL_COMMAND(load, "Make env X save", load_env);
SHELL_COMMAND(fast_load, "Make env X load and resume", fast_load);*/
SHELL_COMMAND(stats, "Get stats on buffer", wamr_buffer_stat);