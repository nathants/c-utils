#include "util.h"
#include "argh.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "validate and converts row data with a schema of columns\n\n"
#define USAGE "... | bschema SCHEMA [--filter]\n\n"
#define EXAMPLE                                                                                   \
    "  --filter remove bad rows instead of erroring\n\n"                                          \
    "  example schemas:\n"                                                                        \
    "    *,*,*             = 3 columns of any size\n"                                             \
    "    8,*               = a column with 8 bytes followed by a column of any size\n"            \
    "    8,*,...           = same as above, but ignore any trailing columns\n"                    \
    "    a:u16,a:i32,a:f64 = convert ascii to numerics\n"                                         \
    "    u16:a,i32:a,f64:a = convert numerics to ascii\n"                                         \
    "    4*,*4             = keep the first 4 bytes of column 1 and the last 4 of column 2\n\n"   \
    ">> echo aa,bbb,cccc | bsv | bschema 2,3,4 | csv\naa,bbb,cccc\n"

#define FILTERING_ASSERT(cond, ...)             \
    do {                                        \
        if (!(cond)) {                          \
            if (filtering) {                    \
                filtered = true;                \
            } else {                            \
                fprintf(stderr, ##__VA_ARGS__); \
                exit(1);                        \
            }                                   \
        }                                       \
    } while(0)

enum conversion {

    // bytes
    PASS,
    SIZE,
    HEAD,
    TAIL,

    // int
    A_I16,
    A_I32,
    A_I64,
    I16_A,
    I32_A,
    I64_A,

    // uint
    A_U16,
    A_U32,
    A_U64,
    U16_A,
    U32_A,
    U64_A,

    // float
    A_F32,
    A_F64,
    F32_A,
    F64_A,

};

#define INIT(type)                              \
    type _##type;

#define N_TO_A(type, format)                                                                                        \
    FILTERING_ASSERT(sizeof(type) == row.sizes[i], "fatal: number->ascii didn't have the write number of bytes\n"); \
    SNNPRINTF(n, scratch + scratch_offset, BUFFER_SIZE - scratch_offset, format, *(type*)row.columns[i]);           \
    ASSERT(scratch_offset + n < BUFFER_SIZE, "fatal: scratch overflow\n");                                          \
    row.columns[i] = scratch + scratch_offset;                                                                      \
    row.sizes[i] = n;                                                                                               \
    scratch_offset += n;

#define A_TO_L(type, min, max)                                                                          \
    ASSERT(sizeof(type) < BUFFER_SIZE - scratch_offset, "fatal: scratch overflow\n");                   \
    tmpl = strtol(row.columns[i], NULL, 10);                                                            \
    ASSERT(tmpl > LONG_MIN, "fatal: above max value: %ld > %ld\n", tmpl, LONG_MIN);                     \
    ASSERT(tmpl < LONG_MAX, "fatal: above max value: %ld > %ld\n", tmpl, LONG_MAX);                     \
    ASSERT(tmpl >= (i64)min, "fatal: below min value: %ld < %ld\n", tmpl, (i64)min);                    \
    ASSERT(tmpl <= (i64)max, "fatal: above max value: %ld > %ld\n", tmpl, (i64)max);                    \
    _##type = tmpl;                                                                                     \
    memcpy(scratch + scratch_offset, &_##type, sizeof(type));                                           \
    row.columns[i] = scratch + scratch_offset;                                                          \
    row.sizes[i] = sizeof(type);                                                                        \
    scratch_offset += sizeof(type);

#define A_TO_UL(type, max)                                                                              \
    ASSERT(sizeof(type) < BUFFER_SIZE - scratch_offset, "fatal: scratch overflow\n");                   \
    ASSERT(compare_str(row.columns[i], "0") >= 0, "fatal: unsigned value cannot be below zero\n");      \
    tmpul = strtoul(row.columns[i], NULL, 10);                                                          \
    _##type = tmpul;                                                                                    \
    ASSERT(tmpul < ULONG_MAX, "fatal: above max value: %lu > %lu\n", tmpul, ULONG_MAX);                 \
    ASSERT(tmpul <= (u64)max, "fatal: above max value: %lu > %lu\n", tmpul, (u64)max);                  \
    memcpy(scratch + scratch_offset, &_##type, sizeof(type));                                           \
    row.columns[i] = scratch + scratch_offset;                                                          \
    row.sizes[i] = sizeof(type);                                                                        \
    scratch_offset += sizeof(type);

#define A_TO_F(type)                                                                    \
    ASSERT(sizeof(type) < BUFFER_SIZE - scratch_offset, "fatal: scratch overflow\n");   \
    _##type = atof(row.columns[i]);                                                     \
    memcpy(scratch + scratch_offset, &_##type, sizeof(type));                           \
    row.columns[i] = scratch + scratch_offset;                                          \
    row.sizes[i] = sizeof(type);                                                        \
    scratch_offset += sizeof(type);

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    i32 exact = 1;
    row_t row;
    u8 *f;
    u8 f_butlast[1024];
    i32 max = -1;
    i32 conversion[MAX_COLUMNS];
    i32 args[MAX_COLUMNS];
    i64 num_filtered = 0;
    bool filtered;
    i64 tmpl;
    u64 tmpul;

    // parse args
    bool filtering = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-f", "--filter") { filtering = true; }
    }
    ASSERT(ARGH_ARGC == 1, "usage: %s", USAGE);
    u8 *fs = ARGH_ARGV[0];

    // parse schema
    while ((f = strsep(&fs, ","))) {
        args[++max] = -1;
        ASSERT(strlen(f) < sizeof(f_butlast), "fatal: schema too large\n");
        strcpy(f_butlast, f);
        f_butlast[strlen(f) - 1] = '\0';

        // bytes
        if (strcmp(f, "*") == 0) {
            conversion[max] = PASS;
        } else if (isdigits(f)) {
            conversion[max] = SIZE;
            args[max] = atoi(f);
        } else if (strlen(f) > 1 && f[0] == '*' && isdigits(f + 1)) {
            conversion[max] = TAIL;
            args[max] = atoi(f + 1);
        } else if (strlen(f) > 1 && f[strlen(f) - 1] == '*' && isdigits(f_butlast)) {
            conversion[max] = HEAD;
            args[max] = atoi(f_butlast);
        }

        // int
        else if (strcmp(f, "a:i16") == 0) { conversion[max] = A_I16; }
        else if (strcmp(f, "a:i32") == 0) { conversion[max] = A_I32; }
        else if (strcmp(f, "a:i64") == 0) { conversion[max] = A_I64; }
        else if (strcmp(f, "i16:a") == 0) { conversion[max] = I16_A; }
        else if (strcmp(f, "i32:a") == 0) { conversion[max] = I32_A; }
        else if (strcmp(f, "i64:a") == 0) { conversion[max] = I64_A; }

        // uint
        else if (strcmp(f, "a:u16") == 0) { conversion[max] = A_U16; }
        else if (strcmp(f, "a:u32") == 0) { conversion[max] = A_U32; }
        else if (strcmp(f, "a:u64") == 0) { conversion[max] = A_U64; }
        else if (strcmp(f, "u16:a") == 0) { conversion[max] = U16_A; }
        else if (strcmp(f, "u32:a") == 0) { conversion[max] = U32_A; }
        else if (strcmp(f, "u64:a") == 0) { conversion[max] = U64_A; }

        // float
        else if (strcmp(f, "a:f32") == 0) { conversion[max] = A_F32; }
        else if (strcmp(f, "a:f64") == 0) { conversion[max] = A_F64; }
        else if (strcmp(f, "f32:a") == 0) { conversion[max] = F32_A; }
        else if (strcmp(f, "f64:a") == 0) { conversion[max] = F64_A; }

        // allow trailing columns
        else if (strcmp(f, "...") == 0) { exact = 0; max--; break; }

        else ASSERT(0, "fatal: bad schema: %s\n", f);

    }

    // scratch buffer
    i32 scratch_offset;
    u8 *scratch;
    MALLOC(scratch, BUFFER_SIZE);
    i32 n;

    // int
    INIT(i16);
    INIT(i32);
    INIT(i64);

    // uint
    INIT(u16);
    INIT(u32);
    INIT(u64);

    // float
    INIT(f32);
    INIT(f64);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;

        filtered = false;

        if (exact)
            FILTERING_ASSERT(max == row.max, "fatal: row had %d columns, needed %d\n", row.max + 1, max + 1);
        else
            FILTERING_ASSERT(max <= row.max, "fatal: row had %d columns, needed at least %d\n", row.max + 1, max + 1);

        if (filtered) {
            num_filtered++;
            continue;
        }

        scratch_offset = 0;
        for (i32 i = 0; i <= max; i++) {
            switch (conversion[i]) {

                // bytes
                case PASS: break;
                case SIZE: FILTERING_ASSERT(row.sizes[i] == args[i], "fatal: column %d was size %d, needed to be %d\n", i, row.sizes[i], args[i]); break;
                case HEAD: FILTERING_ASSERT(row.sizes[i] >= args[i], "fatal: column %d was size %d, needed to be %d\n", i, row.sizes[i], args[i]); row.sizes[i] = args[i]; break;
                case TAIL: FILTERING_ASSERT(row.sizes[i] >= args[i], "fatal: column %d was size %d, needed to be %d\n", i, row.sizes[i], args[i]); row.columns[i] = row.columns[i] + (row.sizes[i] - args[i]); row.sizes[i] = args[i]; break;

                // int
                case A_I16: A_TO_L(i16, SHRT_MIN, SHRT_MAX); break;
                case A_I32: A_TO_L(i32, INT_MIN, INT_MAX); break;
                case A_I64: A_TO_L(i64, LONG_MIN, LONG_MAX); break;
                case I16_A: N_TO_A(i16, "%d"); break;
                case I32_A: N_TO_A(i32, "%d"); break;
                case I64_A: N_TO_A(i64, "%ld"); break;

                // uint
                case A_U16: A_TO_UL(u16, USHRT_MAX); break;
                case A_U32: A_TO_UL(u32, UINT_MAX); break;
                case A_U64: A_TO_UL(u64, ULONG_MAX); break;
                case U16_A: N_TO_A(u16, "%u"); break;
                case U32_A: N_TO_A(u32, "%u"); break;
                case U64_A: N_TO_A(u64, "%lu"); break;

                // float
                case A_F32: A_TO_F(f32); break;
                case A_F64: A_TO_F(f64); break;
                case F32_A: N_TO_A(f32, "%.8g"); break;
                case F64_A: N_TO_A(f64, "%.16g"); break;

                default: ASSERT(0, "not possible\n");
            }
        }

        if (filtered) {
            num_filtered++;
            continue;
        }

        row.max = max;

        dump(&wbuf, &row, 0);
    }

    dump_flush(&wbuf, 0);

    if (num_filtered > 0)
        DEBUG("filtered: %lu\n", num_filtered);
}
