/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "common.h"
#include "model_input.h"

int call_model(const char *input, unsigned int input_size, char *output, unsigned int output_size);
int printf(const char *, ...);

WASM_EXPORT void f(void)
{
    signed char model_output[12];
    printf("pointer address %d\n", (int)model_output);
    for (int i = 0; i < 3; i++)
    {
        printf("\nf: %d\n", i);
        int success = call_model(input, sizeof(input), model_output, sizeof(model_output));
        if (!success)
        {
            printf("\nf predicted value: failed\n");
            continue;
        }

        printf("\nf Expecting: \t\t-128\t-128\t-128\t-128\t-128\t-127\t-128\t-128\t118\t-128\t-128\t-119\n predicted "
               "value:\t");
        for (int i = 0; i < sizeof(model_output); i++)
        {
            printf("%d\t", model_output[i]);
        }
        printf("\n");
    }
}
