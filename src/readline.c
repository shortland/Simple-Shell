#include "readline.h"

static readline_sig_hook *readline_hook;

void readline_set_sig_hook(readline_sig_hook hook) {
    readline_hook = hook;
    return;
}

char *readline(char *prompt, int fd) {
    int size = 64;
    char *buf = NULL;

    if ((buf = malloc(size)) == NULL) {
        fprintf(stderr, "error: unable to malloc space for readline buffer\n");
        return NULL;
    }

    char c;
    char *bp = buf;

    fprintf(stdout, "%s", prompt);
    fflush(stdout);

    while (1) {
        /**
         * Mask signal bits
         */
        sigset_t mask, out_mask;
        sigfillset(&mask);
        sigprocmask(SIG_BLOCK, &mask, &out_mask);

        /**
         * Call the hook
         */
        if (readline_hook != NULL) {
            readline_hook();
        }

        /**
         * Release signals
         */
        fd_set fd_in;
        FD_ZERO(&fd_in);
        FD_SET(fd, &fd_in);

        int ready_desc;
        int check_err;

        if (fd == fileno(stdin)) {
            ready_desc = pselect(1, &fd_in, NULL, NULL, NULL, &out_mask);
        } else {
            ready_desc = 1;
        }

        check_err = errno;

        /** unmask */
        sigprocmask(SIG_SETMASK, &out_mask, NULL);

        /**
         * Check if there are no ready file descriptors; s.t. read would probably fail.
         */
        if (ready_desc < 0) {
            if (check_err == EINTR) {
                continue;
            }

            return NULL;
        } else if (ready_desc == 0) {
            if (bp - buf > 0) {
                break;
            } else {
                free(buf);
                return NULL;
            }
        }

        /**
         * Read in 1 byte at a time.
         */
        if (read(fd, &c, 1) != 1) {
            if (bp - buf > 0) {
                break;
            } else {
                free(buf);
                return NULL;
            }
        }

        /**
         * User pressed enter.
         */
        if (c == '\n') {
            break;
        }

        if (bp - buf >= size - 1) {
            size <<= 1;
            char *new_buf = realloc(buf, size);

            if (new_buf == NULL) {
                fprintf(stderr, "error: unable to realloc for input buffer\n");
                break;
            } else {
                bp = new_buf + (bp - buf);
                buf = new_buf;
            }
        }

        *bp++ = c;
    }

    *bp = '\0';

    return buf;
}
