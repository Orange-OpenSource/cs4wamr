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

int get_max_float(float f1, float f2, float f3)
{
    if (f2 < f1 && f3 < f1)
    {
        return 1;
    }
    if (f1 < f2 && f3 < f2)
    {
        return 2;
    }
    return 3;
}

WASM_EXPORT void f(void)
{
    char model_output[12];
    float *model_output_value = (float *)model_output;
    printf("pointer address %d\n", (int)model_output);
    for (int i = 0; i < 3; i++)
    {
        printf("\nf: %d\n", i);
        int success = call_model(test_list[(i + 1) % 3], 450, model_output, sizeof(model_output));
        if (!success)
        {
            printf("\npredicted value: failed\n");
            continue;
        }
        printf("\npredicted value: %d\n",
               get_max_float(model_output_value[0], model_output_value[1], model_output_value[2]));
    }
}
