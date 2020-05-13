#!/bin/bash
sqlite3 -bail pra-fs-scan.db <<_EOF
CREATE TABLE IF NOT EXISTS nodes (
    node_id             BLOB PRIMARY KEY UNIQUE NOT NULL,
    parent_node         BLOB NOT NULL,
    name                TEXT NOT NULL,
    real_name           BLOB NOT NULL,
    name_charset        TEXT,
    size                INTEGER NOT NULL,
    type                INTEGER NOT NULL,
    mime_string         TEXT,
    content_encoding    TEXT,
    flags               INTEGER,
    mtime               INTEGER,
    inode               INTEGER
);
CREATE UNIQUE INDEX IF NOT EXISTS nodes_node_id ON nodes(node_id);
CREATE UNIQUE INDEX IF NOT EXISTS nodes_parent_node_real_name_inode ON nodes(parent_node,real_name,inode);
PRAGMA journal_mode = WAL;
_EOF

# Nice to have, but performance issue once >= 2GB
# CREATE        INDEX IF NOT EXISTS nodes_real_name ON nodes(real_name);
# CREATE        INDEX IF NOT EXISTS nodes_name ON nodes(name);
