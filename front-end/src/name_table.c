static int add_identifier(const char* name)
{
    for (size_t i = 0; i < yy_ctx->name_table.n_names; i++) {
        if (strcmp(yy_ctx->name_table.names[i].name, name) == 0)
            return i;
    }
    yy_ctx->name_table.names[yy_ctx->name_table.n_names].name = strdup(name);
    yy_ctx->name_table.names[yy_ctx->name_table.n_names].len = strlen(name);
    return yy_ctx->name_table.n_names++;
}

static int get_identifier_index(const char* name)
{
    for (size_t i = 0; i < yy_ctx->name_table.n_names; i++) {
        if (strcmp(yy_ctx->name_table.names[i].name, name) == 0)
            return i;
    }
    /* Not found – should not happen for variables used after declaration */
    return add_identifier(name);  /* or error */
}