/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include <wasm_export.h>

void call_f(void);

void call_g(void);

void wait(wasm_exec_env_t exec_env, int i);

void nothing(void);