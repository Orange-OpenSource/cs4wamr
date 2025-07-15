/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#ifndef _WASM_SYSROOT_COMMON
#define _WASM_SYSROOT_COMMON

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WASM_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
}
#endif

#endif