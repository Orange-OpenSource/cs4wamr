/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#ifndef _WASM_SYSROOT_STDIO_H
#define _WASM_SYSROOT_STDIO_H

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

    typedef unsigned long size_t;

    int printf(const char *, ...);

#ifdef __cplusplus
}
#endif

#endif