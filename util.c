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

#include "clipsim.h"
#include <stdarg.h>

void *util_malloc(const size_t size) {
    void *p;
    if ((p = malloc(size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *util_realloc(void *old, const size_t size) {
    void *p;
    if ((p = realloc(old, size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", size);
        fprintf(stderr, "Reallocating from: %p\n", old);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *util_calloc(const size_t nmemb, const size_t size) {
    void *p;
    if ((p = calloc(nmemb, size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu members of %zu bytes each.\n",
                        nmemb, size);
        exit(EXIT_FAILURE);
    }
    return p;
}

int util_string_int32(int32 *number, const char *string) {
    char *endptr;
    long x;
    errno = 0;
    x = strtol(string, &endptr, 10);
    if ((errno != 0) || (string == endptr) || (*endptr != 0)) {
        return -1;
    } else if ((x > INT32_MAX) || (x < INT32_MIN)) {
        return -1;
    } else {
        *number = (int32) x;
        return 0;
    }
}

void util_die_notify(const char *format, ...) {
    char *notifiers[2] = { "dunstify", "notify-send" };
    int n;
	va_list args;
    char buffer[BUFSIZ];

	va_start(args, format);
	n = vsnprintf(buffer, sizeof (buffer), format, args);
	va_end(args);
    buffer[n] = '\0';

    write(STDERR_FILENO, buffer, n+1);
    for (uint i = 0; i < ARRAY_LENGTH(notifiers); i += 1) {
        execlp(notifiers[i], notifiers[i], "-u", "critical", 
               "clipsim", buffer, NULL);
    }
    exit(EXIT_FAILURE);
}

void util_segv_handler(int unused) {
    (void) unused;
    char *message = "Memory error. Please send a bug report.\n";
    char *notifiers[2] = { "dunstify", "notify-send" };

    write(STDERR_FILENO, message, strlen(message));
    for (uint i = 0; i < ARRAY_LENGTH(notifiers); i += 1) {
        execlp(notifiers[i], notifiers[i], "-u", "critical", 
               "clipsim", message, NULL);
    }
    exit(EXIT_FAILURE);
}

void util_close(File *file) {
    if (file->fd >= 0) {
        if (close(file->fd) < 0) {
            fprintf(stderr, "Error closing %s: %s\n",
                            file->name, strerror(errno));
        }
        file->fd = -1;
    }
    if (file->file != NULL) {
        if (fclose(file->file) != 0) {
            fprintf(stderr, "Error closing %s: %s\n",
                            file->name, strerror(errno));
        }
        file->file = NULL;
    }
    return;
}

int util_open(File *file, const int flag) {
    if ((file->fd = open(file->name, flag)) < 0) {
        fprintf(stderr, "Error opening %s: %s\n",
                        file->name, strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

int util_copy_file(const char *destination, const char *source) {
    int source_fd, destination_fd;
    char buffer[BUFSIZ];
    ssize_t r = 0;
    ssize_t w = 0;
    
    if ((source_fd = open(source, O_RDONLY)) < 0) {
        fprintf(stderr, "Error opening %s for reading: %s\n", 
                        source, strerror(errno));
        return -1;
    }

    if ((destination_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 
                                            S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stderr, "Error opening %s for writing: %s\n",
                         destination, strerror(errno));
        close(source_fd);
        return -1;
    }

    errno = 0;
    while ((r = read(source_fd, buffer, BUFSIZ)) > 0) {
        w = write(destination_fd, buffer, (size_t) r);
        if (w != r) {
            fprintf(stderr, "Error writing data to %s", destination);
            if (errno)
                fprintf(stderr, ": %s", strerror(errno));
            fprintf(stderr, ".\n");
            close(source_fd);
            close(destination_fd);
            return -1;
        }
    }

    if (r < 0) {
        fprintf(stderr, "Error reading data from %s: %s\n",
                        source, strerror(errno));
        close(source_fd);
        close(destination_fd);
        return -1;
    }

    close(source_fd);
    close(destination_fd);
    return 0;
}
