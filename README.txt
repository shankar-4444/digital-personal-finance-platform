Digital Personal Finance Platform - Dynamic Array Migration

Build:
    make

Run:
    ./build/finance

Changes:
- Transactions now use a dynamically allocated array.
- Capacity expands as needed, up to MAX_TRANSACTIONS (100000).
- Capacity growth checks SIZE_MAX and allocation failures.
- Loading rejects negative counts and counts above MAX_TRANSACTIONS.
- Loading allocates only after validating the stored count and checks allocation/read failures.
- main frees transaction storage on normal exit.
- Budget storage remains the existing fixed array because there are at most 9 supported categories.

Important:
- This is a migrated working copy. Keep your original project files until you compile and test this version.
- transactions.dat is still written using "wb"; a failed write can still truncate the existing file. Temporary-file replacement is a separate improvement.
- File format is unchanged (int record count followed by transaction structs), but binary struct files can vary between compiler/platform/struct-layout versions. Keep backups of existing data files.
