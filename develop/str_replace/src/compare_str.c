#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sds.h"
#include "sds_str.h"
#include "sacd.h"

#ifdef _MSC_VER
# include <intrin.h>
#else
# include <x86intrin.h>
#endif

// optional wrapper if you don't want to just use __rdtsc() everywhere
inline
uint64_t readTSC() {
    // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
    uint64_t tsc = __rdtsc();
    // _mm_lfence();  // optionally block later instructions until rdtsc retires
    return tsc;
}

// requires a Nehalem or newer CPU.  Not Core2 or earlier.  IDK when AMD added it.
inline
uint64_t readTSCp() {
    unsigned dummy;
    uint64_t tsc = __rdtscp(&dummy); // waits for earlier insns to retire
    // _mm_lfence();  // optionally block later instructions until rdtscp retires
    // in practice most CPUs seem to implement rdtscp similar to lfence; rdtsc, so later instructions won't start while waiting for earlier to finish.  But not guaranteed on paper.
    // see also the section below about out of order exec.
    return tsc;  
}

#define LEN(x)  (sizeof(x)/sizeof((x)[0]))

static const char char_s[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int char_s_len = LEN(char_s) - 1;

void fill_array(char* dest, size_t length, const char* pattern)
{
    int char_index, apply_pattern_idx = strlen(char_s) + 1;
    size_t pattern_length = strlen(pattern);

    for (int i = 0, ci = 0; i < length - 1; ++i) {
        dest[i] = char_s[ci++];
        if (ci >= char_s_len) {
            ci = 0;
        }
    }
    dest[length - 1] = '\0';

    for (int i = 4; i < length - pattern_length - 1; i += 2 * pattern_length) 
    {
        memcpy(dest + i, pattern, pattern_length);
    }
}


char *str_replace_custom(const char *src, const char *from, const char *to)
{
    if (!src || !from || src[0] == '\0' || from[0] == '\0')
        return NULL;

    if (!to) {
        to = "";
    }

    typedef struct {
        const char* begin;
        const char* end;
    } Token;

    size_t len_src  = strlen(src);
    size_t len_from = strlen(from);
    size_t len_to   = strlen(to);
    const char* end = src + len_src;

    Token* tokens = (Token*)malloc((1 + len_src/len_from) * sizeof(Token));
    if (!tokens) {
        return NULL;
    }
    Token* tok_it = tokens;

    tok_it->begin = src;
    tok_it->end = end;

    const char *found = strstr(tok_it->begin, from);
    while (found) {
        tok_it++->end = found;
        
        tok_it->begin = found + len_from;
        tok_it->end = end;
        
        found = strstr(tok_it->begin, from);
    };

    size_t found_tokens = tok_it - tokens + 1;
    size_t len_res = len_src + (found_tokens - 1) * (len_to - len_from);
    char* result = (char*)malloc(sizeof(char) * (len_res + 1));
    if (!result) {
        free(tokens);
        return NULL;
    }

    char* it = result;
    Token* last_token = tok_it;
    
    for (tok_it = tokens; tok_it <= last_token - 1; ++tok_it)
    {
        memcpy(it, tok_it->begin, sizeof(char)*(tok_it->end - tok_it->begin));
        it += tok_it->end - tok_it->begin;
        memcpy(it, to, len_to);
        it += len_to;
    }
    memcpy(it, tok_it->begin, sizeof(char)*(tok_it->end - tok_it->begin));
    it[tok_it->end - tok_it->begin] = '\0';

    free(tokens);
    return result;
}

#define max(a,b)    (((a) < (b)) ? (b) : (a))

int main(int argc, char** argv)
{
    (void*) &argc;
    (void*) &argv;

    const char from[] = "fromo";
    const uint8_t from_len = 5;

    const char to[] = "";
    const uint8_t to_len = 0;

    const uint64_t text_len = 1024 * 1024;
    char *text = (char*)malloc(text_len *sizeof(char));
    char *text_temp = (char*)malloc(text_len * sizeof(char));

    // fill
    fill_array(text, text_len, from);

    char* sacd_res = NULL, *sds_res = NULL, *cust_res = NULL;
    uint64_t start, end;

    uint16_t times = 256;

    uint64_t sacd_time = 0, sds_time = 0, cust_time = 0;
    for (uint16_t t = 0; t < times; ++t)
    {
        free(sds_res);
        free(sacd_res);
        free(cust_res);

        memcpy(text_temp, text, text_len * sizeof(char));
        start = readTSC();
        sacd_res = str_replace_sacd(text_temp, from, to);
        end = readTSCp();
        if (t != 0) {
            sacd_time += end - start;
        }
    
        memcpy(text_temp, text, text_len * sizeof(char));
        start = readTSC();
        sds_res = str_replace_sds(text_temp, from, to);
        end = readTSCp();
        if (t != 0) {
            sds_time += end - start;
        }

        memcpy(text_temp, text, text_len * sizeof(char));
        start = readTSC();
        cust_res = str_replace_custom(text_temp, from, to);
        end = readTSCp();
        if (t != 0) {
            cust_time += end - start;
        }
    }

    size_t sacd_res_len = strlen(sacd_res), sds_res_len = strlen(sds_res), cust_res_len = strlen(cust_res);
    if ((sacd_res_len == sds_res_len) 
        && (sacd_res_len == cust_res_len)
        && (strcmp(sacd_res, sds_res) == 0)
        && (strcmp(sacd_res, cust_res) == 0))
    {
        printf("\nAll results are the same\n\n");
    }

    uint64_t max_time = max(sacd_time, max(sds_time, cust_time));

    printf("Mean values over %u interations\n", times - 1);
    printf("%16s %10llu %.2f%\n",
        "SDS delay", sds_time / (times - 1),
        100.f * sds_time / max_time);
    printf("%16s %10llu %.2f%\n",
        "SACD delay", sacd_time / (times - 1),
        100.f * sacd_time / max_time);
    printf("%16s %10llu %.2f%\n",
        "Custom delay", cust_time / (times - 1),
        100.f * cust_time / max_time);

    free(sacd_res);
    free(sds_res);
    free(cust_res);


    free(text);
    free(text_temp);

    return 0;
}