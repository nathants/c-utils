.PHONY: all clean test
CFLAGS=-Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native
ALL=docs bbucket bcat bcopy bcounteach bcountrows bcut bdedupe bdropuntil bmerge bpartition brmerge brsort bschema bsort bsplit bsumeachf64 bsumeachu64 bsumu64 bsv btake btakeuntil _copy _csv csv _gen_bsv _gen_csv xxh3

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL) *.*

docs:
	./scripts/readme.py

test: setup
	tox

bbucket: setup
	gcc $(CFLAGS) src/bbucket.c -o bin/bbucket

bcat: setup
	gcc $(CFLAGS) src/bcat.c -o bin/bcat

bcopy: setup
	gcc $(CFLAGS) src/bcopy.c -o bin/bcopy

bcounteach: setup
	gcc $(CFLAGS) src/bcounteach.c -o bin/bcounteach

bcountrows: setup
	gcc $(CFLAGS) src/bcountrows.c -o bin/bcountrows

bcut: setup
	gcc $(CFLAGS) src/bcut.c -o bin/bcut

bdedupe: setup
	gcc $(CFLAGS) src/bdedupe.c -o bin/bdedupe

bdropuntil: setup
	gcc $(CFLAGS) src/bdropuntil.c -o bin/bdropuntil

bmerge: setup
	gcc $(CFLAGS) src/bmerge.c -o bin/bmerge

bpartition: setup
	gcc $(CFLAGS) src/bpartition.c -o bin/bpartition

brmerge: setup
	gcc $(CFLAGS) src/brmerge.c -o bin/brmerge

brsort: setup
	gcc $(CFLAGS) src/brsort.c -o bin/brsort

bschema: setup
	gcc $(CFLAGS) src/bschema.c -o bin/bschema

bsort: setup
	gcc $(CFLAGS) src/bsort.c -o bin/bsort

bsplit: setup
	gcc $(CFLAGS) src/bsplit.c -o bin/bsplit

bsumeachf64: setup
	gcc $(CFLAGS) src/bsumeachf64.c -o bin/bsumeachf64

bsumeachu64: setup
	gcc $(CFLAGS) src/bsumeachu64.c -o bin/bsumeachu64

bsumu64: setup
	gcc $(CFLAGS) src/bsumu64.c -o bin/bsumu64

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

btake: setup
	gcc $(CFLAGS) src/btake.c -o bin/btake

btakeuntil: setup
	gcc $(CFLAGS) src/btakeuntil.c -o bin/btakeuntil

_copy: setup
	gcc $(CFLAGS) src/_copy.c -o bin/_copy

_csv: setup
	gcc $(CFLAGS) src/_csv.c -o bin/_csv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

_gen_bsv: setup
	gcc $(CFLAGS) src/_gen_bsv.c -o bin/_gen_bsv

_gen_csv: setup
	gcc $(CFLAGS) src/_gen_csv.c -o bin/_gen_csv

xxh3: setup
	gcc $(CFLAGS) src/xxh3.c -o bin/xxh3

