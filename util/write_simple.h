#pragma once

#include "util.h"

typedef struct writebuf_s {
    // private
    FILE **files;
    u8 **buffer;
    i32 *offset;
} writebuf_t;


void wbuf_init(writebuf_t *buf, FILE **files, i32 num_files) {
    buf->files = files;
    MALLOC(buf->buffer, sizeof(u8*) * num_files);
    MALLOC(buf->offset, sizeof(i32) * num_files);
    for (i32 i = 0; i < num_files; i++) {
        buf->offset[i] = 0;
        MALLOC(buf->buffer[i], BUFFER_SIZE);
    }
}

inlined void write_flush(writebuf_t *buf, i32 file) {
    if (buf->offset[file]) {
        FWRITE(buf->buffer[file], buf->offset[file], buf->files[file]);
        buf->offset[file] = 0;
    }
}

inlined void write_bytes(writebuf_t *buf, u8 *bytes, i32 size, i32 file) {
    ASSERT(size <= BUFFER_SIZE, "fatal: cant write more than BUFFER_SIZE\n");
    if (size > BUFFER_SIZE - buf->offset[file])
        write_flush(buf, file);
    memcpy(buf->buffer[file] + buf->offset[file], bytes, size);
    buf->offset[file] += size;
}
