
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

using namespace std;

bool debug_out = false;

int main() {
    if (!prl_node_db_open()) {
        fprintf(stderr,"Unable to open SQLite3 DB. Use pra-fs-scan-db-init.sh\n");
        return 1;
    }

    /* build dict */
    {
        prl_file_raw_enum en;
        prl_node_entry node;
        unsigned long long kw_count = 0;
        unsigned long long f_count = 0;
        time_t ref = 0,now;

        if (en.begin_enum()) {
            while (en.next(node)) {
                f_count++;

                prl_node_db_search_delete_by_type_and_node(prl_search_dict_id_filename,node.node_id);

                std::vector<std::string> dict = prl_filename2dict(node.name);

                if (debug_out)
                    printf("%s: %s\n",node.node_id.to_string().c_str(),node.name.c_str());

                for (const auto &s : dict) {
                    std::string n = prl_normalizeword(s);

                    if (debug_out)
                        printf("  '%s' => '%s'\n",s.c_str(),n.c_str());

                    kw_count++;
                }
                if (debug_out)
                    printf("\n");

                now = time(NULL);
                if (ref != now) {
                    printf("\x0D" "files=%llu keywords=%llu" "\x1B[K" "\x1B[?7h",f_count,kw_count);
                    fflush(stdout);
                    ref = now;
                }
            }
            en.end_enum();
        }

        printf("\n");
    }

    prl_node_db_scan_commit();
    prl_node_db_scan_wal_checkpoint();
    prl_node_db_search_commit();
    prl_node_db_search_wal_checkpoint();
    prl_node_db_close();
    return 0;
}

