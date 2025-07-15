/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "F.h"

extern int printf(const char *, ...);
extern void call();
extern void nothing();
extern void wait(int i);

WASM_EXPORT void f(void)
{
    int i = 0;
    while (1)
    {
        for (int j = 0; j < 5; j++)
        {
            printf("\nf %d\n", i++);
            call();
            wait(3);
        }
        printf("\n=> f %d big wait\n", i++);
        for (int j = 0; j < 400000; j++)
        {
            nothing();
        }
        printf("\n=! f end big wait\n");
    }
}
