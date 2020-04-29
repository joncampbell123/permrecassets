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
CREATE        INDEX IF NOT EXISTS nodes_parent_node ON nodes(parent_node);
CREATE        INDEX IF NOT EXISTS nodes_name ON nodes(name);
CREATE        INDEX IF NOT EXISTS nodes_real_name ON nodes(real_name);
CREATE UNIQUE INDEX IF NOT EXISTS nodes_real_name_parent_node_type ON nodes(real_name,parent_node,type);
PRAGMA journal_mode = WAL;
_EOF

