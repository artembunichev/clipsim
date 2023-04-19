/* This file is part of clipsim. */
/* Copyright (C) 2022 Lucas Mior */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#ifndef CLIPSIM_H
#define CLIPSIM_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#ifdef CLIPSIM_DEBUG
#define DEBUG_PRINT(s, ...) printf(s, __VA_ARGS__);
#else
#define DEBUG_PRINT(s, ...) do { } while(0);
#endif

#define NOFD 1000
#define PAUSE10MS (1000 * 1000 * 10)
#define OUT_BUF 8192
#define DEF_ALLOC 32
#define HIST_SIZE 300U
#define HIST_KEEP 150U

typedef enum Command {
    PRINT = 'p',
    INFO = 'i',
    COPY = 'c',
    DELETE = 'd',
    SAVE = 's',
} Command;

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;

typedef struct Entry Entry;
struct Entry {
    size_t len;
    size_t olen;
    char *data;
    char *out;
};

typedef struct Fifo {
    int fd;
    FILE *file;
    const char *name;
} Fifo;

extern char *progname;
extern Entry entries[HIST_SIZE];
extern int32 lastindex;
extern pthread_mutex_t lock;

#endif /* CLIPSIM_H */
