#include "load_dump.h"
#include "row.h"
#include "kvec.h"

#define NUM_ARGS 1
#define DESCRIPTION "sort rows\n\n"
#define USAGE "... | sort\n\n"
#define EXAMPLE ">> echo '\nc\nb\na\n' | bsv | bsort | csv\na\nb\nc\n\n"

#define SORT_NAME str
#define SORT_TYPE row_t *
#define SORT_CMP(x, y) strcmp((x)->buffer, (y)->buffer)
#include "sort.h"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    ROW_INIT();
    int offset;
    int i;
    int j;

    kvec_t(row_t*) array;

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        ROW(load_columns[0], load_max, load_size, load_sizes, load_columns);
        kv_push(row_t*, array, row);
    }

    str_tim_sort(array.a, array.n);

    for (i = 0; i < array.n; i++) {
        row = array.a[i];
        DUMP(0, row->max, row->columns, row->sizes);
    }

    DUMP_FLUSH(0);
}
