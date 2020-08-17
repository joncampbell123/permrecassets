
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_prl_blob.h"
#include "lib_prl_nodes.h"
#include "lib_prl_words.h"

#include <algorithm>

using namespace std;

int main() {
    if (!prl_node_db_open()) {
        fprintf(stderr,"Unable to open SQLite3 DB. Use pra-fs-scan-db-init.sh\n");
        return 1;
    }

    /* build dict */
    {
        prl_file_raw_enum en;
        prl_node_entry node;

        if (en.begin_enum()) {
            while (en.next(node)) {
                std::vector<std::string> dict = prl_filename2dict(node.name);
                printf("%s: %s\n",node.node_id.to_string().c_str(),node.name.c_str());
                for (const auto &s : dict) {
                    std::string n = prl_normalizeword(s);
                    printf("  '%s' => '%s'\n",s.c_str(),n.c_str());
                }
                printf("\n");
            }
            en.end_enum();
        }
    }

    prl_node_db_scan_commit();
    prl_node_db_scan_wal_checkpoint();
    prl_node_db_search_commit();
    prl_node_db_search_wal_checkpoint();
    prl_node_db_close();
    return 0;
}

