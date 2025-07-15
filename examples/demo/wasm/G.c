/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "G.h"

extern int printf(const char *, ...);
extern void call();
extern void nothing();
__attribute__((import_module("H"), import_name("h"))) extern void h();
extern void wait(int i);

WASM_EXPORT void g(void)
{
    int i = 0;
    while (1)
    {
        for (int j = 0; j < 9; j++)
        {
            printf("\ng %d\n", i++);
            h();
            call();
            wait(2);
        }
        printf("\n=> g %d big wait\n", i++);
        for (int j = 0; j < 400000; j++)
        {
            nothing();
        }
        printf("\n=! g end big wait\n");
    }
}
