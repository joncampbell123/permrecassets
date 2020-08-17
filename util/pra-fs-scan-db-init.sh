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

# keyword search
# for a word, normalize it (lowercase) and do an EXACT match against "word" column.
# you will get a node_id to use in the nodes table. The reason for the match is in
# the "mtype" column.
#
# node_id will be NULL if it is not a node match
#
# mtime is the unix time the keyword was entered.
#
# context depends on mtype to provide additional information.
# in most cases (with exceptions) this is a short copy of the text around the keyword.
#
# weight is used to deemphasize very common words or words that happen more often in
# source.
#
# mtype:
#  FN = filename match
sqlite3 -bail pra-fs-search.db <<_EOF
CREATE TABLE IF NOT EXISTS dict (
    word                BLOB PRIMARY KEY NOT NULL,
    node_id             BLOB,
    mtype               TEXT,
    mtime               INTEGER,
    context             BLOB,
    weight              REAL
);
CREATE INDEX IF NOT EXISTS dict_word ON dict(word);
CREATE INDEX IF NOT EXISTS dict_node_id ON dict(node_id);
PRAGMA journal_mode = WAL;
_EOF

