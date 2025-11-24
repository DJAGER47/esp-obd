#!/bin/bash

# Usage: idf.py clang-check [OPTIONS] [PATTERNS]...

#   run clang-tidy check under current folder, write the output into "warnings.txt"

# Options:
#   -C, --project-dir PATH         Project directory.
#   --all-files                    Run clang-tidy with all files. Default run with project dir files only. "--include-path" and "--exclude-path" would be ignored if using this flag.
#   --run-clang-tidy-py TEXT       run-clang-tidy.py path, this file could be downloaded from llvm. will use "run-clang-tidy.py" if not specified.
#   --run-clang-tidy-options TEXT  all optional arguments would be passed to run-clang-tidy.py. the value should be double-quoted
#   --include-paths TEXT           include extra files besides of the project dir. This option can be used for multiple times.
#   --exclude-paths TEXT           exclude extra files besides of the project dir. This option can be used for multiple times.
#   --exit-code                    Exit with code based on the results of the code analysis. By default, exit code reflects the success of
#                                  running the tool only.
#   --help                         Show this message and exit.



source ~/esp/v5.5.1/esp-idf/export.sh
mkdir -p clang_tidy_results

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_FILE="clang_tidy_results/clang_tidy_${TIMESTAMP}.txt"


idf.py clang-check --exclude-paths "~/esp/" \
                   --exclude-paths "/home/plavrovskiy/_My/esp_prj/obd2/managed_components" \
                   --run-clang-tidy-options="-checks=-*,bugprone-*,cert-*,clang-analyzer-*,cppcoreguidelines-*,google-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-constant-array-index,-misc-use-anonymous-namespace,-modernize-use-trailing-return-type,-readability-identifier-length,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-cppcoreguidelines-pro-type-vararg" 2>&1 | tee "${OUTPUT_FILE}";



# idf.py clang-html-report