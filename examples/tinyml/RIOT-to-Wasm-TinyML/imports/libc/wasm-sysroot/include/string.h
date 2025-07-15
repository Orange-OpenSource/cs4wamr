/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#ifndef _WASM_SYSROOT_STRING_H
#define _WASM_SYSROOT_STRING_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned long size_t;
    void *memcpy(void *destination, const void *source, size_t size);
    int strcmp(const char *first, const char *second);

#ifdef __cplusplus
}
#endif

#endif