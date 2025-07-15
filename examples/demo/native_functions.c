/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "wamr_env_thread.h"
#include "ztimer.h"
#include <stdio.h>
#include <wasm_export.h>

void call_f(void)
{
    printf("call_f was called\n");
}

void call_g(void)
{
    printf("call_g was called (the super secret function)\n");
}

void wait(wasm_exec_env_t exec_env, int i)
{
    // printf("wait %d\n", i);
    wamr_env_thread_sleep_time(ZTIMER_SEC, i);
}

void nothing(void)
{
}