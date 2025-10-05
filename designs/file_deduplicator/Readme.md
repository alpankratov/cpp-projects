# Bayan — file deduplicator (designs/file_deduplicator)

Bayan is a console utility that scans one or more directories and prints groups of duplicate files. Two files are considered duplicates if they have the same size and the same content hash (computed block-by-block).

Output format:
- Absolute file paths, one per line.
- Blank line separates duplicate groups.
- If no duplicates are found, no output is produced.

## Build

This project is already wired in the parent CMake workspace. Use the existing CLion profile and target:

```
cmake --build ./cmake-build-debug --target bayan
```

The resulting executable will be here:
- <PROJECT_DIR>/designs/file_deduplicator/cmake-build-debug/bayan

You can also see a convenience copy at the project root (if present), but the canonical path is the one above.

## Command‑line options

```
--scan-dir <dir...>     One or more directories to scan (required)
--exclude-dir <dir...>  One or more directories to exclude (optional)
--depth <n>             Recursion depth (0 = only the directory itself; -1 = unlimited)
--min-size <bytes>      Minimal file size to consider, in bytes (default: 2)
--mask <glob...>        Case-insensitive glob masks for filenames, e.g. *.txt *.csv (optional)
--block-size <bytes>    Size of a block in bytes for hashing (default: 4096)
--hash <algo>           Hash algorithm: crc32 (default) or md5
-h, --help              Show help and exit
```

Notes:
- Masks are case-insensitive and support `*` and `?` wildcards.
- Excluded directories are skipped entirely (no descent into them).
- Depth applies per `--scan-dir`. When `depth=0` only the top directory is scanned.

## Demo data and example commands

The repository contains demo files to quickly try the tool:
- Directory: designs/file_deduplicator/demo
- Excluded sample subdir: designs/file_deduplicator/demo/exclude

Files summary:
- small_duplicate_1.txt, small_duplicate_2.txt — duplicates (12 bytes)
- small_duplicate_1.csv, small_duplicate_2.csv — duplicates (16 bytes)
- large_duplicate_1.txt, large_duplicate_2.txt — duplicates (>800 bytes)
- small_no_duplicates.txt, large_no_duplicates.txt — unique
- exclude/secret_duplicate_1.txt, exclude/secret_duplicate_2.txt — duplicates inside the exclude folder

1) Basic scan of demo (find all duplicates; unlimited depth):
```
./cmake-build-debug/bayan --scan-dir designs/file_deduplicator/demo
```

2) Exclude the "exclude" folder inside demo:
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --exclude-dir designs/file_deduplicator/demo/exclude
```

3) Limit depth to not descend into subdirectories (depth 0):
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --depth 0
```
See how files in the "exclude" folder are not scanned even though they are not excluded by `--exclude-dir ` parameter.

4) Only check specific filename masks (only CSV), case-insensitive:
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --mask *.CSV
```

5) Ignore tiny files (e.g., smaller than 100 bytes):
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --min-size 100
```

6) Use MD5 as the hashing algorithm with a larger block size (e.g., 16 KiB):
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --hash md5 \
  --block-size 16384
```

7) Combine multiple options:
```
./cmake-build-debug/bayan \
  --scan-dir designs/file_deduplicator/demo \
  --exclude-dir designs/file_deduplicator/demo/exclude \
  --mask *.txt \
  --min-size 10 \
  --hash crc32
```

Expected output:
- Absolute paths of duplicate files grouped together, separated by a blank line. For example, you should see groups for the small_* duplicates and the large_* duplicates. If you exclude the demo/exclude folder, the secret_* duplicates will not appear in the results.

## Exit codes
- 0: Successful execution (regardless of whether duplicates were found)
- 1: Invalid arguments or runtime error

## License
This directory is a study/demo implementation for a file deduplicator ("bayan"). See the root project license if applicable.
