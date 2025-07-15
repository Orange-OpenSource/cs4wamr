/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#ifndef F_H
#define F_H

#define WASM_EXPORT __attribute__((visibility("default")))

WASM_EXPORT void f(void);

#endif /* F_H */
