
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

static bool debug_out = false;

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

                if (debug_out) {
                    for (const auto &s : dict)
                        printf("  step1: '%s'\n",s.c_str());
                }

                for (auto &s : dict)
                    s = prl_normalizeword(s);

                std::sort(dict.begin(),dict.end());
                {
                    auto it = std::unique(dict.begin(),dict.end());
                    if (it != dict.end()) dict.erase(it,dict.end());
                }

                for (const auto &s : dict) {
                    if (debug_out)
                        printf("  step2: '%s'\n",s.c_str());

                    prl_node_db_search_insert_type_and_node(prl_search_dict_id_filename,node.node_id,s);
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

