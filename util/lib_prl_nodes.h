
#ifndef UTIL_LIB_PRL_NODES_H
#define UTIL_LIB_PRL_NODES_H

#include "lib_prluuid.h"
#include "lib_prl_blob.h"

extern const prluuid prl_zero_node;

enum prl_node_type {
    NODE_TYPE_NULL=0,                           // 0
    NODE_TYPE_FILE,                             // 1
    NODE_TYPE_DIRECTORY,                        // 2
    NODE_TYPE_ARCHIVE,                          // 3
    NODE_TYPE_ARCHIVE_COLLECTION,               // 4
    NODE_TYPE_VIEW,                             // 5
    NODE_TYPE_SYMLINK,                          // 6

    NODE_TYPE_MAX
};

enum prl_node_flags {
    NODE_FLAG_PRIVATE=(1 << 0),                 // private node, do not show publicly
    NODE_FLAG_USER_MODIFIED_CHARSET=(1 << 1)    // user modified charset, do not auto-modify later
};

/* instead of passing a million parameters for each part, pass a struct instead.
 * addition of fields is just add a new struct member and recompile away. */
struct prl_node_entry {
    prluuid         node_id;
    prluuid         parent_node;
    std::string     name;
    prl_blob        real_name;
    std::string     name_charset;
    uint64_t        size;
    unsigned int    type;
    std::string     mime_string;
    std::string     content_encoding;
    unsigned int    flags;
    unsigned int    index;                      // in case of archive files that list the same file multiple times
    uint64_t        mtime;
    uint64_t        inode;

    prl_node_entry() : size(0),type(0),flags(0),index(0),mtime(0),inode(0) { }
    ~prl_node_entry() { }
};

bool prl_node_db_open(void);
void prl_node_db_close(void);
bool prl_node_db_open_ro(void);
bool prl_node_db_add_fsentbyname(prl_node_entry &ent);
bool prl_node_db_lookup_by_node_id(prl_node_entry &ent);
bool prl_node_db_lookup_children_of_parent(std::vector<prl_node_entry> &rlist,prl_node_entry &pent);

#endif /*UTIL_LIB_PRL_NODES_H*/

