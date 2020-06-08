## why

it should be possible to process data at speeds approaching that of sequential io.

sequential io is fast. cpu is the bottleneck. sequential only data access is the play.

## what

a simple and efficient [data](https://github.com/nathants/bsv/blob/master/util/load.h) [format](https://github.com/nathants/bsv/blob/master/util/dump.h) for sequentially manipulating chunks of rows of columns without allocating or copying.

[cli](https://github.com/nathants/bsv/blob/master/src) utilities to manipulate data based on [shared](https://github.com/nathants/bsv/blob/master/util) code.

## how

a column is 0-65536 bytes.

a [row](https://github.com/nathants/bsv/blob/master/util/row.h) is 0-65536 columns.

a chunk is up to 5MB containing 1 or more complete rows.

note: row data cannot exceed chunk size.

## layout

[chunk](https://github.com/nathants/bsv/blob/master/util/read.h): `| i32:chunk_size | row-1 | ... | row-n |`

[row](https://github.com/nathants/bsv/blob/master/util/row.h): `| u16:max | u16:size-1 | ... | u16:size-n | u8:column-1 + \0 | ... | u8:column-n + \0 |`

note: column bytes are always followed by a single nullbyte: `\0`

note: max is the maximum zero based index into the row, ie: `max = size(row) - 1`

## testing methodology

[quickcheck](https://hypothesis.readthedocs.io/en/latest/) style [testing](https://github.com/nathants/bsv/blob/master/test) with python implementations of every utility to verify correct behavior for arbitrary inputs and varying buffer sizes.

## experiments

[performance](https://github.com/nathants/bsv/blob/master/experiments/readme.md) experiments and alternate implementations.

## utilities

- [bbucket](#bbucket) - prefix each row with a u64 consistent hash of the first column
- [bcat](#bcat) - cat some bsv file to csv
- [bcopy](#bcopy) - pass through data, to benchmark load/dump performance
- [bcounteach](#bcounteach) - count as u64 each contiguous identical row by strcmp the first column
- [bcountrows](#bcountrows) - count rows as u64
- [bcut](#bcut) - select some columns
- [bdedupe](#bdedupe) - dedupe identical contiguous rows by strcmp the first column, keeping the first
- [bdropuntil](#bdropuntil) - drop until the first column is gte to VALUE
- [bmerge](#bmerge) - merge sorted files
- [bpartition](#bpartition) - split into multiple files by the first column value
- [brmerge](#brmerge) - merge reverse sorted files
- [brsort](#brsort) - reverse timsort rows by strcmp the first column
- [bschema](#bschema) - validate and convert column values
- [bsort](#bsort) - timsort rows by strcmp the first column
- [bsplit](#bsplit) - split a stream into multiple files
- [bsum](#bsum) - u64 sum the first column
- [bsv](#bsv) - convert csv to bsv, numerics remain ascii for faster parsing
- [btake](#btake) - take while the first column is VALUE
- [btakeuntil](#btakeuntil) - take until the first column is gte to VALUE
- [csv](#csv) - convert bsv to csv, numerics are treated as ascii
- [xxh3](#xxh3) - xxh3_64 hash stdin. defaults to hex, can be --int. --stream to pass stdin through to stdout with hash on stderr

### [bbucket](https://github.com/nathants/bsv/blob/master/src/bbucket.c)

prefix each row with a u64 consistent hash of the first column

usage: `... | bbucket NUM_BUCKETS`

```
>> echo '
a
b
c
' | bsv | bbucket 100 | csv
50,a
39,b
83,c
```

### [bcat](https://github.com/nathants/bsv/blob/master/src/bcat.c)

cat some bsv file to csv

usage: `bcat [--prefix] [--head NUM] FILE1 ... FILEN`

```
>> for char in a a b b c c; do
     echo $char | bsv >> /tmp/$char
   done

>> bcat --head 1 --prefix /tmp/{a,b,c}
/tmp/a:a
/tmp/b:b
/tmp/c:c
```

### [bcopy](https://github.com/nathants/bsv/blob/master/src/bcopy.c)

pass through data, to benchmark load/dump performance

usage: `... | bcopy`

```
>> echo a,b,c | bsv | bcopy | csv
a,b,c
```

### [bcounteach](https://github.com/nathants/bsv/blob/master/src/bcounteach.c)

count as u64 each contiguous identical row by strcmp the first column

usage: `... | bcounteach`

```
echo 'a
a
b
b
b
a
' | bsv | bcounteach | csv
a,2
b,3
a,1
```

### [bcountrows](https://github.com/nathants/bsv/blob/master/src/bcountrows.c)

count rows as u64

usage: `... | bcountrows`

```
>> echo -e '1
2
3
4.1
' | bsv | bcountrows | csv
4
```

### [bcut](https://github.com/nathants/bsv/blob/master/src/bcut.c)

select some columns

usage: `... | bcut FIELD1,...,FIELDN`

```
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### [bdedupe](https://github.com/nathants/bsv/blob/master/src/bdedupe.c)

dedupe identical contiguous rows by strcmp the first column, keeping the first

usage: `... | bdedupe`

```
>> echo '
a
a
b
b
a
a
' | bsv | bdedupe | csv
a
b
a
```

### [bdropuntil](https://github.com/nathants/bsv/blob/master/src/bdropuntil.c)

drop until the first column is gte to VALUE

usage: `... | bdropuntil VALUE`

```
>> echo '
a
b
c
d
' | bsv | bdropuntil c | csv
c
d
```

### [bmerge](https://github.com/nathants/bsv/blob/master/src/bmerge.c)

merge sorted files

usage: `bmerge FILE1 ... FILEN`

```
>> echo -e 'a
c
e
' | bsv > a.bsv
>> echo -e 'b
d
f
' | bsv > b.bsv
>> bmerge a.bsv b.bsv
a
b
c
d
e
f
```

### [bpartition](https://github.com/nathants/bsv/blob/master/src/bpartition.c)

split into multiple files by the first column value

usage: `... | bbucket NUM_BUCKETS | bpartition PREFIX NUM_BUCKETS`

```
>> echo '
0,a
1,b
2,c
' | bsv | bpartition prefix 10
prefix00
prefix01
prefix02
```

### [brmerge](https://github.com/nathants/bsv/blob/master/src/brmerge.c)

merge reverse sorted files

usage: `brmerge FILE1 ... FILEN`

```
>> echo -e 'e
c
a
' | bsv > a.bsv
>> echo -e 'f
d
b
' | bsv > b.bsv
>> brmerge a.bsv b.bsv
f
e
d
c
b
a
```

### [brsort](https://github.com/nathants/bsv/blob/master/src/brsort.c)

reverse timsort rows by strcmp the first column

usage: `... | bsort`

```
>> echo '
a
b
c
' | bsv | brsort | csv
c
b
a
```

### [bschema](https://github.com/nathants/bsv/blob/master/src/bschema.c)

validate and convert column values

usage: `... | bschema 4,u64:a,a:i32,2,*,...`

```
>> echo aa,bbb,cccc | bsv | bschema 2,3,4 | csv
aa,bbb,cccc
```

### [bsort](https://github.com/nathants/bsv/blob/master/src/bsort.c)

timsort rows by strcmp the first column

usage: `... | bsort`

```
>> echo '
c
b
a
' | bsv | bsort | csv
a
b
c
```

### [bsplit](https://github.com/nathants/bsv/blob/master/src/bsplit.c)

split a stream into multiple files

usage: `... | bsplit [chunks_per_file=1]`

```
>> echo -n a,b,c | bsv | bsplit
1595793589_0000000000
```

### [bsum](https://github.com/nathants/bsv/blob/master/src/bsum.c)

u64 sum the first column

usage: `... | bsum`

```
>> echo -e '1
2
3
4
' | bsv | bschema a:u64 | bsum | bschema u64:a | csv
10
```

### [bsv](https://github.com/nathants/bsv/blob/master/src/bsv.c)

convert csv to bsv, numerics remain ascii for faster parsing

usage: `... | bsv`

```
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```

### [btake](https://github.com/nathants/bsv/blob/master/src/btake.c)

take while the first column is VALUE

usage: `... | btake VALUE`

```
>> echo '
a
b
c
d
' | bsv | bdropntil c | btake c | csv
c
```

### [btakeuntil](https://github.com/nathants/bsv/blob/master/src/btakeuntil.c)

take until the first column is gte to VALUE

usage: `... | btakeuntil VALUE`

```
>> echo '
a
b
c
d
' | bsv | btakeuntil c | csv
a
b
```

### [csv](https://github.com/nathants/bsv/blob/master/src/csv.c)

convert bsv to csv, numerics are treated as ascii

usage: `... | csv`

```
>> echo a,b,c | bsv | csv
a,b,c
```

### [xxh3](https://github.com/nathants/bsv/blob/master/src/xxh3.c)

xxh3_64 hash stdin. defaults to hex, can be --int. --stream to pass stdin through to stdout with hash on stderr

usage: `... | xxh3 [--stream|--int]`

```
>> echo abc | xxh3
B5CA312E51D77D64
```
