#include "csv.h"
#include "dump.h"

#define DESCRIPTION "convert csv to bsv\n\n"
#define USAGE "... | bsv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcut 3,2,1 | csv\nc,b,a\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // setup input
    CSV_INIT();

    // setup output
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;

    // process input row by row
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max > 0 || csv_sizes[0] > 0) {
            row.max = csv_max;
            for (i32 i = 0; i <= row.max; i++) {
                row.columns[i] = csv_columns[i];
                row.sizes[i] = csv_sizes[i];
            }
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);
}
