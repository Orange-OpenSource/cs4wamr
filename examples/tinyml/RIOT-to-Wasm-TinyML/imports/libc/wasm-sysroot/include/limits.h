/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#ifndef _WASM_SYSROOT_LIMITS_H
#define _WASM_SYSROOT_LIMITS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CHAR_BIT 8
#define SCHAR_MIN -128
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define CHAR_MIN -128
#define CHAR_MAX 127
#define MB_LEN_MAX 1
#define SHRT_MIN -32768
#define SHRT_MAX 32767
#define USHRT_MAX 65535
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#define UINT_MAX 4294967295
#define LONG_MIN -9223372036854775808
#define LONG_MAX 9223372036854775807
#define ULONG_MAX 18446744073709551615

#ifdef __cplusplus
}
#endif

#endif