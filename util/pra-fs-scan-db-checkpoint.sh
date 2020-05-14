#!/bin/bash
sqlite3 -bail pra-fs-scan.db <<_EOF
PRAGMA wal_checkpoint(TRUNCATE);
_EOF
