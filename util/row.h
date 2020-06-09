#pragma once

#include "util.h"

typedef struct row_s {
    i32 stop;
    i32 max;
    i32 sizes[MAX_COLUMNS];
    u8 *columns[MAX_COLUMNS];
} row_t;

typedef struct raw_row_s {
    #ifdef ROW_META
        i32 meta;
    #endif
    u8 *header;
    i32 header_size;
    u8 *buffer;
    i32 buffer_size;
} raw_row_t;

inlined void row_to_raw(row_t *row, raw_row_t *raw_row) {
    raw_row->header_size = sizeof(u16) + (row->max + 1) * sizeof(u16);
    raw_row->header = row->columns[0] - raw_row->header_size;
    raw_row->buffer = row->columns[0];
    raw_row->buffer_size = 0;
    for (i32 i = 0; i <= row->max; i++)
        raw_row->buffer_size += row->sizes[i] + 1;
}
