#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "argv_parser.h"
#include "custom_assert.h"

//——————————————————————————————————————————————————————————————————————————————

inline void print_usage()
{
    fputs("Usage: ./ [options]\n"
          "Options:\n"
          "  --help, -h                 Print this message.\n"
          "  --Source=<filename>, -S    Set assembler dump file name.\n"
          "  --emit-obj                 Emit .splobj instead of executable ELF.\n"
          "  --convert-splobj                 Link one input .splobj into executable ELF.\n"
          "  --out=<filename>, -o       Set output file name.\n", stderr);
}

//——————————————————————————————————————————————————————————————————————————————

ap_status_t parse_argv(int argc, char *argv[], ap_ctx_t* ctx)
{
    int opt    = 0;
    int optind = 0;

    option l_opts[] = {
        {"help",           no_argument,       NULL, 'h'},
        {"Source",         required_argument, NULL, 'S'},
        {"input",          required_argument, NULL, 'i'},
        {"out",            required_argument, NULL, 'o'},
        {"emit-obj",       no_argument,       NULL, 'c'},
        {"convert-splobj", no_argument,       NULL, 'l'},
        {},
    };

    const char* s_opts = "hS:i:o:";

    while ((opt = getopt_long(argc, argv, s_opts, l_opts, &optind)) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                break;
            case 'i':
                ctx->input_file  = optarg;
                break;
            case 'o':
                ctx->output_file = optarg;
                break;
            case 'S':
                ctx->dump_source = true;
                ctx->source_name = optarg;
                break;
            case 'c':
                ctx->emit_obj = true;
                break;
            case 'l':
                ctx->link_obj = true;
                break;
            case '?':
                print_usage();
                return AP_ERROR;
            default:
                print_usage();
                return AP_ERROR;
        }
    }

    if (!ctx->output_file) {
        ctx->output_file = BackendDefaultOutputName;
    }

    if (!ctx->input_file) {
        ctx->input_file = BackendDefaultInputName;
    }

    return AP_SUCCESS;
}
