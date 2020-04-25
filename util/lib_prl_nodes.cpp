
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

using namespace std;

const prluuid prl_zero_node = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

static sqlite3* prl_node_db_sqlite = NULL;

bool prl_node_db_open_ro(void) {
    if (prl_node_db_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-scan.db",&prl_node_db_sqlite,SQLITE_OPEN_READONLY|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    return true;
}

bool prl_node_db_open(void) {
    if (prl_node_db_sqlite == NULL) {
        if (sqlite3_open_v2("pra-fs-scan.db",&prl_node_db_sqlite,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX/*|SQLITE_OPEN_NOFOLLOW*/,NULL) != SQLITE_OK)
            return false;

        if (sqlite3_exec(prl_node_db_sqlite,"PRAGMA synchronous = 0;",NULL,NULL,NULL) != SQLITE_OK)
            fprintf(stderr,"PRAGMA synchronous failed\n");
    }

    return true;
}

void prl_node_db_close(void) {
    if (prl_node_db_sqlite != NULL) {
        sqlite3_close_v2(prl_node_db_sqlite);
        prl_node_db_sqlite = NULL;
    }
}

/* in ent.node_id
 * out: array of ent.* */
bool prl_node_db_lookup_children_of_parent(std::vector<prl_node_entry> &rlist,prl_node_entry &pent) {
    sqlite3_stmt* stmt = NULL;
    const char* pztail = NULL;
    int results,sr;

    rlist.clear();

    /*                                                0           1    2         3            4    5    6           7                8     9     10 */
    if (sqlite3_prepare_v2(prl_node_db_sqlite,"SELECT node_id,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes WHERE parent_node = ? ORDER BY name ASC;",-1,&stmt,&pztail) != SQLITE_OK) {
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
            fprintf(stderr,"SQLITE statement error\n");
            return -1;
        }
    } while(1);
    sqlite3_finalize(stmt);

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
    if (sqlite3_prepare_v2(prl_node_db_sqlite,"SELECT parent_node,name,real_name,name_charset,size,type,mime_string,content_encoding,flags,mtime,inode FROM nodes WHERE node_id = ? LIMIT 1;",-1,&stmt,&pztail) != SQLITE_OK) {
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
            fprintf(stderr,"SQLITE statement error\n");
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

    /* add node with parent_node == parent_node, name = name
     * If already exists, return without changing. */
    /*                                                1                                                               */
    /*                                                                             1                   2            3 */
    if (sqlite3_prepare_v2(prl_node_db_sqlite,"SELECT node_id FROM nodes WHERE real_name = ? AND parent_node = ? AND type = ? LIMIT 1;",-1,&stmt,&pztail) != SQLITE_OK) {
        fprintf(stderr,"db_add_archive statement prepare failed\n");
        return false;
    }
    results = 0;
    sqlite3_bind_blob(stmt,1,&ent.real_name[0],ent.real_name.size(),NULL);/*name*/
    sqlite3_bind_blob(stmt,2,ent.parent_node.uuid,sizeof(ent.parent_node.uuid),NULL);/*parent_node*/
    sqlite3_bind_int(stmt,3,ent.type);/*type*/
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
            fprintf(stderr,"SQLITE statement error\n");
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);

    if (results > 0) {
        /* already there, nothing to do. The code updated node_id */
        return true;
    }

    prluuidgen(ent.node_id);

    /*                                                            1       2           3    4         5            6    7    8     9             1 2 3 4 5 6 7 8 9 */
    if (sqlite3_prepare_v2(prl_node_db_sqlite,"INSERT INTO nodes (node_id,parent_node,name,real_name,name_charset,size,type,mtime,inode) VALUES(?,?,?,?,?,?,?,?,?);",-1,&stmt,&pztail) != SQLITE_OK) {
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
        else {
            fprintf(stderr,"SQLITE statement error\n");
            break;
        }
    } while(1);
    sqlite3_finalize(stmt);
    if (results == 0) return false;

    return true;
}

