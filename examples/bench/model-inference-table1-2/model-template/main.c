/*
 * Software Name: CS4WAMR
 * SPDX-FileCopyrightText: Copyright (c) Orange SA
 * SPDX-License-Identifier: MIT
 *
 * This software is distributed under the MIT licence,
 * see the "LICENSE" file for more details or https://opensource.org/license/mit
 *
 */
#include "mlmci.h"
#include <benchmark.h>
#include <stdio.h>
#include <string.h>
#include "input.h"
extern mlmodel_t *model_ptr;
int printf(const char *, ...);


void init(void)
{
    printf("Start\n");
    mlmodel_init(model_ptr);
}

uint8_t *get_input_address(void)
{
    int index = 0;
    mlmodel_iovar_t *input = mlmodel_get_input_variable(model_ptr, index);
    printf("Intern input value %p\n", input->values);
    return input->values;
}

unsigned int get_input_size(void)
{
    int index = 0;
    mlmodel_iovar_t *input = mlmodel_get_input_variable(model_ptr, index);
    return input->num_bytes;
}

uint8_t *get_output_address(void)
{
    int index = 0;
    mlmodel_iovar_t *output = mlmodel_get_output_variable(model_ptr, index);
    return output->values;
}

unsigned int get_output_size(void)
{
    int index = 0;
    mlmodel_iovar_t *output = mlmodel_get_output_variable(model_ptr, index);
    return output->num_bytes;
}

void run(void)
{
    mlmodel_inference(model_ptr);
}


void load_input(void)
{
    uint8_t *input = get_input_address();
    memcpy(input, input_tab, sizeof(input_tab));
}

void check_result(void)
{
    int8_t *output = (int8_t *)get_output_address();
    for (int i = 0; i < 12; i++)
    {
        printf("%d %d\n", i, output[i]);
    }
}

int main(void)
{

    ztimer_sleep(ZTIMER_SEC, 5);
    init();
    load_input();
    run();
    BENCHMARK_FUNC("run model", 20, run(););
    check_result();

    return 0;
}