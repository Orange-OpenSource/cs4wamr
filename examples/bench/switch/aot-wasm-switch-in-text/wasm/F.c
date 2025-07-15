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
    printf("\npointer address %d\n", (int)model_output);
    for (int i = 0; i < 3; i++)
    {
        printf("\nf: %d\n", i);
        int success = call_model(input, sizeof(input), model_output, sizeof(model_output));
        if (!success)
        {
            printf("\npredicted value: failed\n");
            continue;
        }
        int maxValue = model_output[0];
        int maxI = 0;
        for (int i = 1; i < sizeof(model_output); i++)
        {
            if (model_output[i] > maxValue)
            {
                maxValue = model_output[i];
                maxI = i;
            }
        }
        printf("\npredicted value f: %d\n", maxI);
    }
}
