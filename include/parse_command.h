#ifndef PARSE_COMMAND_H
#define PARSE_COMMAND_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "globals.h"
#include "parse_path.h"
#include "pointer_pointer_helper.h"
#include "string_list.h"

static int jobs;

typedef struct commander {
    int job_id;

    int bgfg;  // -1 = is foreground job, 1 is background job.

    pid_t pid;      // should only have non-zero pid if it's been started
    int running;    // 1 = running, -1 = not running.
    int exit_code;  // exit code of this command after it finished running

    time_t started;   // timestamp when started
    time_t finished;  // timestamp when finished

    string_list *raw_command;

    char *bin_dir;                // e.g.) '/usr/local/bin'
    char *bin;                    // e.g.) 'echo'
    char **bin_params;            // e.g.) '-s -x'
    int num_bin_params;           // e.g.) '2' (need to keep track of size of param array above)
    char *output_redirect;        // e.g.) '>someOutput'
    char *output_error_redirect;  // e.g.) '2>somefile'
    char *input_redirect;         // e.g.) '<someInputFile'
} commander;

string_list *parse_command_to_string_list(char *command);

commander *parse_command_from_string_list(string_list *command);

void parse_command_debug_commander(commander *cmd);

#endif
