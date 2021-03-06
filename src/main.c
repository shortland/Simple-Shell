#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "batch_mode.h"
#include "command_return_list.h"
#include "debug.h"
#include "executor.h"
#include "globals.h"
#include "interactive_mode.h"
#include "io.h"
#include "parse_command.h"
#include "parse_path.h"
#include "readline.h"
#include "signals.h"
#include "string_list.h"

int main(int argc, char *argv[], char *envp[]) {
    /** Determine whether batchmode was initialized. */
    char *filename = NULL;
    int d = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], DEBUG_FLAG) == 0) {
            d = 0;

            debug_enable();
            debug("debugging output enabled\n");

            if (filename != NULL) {
                /** already parsed anything it can - begin to execute file line by line */
                break;
            }

            continue;
        }

        if (strcmp(argv[i], "-v") == 0) {
            fprintf(stdout, "version: %s\n", SMASH_VERSION);
            return 0;
        }

        if (access(argv[i], F_OK) != -1) {
            debug("attempting to execute shell file.\n");

            if (access(argv[i], X_OK) == 0) {
                debug("can execute the file contents\n");
                filename = argv[i];

                if (d == 0) {
                    /** already parsed anything it can - begin to execute file line by line. */
                    break;
                }
            } else {
                fprintf(stderr, "smash: error - the file '%s' is not executable; tip: $ chmod +x %s\n", argv[i], argv[i]);
                return 1;
            }
        } else {
            fprintf(stderr, "smash: error - the file '%s' does not exist\n", argv[i]);
            return 1;
        }
    }

    /**
     * Parses out all the environment variables for later ease of retrieval using parse_path_get_env()
     */
    char *path_string = NULL;
    char **env_list = NULL;

    if ((env_list = parse_path_all_env_params(envp)) == NULL) {
        fprintf(stderr, "error: unable to parse path of the env variables");
        return 1;
    }

    if ((path_string = parse_path_get_env(ENV_PATH_KEY)) == NULL) {
        fprintf(stderr, "error: unable to get path string. Is there one? - Check globals.h to configure.\n");
        return 1;
    }

    /**
     * Parse directories that have binaries
     */
    string_list *bin_list = NULL;

    if ((bin_list = parse_path_bin_dirs(path_string)) == NULL) {
        fprintf(stderr, "error: unable to parse path of bin dirs");
        return 1;
    }

    /**
     * Initialize the jobs list
     * -> Initializes a list which contains currently running job info.
     * 
     * Must call before hooking readline.
     */
    if (executor_init_execd() != 0) {
        fprintf(stderr, "error: unable to initialize executor execd list");
        return 1;
    }

    /**
     * Set sig hook function in readline
     */
    struct sigaction sc;
    sc.sa_handler = signals_job_done;
    sc.sa_flags = 0;

    sigemptyset(&sc.sa_mask);

    if (sigaction(SIGCHLD, &sc, NULL) == -1) {
        fprintf(stderr, "error: unable to set SIGCHLD handler for finding finished jobs\n");
        return 1;
    }

    readline_set_sig_hook(signals_readline_operator);

    /**
     * Run interactive mode if no file was given as arg.
     */
    if (filename == NULL) {
        debug("interactive mode enabled\n");
        pointer_pointer_debug(env_list, -1);

        return interactive_mode_run(argc, argv, bin_list, env_list);
    }

    debug("batch (non-interactive) mode enabled\n");

    return batch_mode_run(filename, bin_list, env_list);
}
