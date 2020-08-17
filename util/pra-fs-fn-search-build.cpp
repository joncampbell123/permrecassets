
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

#include <algorithm>

using namespace std;

bool prl_wordbreak(const char c) {
    if ((c > 0 && c < 32) || c == ' ' || c == ',' || c == ';' || c == '\\' || c == '.' || c == '-' || c == '+' || c == '*' || c == '[' || c == ']' || c == '{' || c == '}')
        return true;

    return false;
}

std::vector<std::string> prl_filename2dict(const std::string &spp) {
    std::vector<const char *> wbeg;
    std::vector<std::string> ret;
    const char *s = spp.c_str();
    const char *wordbeg = NULL;

    while (*s != 0) {
        if (prl_wordbreak(*s)) {
            if (wordbeg != NULL) {
                wbeg.push_back(wordbeg);
                wordbeg = NULL;

                for (const auto &sb : wbeg) {
                    if (sb < s)
                        ret.push_back(std::move(std::string(sb,(size_t)(s - sb))));
                }
            }
        }
        else if (*s >= 32 || *s < 0) {
            if (wordbeg == NULL)
                wordbeg = s;
        }

        s++;
    }

    if (wordbeg != NULL) {
        wbeg.push_back(wordbeg);
        wordbeg = NULL;

        for (const auto &sb : wbeg) {
            if (sb < s)
                ret.push_back(std::move(std::string(sb,(size_t)(s - sb))));
        }
    }

    std::sort(ret.begin(),ret.end());
    return ret;
}

std::string prl_normalizeword(const std::string &s) {
    std::string r;
    char pc = 0;

    for (const auto &c : s) {
        if (c > 32 && c < 127 && !prl_wordbreak(c))
            r += tolower(c);
        else if (c < 0)
            r += c; /* allow unicode for now as-is */
        else if (pc > 32 && pc < 127 && !prl_wordbreak(pc))
            r += ' ';

        pc = c;
    }

    return r;
}

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

