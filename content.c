/* This file is part of clipsim. */
/* Copyright (C) 2022-2023 Lucas Mior */

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
#include <magic.h>

void content_remove_newline(char *text, size_t *length) {
    DEBUG_PRINT("%s, %zu", text, *length);
    text[*length] = '\0';
    while (text[*length-1] == '\n') {
        text[*length-1] = '\0';
        *length -= 1;
    }
    return;
}

void content_trim_spaces(char **trimmed, size_t *trimmed_length,
                         char *content, const size_t length) {
    DEBUG_PRINT("%p, %p, %s, %zu",
                (void *) trimmed, (void *) trimmed_length, content, length);
    char *p;
    char temp = '\0';
    char *c = content;

    *trimmed = p = util_malloc(MIN(length+1, TRIMMED_SIZE+1));

    if (length >= TRIMMED_SIZE) {
        temp = content[TRIMMED_SIZE];
        content[TRIMMED_SIZE] = '\0';
    }

    while (IS_SPACE(*c))
        c++;
    while (*c != '\0') {
        while (IS_SPACE(*c) && IS_SPACE(*(c+1)))
            c++;

        *p++ = *c++;
    }
    *p = '\0';
    *trimmed_length = (size_t) (p - *trimmed);

    if (temp) {
        content[TRIMMED_SIZE] = temp;
        temp = '\0';
    }

    if (*trimmed_length == length) {
        free(*trimmed);
        *trimmed = content;
    } else {
        *trimmed = util_realloc(*trimmed, *trimmed_length+1);
    }
    return;
}

int32 content_check_content(uchar *data, const size_t length) {
    DEBUG_PRINT("%s, %zu", data, length);

    { /* Check if it is made only of spaces and newlines */
        uchar *aux = data;
        do {
            aux++;
        } while (IS_SPACE(*(aux-1)));
        if (*(aux-1) == '\0') {
            fprintf(stderr, "Only white space copied to clipboard. "
                            "This won't be added to history.\n");
            return CLIPBOARD_ERROR;
        }
    }

    if (length <= 2) { /* Check if it is a single ascii character
                       possibly followed by new line */
        if ((' ' <= *data) && (*data <= '~')) {
            if (length == 1 || (*(data+1) == '\n')) {
                fprintf(stderr, "Ignoring single character '%c'\n", *data);
                return CLIPBOARD_ERROR;
            }
        }
    }

    do {
        magic_t magic;
        const char *mime_type;
        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            break;
        }
        if (magic_load(magic, NULL) != 0) {
            magic_close(magic);
            break;
        }
        if ((mime_type = magic_buffer(magic, data, length)) == NULL) {
            magic_close(magic);
            break;
        }
        if (!strncmp(mime_type, "image/", 6)) {
            magic_close(magic);
            return CLIPBOARD_IMAGE;
        }
        magic_close(magic);
    } while (0);

    if (length > ENTRY_MAX_LENGTH) {
        fprintf(stderr, "Too large entry. This wont' be added to history.\n");
        return CLIPBOARD_ERROR;
    }

    return CLIPBOARD_TEXT;
}
