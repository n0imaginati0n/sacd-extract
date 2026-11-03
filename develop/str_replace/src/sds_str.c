#ifndef _SADC_H
#define _SADC_H

#include <string.h>
#include <stdlib.h>

#include "sds.h"

#define SIMPLE

static inline char* str_replace(const char* src, const char* from, const char* to)
{
    // according to the old function code, we believe, that all input parameters
    // are not NULL pointers, and point to null-terminated strings. otherwise
    // strlen will lead to UB
    if (src == NULL || src[0] == '\0' || // no reason to replace something in NULL or empty string
        from == NULL || from[0] == '\0') // no reason to seek for NULL or empty string
    {
        return NULL;
    }

    if (to == NULL)
    {   // if 'to' is empty, function will just remove pattern 'from'
        to = "";
    }

    int tokens_num = 0;
    sds* tokens = sdssplitlen(src, strlen(src), from, strlen(from), &tokens_num);
    if (tokens == NULL || tokens_num == 0)
    {
        return NULL;
    }

    sds merged = sdsjoinsds(tokens, tokens_num, to, strlen(to));

    sdsfreesplitres(tokens, tokens_num);

    char* result = calloc(1, sdslen(merged) + 1);
    memcpy(result, merged, sdslen(merged));
    sdsfree(merged);

    return result;
}

char *str_replace_sds(const char *src, const char *from, const char *to)
{
    return str_replace(src, from, to);
}

#endif

