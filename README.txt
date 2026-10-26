Digital Personal Finance Platform

BUILD AND RUN
    make
    ./build/finance

AUTOMATED CHECKS
    make test       Build and run the Google Test suite in isolated test data.
    make valgrind   Run the startup/exit smoke test under Valgrind.
    make cppcheck   Run general Cppcheck static analysis.
    make misra      Run Cppcheck's MISRA addon, if supported by the installed Cppcheck package.
    make check      Run all checks in sequence.
    make clean      Remove generated executable, test executable, and object files.

REQUIREMENTS (Ubuntu/WSL)
    sudo apt update
    sudo apt install build-essential valgrind cppcheck libgtest-dev

FEATURES
- Transactions use a dynamically allocated array with a maximum of 100000 records.
- Capacity growth checks allocation limits and allocation failures.
- Transaction records include a timestamp in YYYY-MM-DD HH:MM:SS format.
- Add, update, and delete operations append a timestamped entry to activity.log.
- Existing transaction files using the previous record structure can be loaded. Their historical timestamps are shown as Unknown because the old format did not store timestamps.
- New transaction files store the timestamp field. Back up .dat files before running a changed build.
- Savings and budget data continue to use savings.dat and budgets.dat.
- Budget storage remains a fixed array because at most 9 categories are supported.

DATA SAFETY
Google Tests create and remove transactions.dat, savings.dat, budgets.dat, and activity.log in the current working directory. The Makefile runs tests from build/gtest-data to protect real data. Valgrind also runs from build/valgrind-data. Do not manually run the test binary from the project root.

MISRA NOTE
The 'make misra' target uses the MISRA addon distributed with Cppcheck. It checks only the rules supported by that addon and the installed version. This is not a claim of full MISRA C compliance or certification. Review every finding and document justified deviations.

BINARY FILE NOTE
The .dat files are binary files tied to the C structure layout and platform/compiler conventions. Keep backups and use the application version that understands the file format. The transaction loader recognizes the project's prior transaction structure and labels old records' timestamps as Unknown.
