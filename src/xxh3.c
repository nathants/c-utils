#include "util.h"
#include "read_simple.h"
#include "write_simple.h"
#include "argh.h"
#include "xxh3.h"

#define DESCRIPTION "xxh3_64 hash stdin\n\n"
#define USAGE "... | xxh3 [--stream|--int]\n\n"
#define EXAMPLE                                                       \
    "  --stream pass stdin through to stdout with hash on stderr\n\n" \
    "  --int output hash as int not hash\n\n"                         \
    ">> echo abc | xxh3\n079364cbfdf9f4cb\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1);

    // parse args
    bool int_out = false;
    bool stream = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if      ARGH_BOOL("-i", "--int")    { int_out = true; }
        else if ARGH_BOOL("-s", "--stream") { stream = true; }
    }

    // setup state
    XXH3_state_t state;
    ASSERT(XXH3_64bits_reset(&state) != XXH_ERROR, "xxh3 reset failed\n");

    // process input row by row
    while (1) {
        read_bytes(&rbuf, BUFFER_SIZE, 0);
        ASSERT(XXH3_64bits_update(&state, rbuf.buffer, rbuf.bytes) != XXH_ERROR, "xxh3 update failed\n");
        if (stream)
            write_bytes(&wbuf, rbuf.buffer, rbuf.bytes, 0);
        if (BUFFER_SIZE != rbuf.bytes)
            break;
    }

    //
    u64 hash = XXH3_64bits_digest(&state);
    FILE *out = (stream) ? stderr : stdout;
    if (int_out)
        FPRINTF(out, "%lu\n", hash);
    else
        FPRINTF(out, "%08x%08x\n", (i32)(hash>>32), (i32)hash);
    if (stream)
        write_flush(&wbuf, 0);
}
