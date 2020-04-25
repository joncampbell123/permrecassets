
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

