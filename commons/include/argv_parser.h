#ifndef ARGV_PARSER_H__
#define ARGV_PARSER_H__

//——————————————————————————————————————————————————————————————————————————————

#include <inttypes.h>

//——————————————————————————————————————————————————————————————————————————————

const char* const BackendDefaultOutputName = "program.out";
const char* const BackendDefaultInputName  = "frontend.txt";

//——————————————————————————————————————————————————————————————————————————————

enum ap_status_t
{
    AP_SUCCESS = 0,
    AP_ERROR   = 1,
};

//——————————————————————————————————————————————————————————————————————————————

struct ap_ctx_t
{
    bool        dump_source;
	bool        emit_obj;
    const char* source_name;
    const char* input_file;
    const char* output_file;
};

//——————————————————————————————————————————————————————————————————————————————

ap_status_t parse_argv(int argc, char *argv[], ap_ctx_t* ctx);

//——————————————————————————————————————————————————————————————————————————————

#endif // ARGV_PARSER_H__
