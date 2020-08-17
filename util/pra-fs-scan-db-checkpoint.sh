#!/bin/bash
sqlite3 -bail pra-fs-scan.db <<_EOF
PRAGMA wal_checkpoint(TRUNCATE);
_EOF
sqlite3 -bail pra-fs-search.db <<_EOF
PRAGMA wal_checkpoint(TRUNCATE);
_EOF
