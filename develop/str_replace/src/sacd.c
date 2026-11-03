#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static inline char *str_replace(const char *src, const char *from, const char *to)
{
    size_t size    = strlen(src) + 1;
    size_t fromlen = strlen(from);
    size_t tolen   = strlen(to);
    char *value = malloc(size);
    char *dst = value;
    if (value != NULL)
    {
        for ( ;; )
        {
            const char *match = strstr(src, from);
            if ( match != NULL )
            {
                size_t count = match - src;
                char *temp;
                size += tolen - fromlen;
                temp = realloc(value, size);
                if ( temp == NULL )
                {
                    free(value);
                    return NULL;
                }
                dst = temp + (dst - value);
                value = temp;
                memmove(dst, src, count);
                src += count;
                dst += count;
                memmove(dst, to, tolen);
                src += fromlen;
                dst += tolen;
            }
            else /* No match found. */
            {
                strcpy(dst, src);
                break;
            }
        }
    }
    return value;
}


char *str_replace_sacd(const char *src, const char *from, const char *to)
{
	return str_replace(src, from, to);
}
