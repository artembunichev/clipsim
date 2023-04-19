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

#include <X11/X.h>
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include "clip.h"
#include "clipsim.h"
#include "config.h"
#include "hist.h"
#include "util.h"
#include "send_signal.h"

static Display *DISPLAY;
static Window ROOT;
static Atom CLIPBOARD, PROPERTY, INCREMENT;
static Atom UTF8, IMG, TARGET;
static XEvent XEV;
static Window WINDOW;

typedef enum ClipResult {
    TEXT,
    LARGE,
    IMAGE,
    OTHER,
    ERROR,
} ClipResult;

static Atom get_target(Atom);
static ClipResult get_clipboard(char **, ulong *);
static bool valid_content(uchar *, ulong);
static void signal_program(void);

typedef union Signal {
    char *str;
    int num;
} Signal;

void signal_program(void) {
    Signal sig;
    char *program;
    if (!(sig.str = getenv("CLIPSIM_SIGNAL_CODE"))) {
        fprintf(stderr, "CLIPSIM_SIGNAL_CODE environment variable not set.\n");
        return;
    }
    if (!(program = getenv("CLIPSIM_SIGNAL_PROGRAM"))) {
        fprintf(stderr, "CLIPSIM_SIGNAL_PROGRAM environment variable not set.\n");
        return;
    }
    if ((sig.num = atoi(sig.str)) < 10) {
        fprintf(stderr, "Invalid CLIPSIM_SIGNAL_CODE environment variable.\n");
        return;
    }

    send_signal(program, sig.num);
}

void *daemon_watch_clip(void *unused) {
    ulong color;
    struct timespec pause;
    pause.tv_sec = 0;
    pause.tv_nsec = PAUSE10MS;
    (void) unused;

    if (!(DISPLAY = XOpenDisplay(NULL))) {
        fprintf(stderr, "Can't open X display.\n");
        exit(1);
    }

    ROOT = DefaultRootWindow(DISPLAY);
    CLIPBOARD = XInternAtom(DISPLAY, "CLIPBOARD", False);
    PROPERTY  = XInternAtom(DISPLAY, "XSEL_DATA", False);
    INCREMENT = XInternAtom(DISPLAY, "INCR", False);
    UTF8      = XInternAtom(DISPLAY, "UTF8_STRING", False);
    IMG       = XInternAtom(DISPLAY, "image/png", False);
    TARGET    = XInternAtom(DISPLAY, "TARGETS", False);

    color = BlackPixel(DISPLAY, DefaultScreen(DISPLAY));
    WINDOW = XCreateSimpleWindow(DISPLAY, DefaultRootWindow(DISPLAY),
                                 0,0, 1,1, 0, color, color);

    XFixesSelectSelectionInput(DISPLAY, ROOT, CLIPBOARD, (ulong)
                               XFixesSetSelectionOwnerNotifyMask
                             | XFixesSelectionClientCloseNotifyMask
                             | XFixesSelectionWindowDestroyNotifyMask);

    while (true) {
        char *save = NULL;
        ulong len;
        nanosleep(&pause , NULL);
        (void) XNextEvent(DISPLAY, &XEV);
        pthread_mutex_lock(&lock);

        signal_program();

        switch (get_clipboard(&save, &len)) {
            case TEXT:
                if (valid_content((uchar *) save, len))
                    hist_add(save, len);
                break;
            case IMAGE:
                fprintf(stderr, "Image copied to clipboard. "
                                "This won't be added to history.\n");
                break;
            case OTHER:
                fprintf(stderr, "Unsupported format. Clipsim only"
                                " works with UTF-8.\n");
                break;
            case LARGE:
                fprintf(stderr, "Buffer is too large and "
                                "INCR reading is not implemented yet. "
                                "This entry won't be saved to history.\n");
                break;
            case ERROR:
                hist_rec(0);
                break;
        }
        pthread_mutex_unlock(&lock);
    }
}

static Atom get_target(Atom target) {
    XEvent event;

    XConvertSelection(DISPLAY, CLIPBOARD, target, PROPERTY,
                      WINDOW, CurrentTime);
    do {
        (void) XNextEvent(DISPLAY, &event);
    } while (event.type != SelectionNotify
          || event.xselection.selection != CLIPBOARD);

    return event.xselection.property;
}

static ClipResult get_clipboard(char **save, ulong *len) {
    int actual_format_return;
    ulong nitems_return;
    ulong bytes_after_return;
    Atom return_atom;

    if (get_target(UTF8)) {
        XGetWindowProperty(DISPLAY, WINDOW, PROPERTY, 0, LONG_MAX/4,
                           False, AnyPropertyType, &return_atom,
                           &actual_format_return, &nitems_return, 
                           &bytes_after_return, (uchar **) save);
        if (return_atom == INCREMENT) {
            return LARGE;
        } else {
            *len = nitems_return;
            return TEXT;
        }
    } else if (get_target(IMG)) {
        return IMAGE;
    } else if (get_target(TARGET)) {
        return OTHER;
    }
    return ERROR;
}

static bool valid_content(uchar *data, ulong len) {
    static const uchar PNG[] = {0x89, 0x50, 0x4e, 0x47};

    { /* Check if it is made only of spaces and newlines */
        uchar *aux = data;
        do {
            aux++;
        } while ((*(aux-1) == ' ')
              || (*(aux-1) == '\t')
              || (*(aux-1) == '\n'));
        if (*(aux-1) == '\0') {
            fprintf(stderr, "Only white space copied to clipboard. "
                            "This won't be added to history.\n");
            return false;
        }
    }

    if (len <= 2) {
        if ((' ' <= *data) && (*data <= '~')) {
            if (len == 1 || (*(data+1) == '\n')) {
                fprintf(stderr, "Ignoring single character '%c'\n", *data);
                return false;
            }
        }
    }

    if (len >= 4) { /* check if it is an image */
        if (!memcmp(data, PNG, 4)) {
            fprintf(stderr, "Image copied to clipboard. "
                            "This won't be added to history.\n");
            return false;
        }
    }
    return true;
}
