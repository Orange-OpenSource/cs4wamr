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

float roundf(float x)
{
    if (x < 0.0)
        return (float)(int)(x - 0.5);
    else
        return (float)(int)(x + 0.5);
}

// Simplified implementation of expf using series
float expf(float x)
{
    float sum = 1.0f;
    float term = 1.0f;
    int n = 1;

    if (x == 0.0f)
        return 1.0f;

    if (x < 0.0f)
        return 1.0f / expf(-x);

    while (n < 500)
    {
        term *= x / n;
        sum += term;
        n++;
    }
    return sum;
}

#define PI 3.14159265358979323846264338327950288419716939937510

float sinf(float x)
{
    float term = x;
    float sum = term;
    for (int n = 1; n < 500; n++)
    {
        term *= -x * x / (2 * n * (2 * n + 1));
        sum += term;
    }
    return sum;
}

float cosf(float x)
{
    float term = 1;
    float sum = term;
    for (int n = 1; n < 500; n++)
    {
        term *= -x * x / (2 * n * (2 * n - 1));
        sum += term;
    }
    return sum;
}

float tanf_custom(float x)
{
    return sinf(x) / cosf(x);
}

// Simplified implementation using Taylor series

float tanhf(float x)
{
    return tanf_custom(x);
}