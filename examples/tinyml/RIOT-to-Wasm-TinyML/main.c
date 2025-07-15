/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include <stdio.h>
#include <string.h>

#include "mlmci.h"

extern mlmodel_t *model_ptr;

WASM_EXPORT void init(void)
{
    printf("Start\n");
    mlmodel_init(model_ptr);
}

WASM_EXPORT uint8_t *get_input_address()
{
    int index = 0;
    mlmodel_iovar_t *input = mlmodel_get_input_variable(model_ptr, index);
    printf("Intern input value %d\n", input->values);
    return input->values;
}

WASM_EXPORT unsigned int get_input_size()
{
    int index = 0;
    mlmodel_iovar_t *input = mlmodel_get_input_variable(model_ptr, index);
    return input->num_bytes;
}

WASM_EXPORT uint8_t *get_output_address()
{
    int index = 0;
    mlmodel_iovar_t *output = mlmodel_get_output_variable(model_ptr, index);
    return output->values;
}

WASM_EXPORT unsigned int get_output_size()
{
    int index = 0;
    mlmodel_iovar_t *output = mlmodel_get_output_variable(model_ptr, index);
    return output->num_bytes;
}

WASM_EXPORT void run(void)
{
    mlmodel_inference(model_ptr);
}
