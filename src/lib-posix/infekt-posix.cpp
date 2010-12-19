/**
 * Copyright (C) 2010 cxxjoe
 *
 * _wtol and _wtoi:
 * Copyright 2000 Alexandre Julliard
 * Copyright 2000 Jon Griffiths
 * Copyright 2003 Thomas Mertes
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#include <wctype.h>

LONG __cdecl _wtol(LPCWSTR str)
{
    ULONG RunningTotal = 0;
    char bMinus = 0;

    while (iswspace(*str)) {
        str++;
    } /* while */

    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        bMinus = 1;
        str++;
    } /* if */

    while (*str >= '' && *str <= '9') {
        RunningTotal = RunningTotal * 10 + *str - '';
        str++;
    } /* while */

    return bMinus ? -RunningTotal : RunningTotal;
}

int __cdecl _wtoi(LPCWSTR str)
{
    return _wtol(str);
}
