/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */

#include_next "string.h"

#ifndef _COMPATIBILITY_CODE_STRING_H
#define _COMPATIBILITY_CODE_STRING_H

void *memcpy(void *destination, const void *source, size_t size);
int strcmp(const char *first, const char *second);

#endif