
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"
#include "lib_prluuid.h"
#include "lib_splitpath.h"
#include "lib_prl_blob.h"
#include "lib_prl_nodes.h"

#include <sqlite3.h>

#include <algorithm>

using namespace std;

const prluuid prl_zero_node = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

const std::string prl_search_dict_id_filename = "FN";

static sqlite3* prl_node_db_scan_sqlite = NULL;
static sqlite3* prl_node_db_search_sqlite = NULL;

bool prl_node_db_open_ro(void) {
    if (prl_node_db_scan_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-scan.db",&prl_node_db_scan_sqlite,SQLITE_OPEN_READONLY|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_scan_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    if (prl_node_db_search_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-search.db",&prl_node_db_search_sqlite,SQLITE_OPEN_READONLY|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_search_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    return true;
}

bool prl_node_db_open(void) {
    if (prl_node_db_scan_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-scan.db",&prl_node_db_scan_sqlite,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_scan_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    if (prl_node_db_search_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-search.db",&prl_node_db_search_sqlite,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_search_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    return true;
}

static bool prl_db_scan_in_transaction = false;

bool prl_node_db_scan_begin_transaction(void) {
    if (!prl_db_scan_in_transaction) {
        if (sqlite3_exec(prl_node_db_scan_sqlite,"BEGIN TRANSACTION;",NULL,NULL,NULL) != SQLITE_OK) {
            fprintf(stderr,"BEGIN TRANSACTION failed\n");
            return false;
        }

        prl_db_scan_in_transaction=true;
    }

    return true;
}

bool prl_node_db_scan_commit(void) {
    if (prl_db_scan_in_transaction) {
        prl_db_scan_in_transaction=false;
        if (sqlite3_exec(prl_node_db_scan_sqlite,"COMMIT;",NULL,NULL,NULL) != SQLITE_OK) {
            fprintf(stderr,"COMMIT failed\n");
            return false;
        }
    }

    return true;
}

bool prl_node_db_scan_wal_checkpoint(void) {
    if (sqlite3_exec(prl_node_db_scan_sqlite,"PRAGMA wal_checkpoint;",NULL,NULL,NULL) != SQLITE_OK) {
        fprintf(stderr,"Checkpoint failed\n");
        return false;
    }

    return true;
}

static bool prl_db_search_in_transaction = false;

bool prl_node_db_search_begin_transaction(void) {
    if (!prl_db_search_in_transaction) {
        if (sqlite3_exec(prl_node_db_search_sqlite,"BEGIN TRANSACTION;",NULL,NULL,NULL) != SQLITE_OK) {
            fprintf(stderr,"BEGIN TRANSACTION failed\n");
            return false;
        }

        prl_db_search_in_transaction=true;
    }

    return true;
}

bool prl_node_db_search_commit(void) {
    if (prl_db_search_in_transaction) {
        prl_db_search_in_transaction=false;
        if (sqlite3_exec(prl_node_db_search_sqlite,"COMMIT;",NULL,NULL,NULL) != SQLITE_OK) {
            fprintf(stderr,"COMMIT failed\n");
            return false;
        }
    }

    return true;
}

bool prl_node_db_search_wal_checkpoint(void) {
    if (sqlite3_exec(prl_node_db_search_sqlite,"PRAGMA wal_checkpoint;",NULL,NULL,NULL) != SQLITE_OK) {
        fprintf(stderr,"Checkpoint failed\n");
        return false;
    }

    return true;
}

void prl_node_db_close(void) {
    if (prl_node_db_scan_sqlite != NULL) {
        prl_node_db_scan_commit();
        sqlite3_close_v2(prl_node_db_scan_sqlite);
        prl_node_db_scan_sqlite = NULL;
    }
    if (prl_node_db_search_sqlite != NULL) {
        prl_node_db_search_commit();
        sqlite3_close_v2(prl_node_db_search_sqlite);
        prl_node_db_search_sqlite = NULL;
    }
}

/* in ent.node_id
 * out: array of ent.* */
bool prl_node_db_lookup_file_query(std::vector<prl_node_entry> &rlist,const std::string &query) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int results,sr;

    // disabled. index for real_name and name removed.
    return false;

    rlist.clear();

    /*                                                0           1       2    3         4            5    6    7           8                9     10,   11 */
    if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"SELECT parent_node,node_id,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes WHERE real_name = ? ORDER BY name COLLATE NOCASE ASC;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_add_archive statement prepare failed\n");
        return false;
    }
    results = 0;
    sqlite3_bind_blob(stmt,1,query.c_str(),query.size(),NULL);
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) break;
        else if (sr == SQLITE_ROW) {
            prl_node_entry chent;

            {
                /* parent_node */
                {
                    chent.parent_node = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,0);
                    if (blobsz == sizeof(chent.parent_node.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,0);
                        if (b != NULL) memcpy(chent.parent_node.uuid,b,sizeof(chent.parent_node.uuid));
                    }
                }
                /* node_id */
                {
                    chent.node_id = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,1);
                    if (blobsz == sizeof(chent.node_id.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,1);
                        if (b != NULL) memcpy(chent.node_id.uuid,b,sizeof(chent.node_id.uuid));
                    }
                }
                /* name */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,2);
                    chent.name = (t != NULL) ? (char*)t : "";
                }
                /* real_name */
                {
                    int blobsz = sqlite3_column_bytes(stmt,3);
                    const void *b = sqlite3_column_blob(stmt,3);
                    if (blobsz > 0 && b != NULL) {
                        chent.real_name.resize(blobsz);
                        memcpy(&chent.real_name[0],b,blobsz);
                    }
                    else {
                        chent.real_name.clear();
                    }
                }
                /* name_charset */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,4);
                    chent.name_charset = (t != NULL) ? (char*)t : "";
                }
                /* size */
                chent.size = sqlite3_column_int64(stmt,5);
                /* type */
                chent.type = sqlite3_column_int(stmt,6);
                /* mime_string */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,7);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* content_encoding */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,8);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* flags */
                chent.flags = sqlite3_column_int64(stmt,9);
                /* mtime */
                chent.mtime = sqlite3_column_int64(stmt,10);
                /* inode */
                chent.inode = sqlite3_column_int64(stmt,11);
            }

            rlist.push_back(chent);
            results++;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            return -1;
        }
    } while(1);
    sqlite3_finalize(stmt);

    if (results >= 0)
        return true;

    return false;
}

/* in ent.node_id
 * out: array of ent.* */
bool prl_node_db_lookup_children_of_parent(std::vector<prl_node_entry> &rlist,prl_node_entry &pent) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int results,sr;

    rlist.clear();

    /*                                                0           1    2         3            4    5    6           7                8     9     10 */
    if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"SELECT node_id,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes WHERE parent_node = ? ORDER BY name COLLATE NOCASE ASC;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_add_archive statement prepare failed\n");
        return false;
    }
    results = 0;
    sqlite3_bind_blob(stmt,1,pent.node_id.uuid,sizeof(pent.node_id.uuid),NULL);/*node_id*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) break;
        else if (sr == SQLITE_ROW) {
            prl_node_entry chent;

            {
                /* parent_node */
                chent.parent_node = pent.node_id;
                /* node_id */
                {
                    chent.node_id = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,0);
                    if (blobsz == sizeof(chent.node_id.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,0);
                        if (b != NULL) memcpy(chent.node_id.uuid,b,sizeof(chent.node_id.uuid));
                    }
                }
                /* name */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,1);
                    chent.name = (t != NULL) ? (char*)t : "";
                }
                /* real_name */
                {
                    int blobsz = sqlite3_column_bytes(stmt,2);
                    const void *b = sqlite3_column_blob(stmt,2);
                    if (blobsz > 0 && b != NULL) {
                        chent.real_name.resize(blobsz);
                        memcpy(&chent.real_name[0],b,blobsz);
                    }
                    else {
                        chent.real_name.clear();
                    }
                }
                /* name_charset */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,3);
                    chent.name_charset = (t != NULL) ? (char*)t : "";
                }
                /* size */
                chent.size = sqlite3_column_int64(stmt,4);
                /* type */
                chent.type = sqlite3_column_int(stmt,5);
                /* mime_string */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,6);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* content_encoding */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,7);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* flags */
                chent.flags = sqlite3_column_int64(stmt,8);
                /* mtime */
                chent.mtime = sqlite3_column_int64(stmt,9);
                /* inode */
                chent.inode = sqlite3_column_int64(stmt,10);
            }

            rlist.push_back(chent);
            results++;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            return -1;
        }
    } while(1);
    sqlite3_finalize(stmt);

    if (pent.type == NODE_TYPE_ARCHIVE_COLLECTION || !memcmp(pent.node_id.uuid,prl_zero_node.uuid,sizeof(prl_zero_node.uuid))) {
        /* Some archives take the form JMC YYYY-MM-DD and some JMC-YYYY-MM-DD.
         * To maintain chronological order, re-sort without considering the hyphen, or
         * better yet, matching first the JMC- or J- and then the YYYY-MM-DD. */
        std::sort(rlist.begin(),rlist.end(),prl_archive_sort_func);
    }

    if (results >= 0)
        return true;

    return false;
}

/* in ent.node_id
 * out: ent.* */
bool prl_node_db_lookup_by_node_id(prl_node_entry &ent) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int results,sr;

    /*                                                0           1    2         3            4    5    6           7                8     9     10 */
    if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"SELECT parent_node,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes WHERE node_id = ? LIMIT 1;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_add_archive statement prepare failed\n");
        return false;
    }
    results = 0;
    sqlite3_bind_blob(stmt,1,ent.node_id.uuid,sizeof(ent.node_id.uuid),NULL);/*node_id*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) break;
        else if (sr == SQLITE_ROW) {
            if (results == 0) {
                /* parent_node */
                {
                    ent.parent_node = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,0);
                    if (blobsz == sizeof(ent.parent_node.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,0);
                        if (b != NULL) memcpy(ent.parent_node.uuid,b,sizeof(ent.parent_node.uuid));
                    }
                }
                /* name */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,1);
                    ent.name = (t != NULL) ? (char*)t : "";
                }
                /* real_name */
                {
                    int blobsz = sqlite3_column_bytes(stmt,2);
                    const void *b = sqlite3_column_blob(stmt,2);
                    if (blobsz > 0 && b != NULL) {
                        ent.real_name.resize(blobsz);
                        memcpy(&ent.real_name[0],b,blobsz);
                    }
                    else {
                        ent.real_name.clear();
                    }
                }
                /* name_charset */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,3);
                    ent.name_charset = (t != NULL) ? (char*)t : "";
                }
                /* size */
                ent.size = sqlite3_column_int64(stmt,4);
                /* type */
                ent.type = sqlite3_column_int(stmt,5);
                /* mime_string */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,6);
                    ent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* content_encoding */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,7);
                    ent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* flags */
                ent.flags = sqlite3_column_int64(stmt,8);
                /* mtime */
                ent.mtime = sqlite3_column_int64(stmt,9);
                /* inode */
                ent.inode = sqlite3_column_int64(stmt,10);
            }
            results++;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);

    if (results > 0)
        return true;

    return false;
}

/* in: ent.name, ent.parent_node
 * out: ent.* */
bool prl_node_db_add_fsentbyname(prl_node_entry &ent) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int results,sr;

    if (!ent.assume_not_exist) {
        /* add node with parent_node == parent_node, name = name
         * If already exists, return without changing. */
        /*                                                1                                                                */
        /*                                                                         1                 2                   3 */
        if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"SELECT node_id FROM nodes WHERE real_name = ? AND parent_node = ? AND inode = ? LIMIT 1;",-1,&stmt,&pztail) != SQLITE_OK) {
            fprintf(stderr,"db_add_archive statement prepare failed\n");
            return false;
        }
        results = 0;
        sqlite3_bind_blob(stmt,1,&ent.real_name[0],ent.real_name.size(),NULL);/*name*/
        sqlite3_bind_blob(stmt,2,ent.parent_node.uuid,sizeof(ent.parent_node.uuid),NULL);/*parent_node*/
        sqlite3_bind_int64(stmt,3,ent.inode);/*inode*/
        do {
            sr = sqlite3_step(stmt);
            if (sr == SQLITE_BUSY) continue;
            else if (sr == SQLITE_DONE) break;
            else if (sr == SQLITE_ROW) {
                if (results == 0) {
                    int blobsz = sqlite3_column_bytes(stmt,0);
                    if (blobsz == sizeof(ent.node_id.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,0);
                        if (b != NULL) memcpy(ent.node_id.uuid,b,sizeof(ent.node_id.uuid));
                    }
                }
                results++;
            }
            else {
                fprintf(stderr,"SQLITE statement error %d\n",sr);
                break;
            }
        } while(1);
        sqlite3_finalize(stmt);

        if (results > 0) {
            /* already there, nothing to do. The code updated node_id */
            return true;
        }
    }

    ent.this_node_did_not_exist = true; /* therefore when inserting child nodes, assume the child nodes do not exist */
    prluuidgen(ent.node_id);

    /*                                                            1       2           3    4         5            6    7    8     9             1 2 3 4 5 6 7 8 9 */
    if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"INSERT INTO nodes (node_id,parent_node,name,real_name,name_charset,size,type,mtime,inode) VALUES(?,?,?,?,?,?,?,?,?);",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_add_archive insert statement prepare failed\n");
        return false;
    }
    results = 0;
    sqlite3_bind_blob(stmt,1,ent.node_id.uuid,sizeof(ent.node_id.uuid),NULL);/*node_id*/
    sqlite3_bind_blob(stmt,2,ent.parent_node.uuid,sizeof(ent.parent_node.uuid),NULL);/*parent_node*/
    sqlite3_bind_text(stmt,3,ent.name.c_str(),-1,NULL);/*name*/
    sqlite3_bind_blob(stmt,4,&ent.real_name[0],ent.real_name.size(),NULL);/*real_name*/
    sqlite3_bind_text(stmt,5,ent.name_charset.c_str(),-1,NULL);/*name_charset*/
    sqlite3_bind_int64(stmt,6,ent.size);/*size*/
    sqlite3_bind_int(stmt,7,ent.type);/*type*/
    sqlite3_bind_int64(stmt,8,ent.mtime);/*mtime*/
    sqlite3_bind_int64(stmt,9,ent.inode);/*inode*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) {
            results++;
            break;
        }
        else if (sr == SQLITE_CONSTRAINT) {
            /* whoops, guess it already existed */
            fprintf(stderr,"SQLITE warning: SQLITE_CONSTRAINT means the node already does exist. Assumption failed.\n");
            results++;
            break;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);
    if (results == 0) return false;

    return true;
}

bool prl_node_db_lookup_node_tree_path(std::vector<prl_node_entry> &plist,prl_node_entry &pent) {
    plist.clear();

    std::vector<prl_node_entry> tmp;
    prl_node_entry search = pent;

    while (memcmp(search.node_id.uuid,prl_zero_node.uuid,sizeof(prl_zero_node.uuid)) != 0) {
        tmp.push_back(search);

        search.node_id = search.parent_node;
        if (!prl_node_db_lookup_by_node_id(search)) break;
    }

    for (std::vector<prl_node_entry>::reverse_iterator i=tmp.rbegin();i!=tmp.rend();i++)
        plist.push_back(*i);

    return true;
}

std::string prl_archive_sort_func_filter(const string &s) {
    string r = s;

    if (r.length() >= 4) {
        if (r[0] == 'J' && r[1] == 'M' && r[2] == 'C') {
            if (r[3] == '-') r[3] = ' ';
        }
    }

    return r;
}

bool prl_archive_sort_func(const prl_node_entry &a,const prl_node_entry &b) {
    /* 'JMC-' or 'JMC ' need to be treated the same */
    string sa = prl_archive_sort_func_filter(a.name);
    string sb = prl_archive_sort_func_filter(b.name);
    return sa < sb;
}

bool prl_file_raw_enum::begin_enum() {
    if (stmt == NULL) {
        const char* pztail = NULL;

        /*                                                     0       1           2    3         4            5    6    7           8                9     10    11 */
        if (sqlite3_prepare_v2(prl_node_db_scan_sqlite,"SELECT node_id,parent_node,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes;",-1,&stmt,&pztail) != SQLITE_OK) {
            fprintf(stderr,"begin_enum statement prepare failed\n");
            return false;
        }
    }

    return true;
}

bool prl_file_raw_enum::next(struct prl_node_entry &prl) {
    int sr;

    if (stmt != NULL) {
        do {
            sr = sqlite3_step(stmt);
            if (sr == SQLITE_BUSY) continue;
            else if (sr == SQLITE_DONE) break;
            else if (sr == SQLITE_ROW) {
                prl_node_entry chent;

                /* node_id */
                {
                    chent.node_id = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,0);
                    if (blobsz == sizeof(chent.node_id.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,0);
                        if (b != NULL) memcpy(chent.node_id.uuid,b,sizeof(chent.node_id.uuid));
                    }
                }
                /* parent_node */
                {
                    chent.parent_node = prl_zero_node;
                    int blobsz = sqlite3_column_bytes(stmt,1);
                    if (blobsz == sizeof(chent.parent_node.uuid)) {
                        const void *b = sqlite3_column_blob(stmt,1);
                        if (b != NULL) memcpy(chent.parent_node.uuid,b,sizeof(chent.parent_node.uuid));
                    }
                }
                /* name */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,2);
                    chent.name = (t != NULL) ? (char*)t : "";
                }
                /* real_name */
                {
                    int blobsz = sqlite3_column_bytes(stmt,3);
                    const void *b = sqlite3_column_blob(stmt,3);
                    if (blobsz > 0 && b != NULL) {
                        chent.real_name.resize(blobsz);
                        memcpy(&chent.real_name[0],b,blobsz);
                    }
                    else {
                        chent.real_name.clear();
                    }
                }
                /* name_charset */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,4);
                    chent.name_charset = (t != NULL) ? (char*)t : "";
                }
                /* size */
                chent.size = sqlite3_column_int64(stmt,5);
                /* type */
                chent.type = sqlite3_column_int(stmt,6);
                /* mime_string */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,7);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* content_encoding */
                {
                    const unsigned char *t = sqlite3_column_text(stmt,8);
                    chent.mime_string = (t != NULL) ? (char*)t : "";
                }
                /* flags */
                chent.flags = sqlite3_column_int64(stmt,9);
                /* mtime */
                chent.mtime = sqlite3_column_int64(stmt,10);
                /* inode */
                chent.inode = sqlite3_column_int64(stmt,11);

                prl = chent;
                return true;
            }
            else {
                fprintf(stderr,"SQLITE statement error %d\n",sr);
                break;
            }
        } while(1);
    }

    return false;
}

void prl_file_raw_enum::end_enum() {
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
}

bool prl_node_db_search_delete_by_type(const std::string &mtype) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int sr;

    if (sqlite3_prepare_v2(prl_node_db_search_sqlite,"DELETE from dict WHERE mtype = ?;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_search_delete_type insert statement prepare failed\n");
        return false;
    }
    sqlite3_bind_text(stmt,1,mtype.c_str(),-1,NULL);/*mtype*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) {
            break;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);

    return true;
}

bool prl_node_db_search_delete_by_type_and_node(const std::string &mtype,const prluuid &node_id) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int sr;

    if (sqlite3_prepare_v2(prl_node_db_search_sqlite,"DELETE from dict WHERE mtype = ? and node_id = ?;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_search_delete_type insert statement prepare failed\n");
        return false;
    }
    sqlite3_bind_text(stmt,1,mtype.c_str(),-1,NULL);/*mtype*/
    sqlite3_bind_blob(stmt,2,node_id.uuid,sizeof(node_id.uuid),NULL);/*node_id*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) {
            break;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);

    return true;
}

bool prl_node_db_search_insert_type_and_node(const std::string &mtype,const prluuid &node_id,const std::string &word) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    time_t now = time(NULL);
    int sr;

    if (sqlite3_prepare_v2(prl_node_db_search_sqlite,"INSERT INTO dict (mtype,node_id,word,mtime) VALUES(?,?,?,?);",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_search_delete_type insert statement prepare failed\n");
        return false;
    }
    sqlite3_bind_text(stmt,1,mtype.c_str(),-1,NULL);/*mtype*/
    sqlite3_bind_blob(stmt,2,node_id.uuid,sizeof(node_id.uuid),NULL);/*node_id*/
    sqlite3_bind_text(stmt,3,word.c_str(),-1,NULL);/*word*/
    sqlite3_bind_int64(stmt,4,now);/*mtime*/
    do {
        sr = sqlite3_step(stmt);
        if (sr == SQLITE_BUSY) continue;
        else if (sr == SQLITE_DONE) {
            break;
        }
        else {
            fprintf(stderr,"SQLITE statement error %d\n",sr);
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);

    return true;
}

