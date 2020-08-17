
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

    bool            assume_not_exist;           // set this flag if the parent was created (did not exist) because it means the child node (this) cannot exist
    bool            this_node_did_not_exist;    // on adding the node, it did not exist, was created, therefore child nodes cannot exist (set assume_not_exist=true)

    prl_node_entry() : size(0),type(0),flags(0),index(0),mtime(0),inode(0),assume_not_exist(false),this_node_did_not_exist(false) { }
    ~prl_node_entry() { }
};

bool prl_node_db_open(void);
void prl_node_db_close(void);
bool prl_node_db_open_ro(void);
bool prl_node_db_add_fsentbyname(prl_node_entry &ent);
bool prl_node_db_lookup_by_node_id(prl_node_entry &ent);
bool prl_node_db_lookup_file_query(std::vector<prl_node_entry> &rlist,const std::string &query);
bool prl_node_db_lookup_node_tree_path(std::vector<prl_node_entry> &plist,prl_node_entry &pent);
bool prl_node_db_lookup_children_of_parent(std::vector<prl_node_entry> &rlist,prl_node_entry &pent);

bool prl_node_db_search_begin_transaction(void);
bool prl_node_db_search_wal_checkpoint(void);
bool prl_node_db_search_commit(void);

bool prl_node_db_scan_begin_transaction(void);
bool prl_node_db_scan_wal_checkpoint(void);
bool prl_node_db_scan_commit(void);

std::string prl_archive_sort_func_filter(const std::string &s);
bool prl_archive_sort_func(const prl_node_entry &a,const prl_node_entry &b);

#endif /*UTIL_LIB_PRL_NODES_H*/

