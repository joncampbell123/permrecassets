
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <termios.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"
#include "lib_prluuid.h"
#include "lib_splitpath.h"
#include "lib_prl_blob.h"
#include "lib_prl_nodes.h"
#include "lib_cpp_realpath.h"
#include "lib_fs_isrdonly.h"

using namespace std;

static prl_node_entry parent_node;

char read_char(void) {
	char c = 0;

	read(0/*STDIN*/,&c,1);

	return c;
}

std::string read_in(void) {
	std::string ret;
	char c;

	c = read_char();
	ret += c;

	if (c == 27) {
		c = read_char();
		ret += c;

		if (c == '[') {
			do {
				c = read_char();
				ret += c;
			} while(isdigit(c));
		}
	}

	return ret;
}

std::string file_size_human_friendly(uint64_t sz) {
    const char *suffix = "B";
    uint64_t frac = 0;
    char tmp[128];

    if (sz >= (uint64_t)1024ull) {
        frac = sz & (uint64_t)1023ull;
        sz >>= (uint64_t)10ull;
        suffix = "KB";
    }

    if (sz >= (uint64_t)1024ull) {
        frac = sz & (uint64_t)1023ull;
        sz >>= (uint64_t)10ull;
        suffix = "MB";
    }

    if (sz >= (uint64_t)1024ull) {
        frac = sz & (uint64_t)1023ull;
        sz >>= (uint64_t)10ull;
        suffix = "GB";
    }

    sprintf(tmp,"%llu.%llu",(unsigned long long)sz,(frac * 1000ull) / 1024ull);
    return std::string(tmp) + suffix;
}

void editorLoop(void) {
    int sheight = 25;
    int swidth = 80;
	unsigned char run = 1;
	unsigned char redraw = 1;
    int select = 0;
    int scroll = 0;
	struct termios omode,mode;
    std::vector<prl_node_entry> rlist;
    std::string key;

    if (!prl_node_db_open_ro()) {
        fprintf(stderr,"Unable to open SQLite3 DB. Use pra-fs-scan-db-init.sh\n");
        return;
    }

    parent_node = prl_node_entry();
    parent_node.node_id = prl_zero_node;
    prl_node_db_lookup_by_node_id(parent_node);
    prl_node_db_lookup_children_of_parent(/*&r*/rlist,parent_node);

	tcgetattr(0/*STDIN*/,&omode);
	mode = omode;
	mode.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL);
	tcsetattr(0/*STDIN*/,TCSANOW,&mode);

	while (run) {
		if (redraw) {
            /* erase screen, home cursor */
            printf("\x1B[0m");
            printf("\x1B[2J" "\x1B[H"); fflush(stdout);

            printf("\x1B[1;1H" "Parent node %s: '%s' (%zu results)\n",
                parent_node.node_id.to_string().c_str(),
                parent_node.name.c_str(),
                rlist.size());

            for (int s=0;(s+2) < sheight;s++) {
                if ((size_t)(s+scroll) < rlist.size()) {
                    const char *typ = "?";
                    const prl_node_entry &ent = rlist[s+scroll];

                    switch (ent.type) {
                        case NODE_TYPE_FILE:        typ = "File"; break;
                        case NODE_TYPE_DIRECTORY:   typ = "Dir"; break;
                        case NODE_TYPE_ARCHIVE:     typ = "Archive"; break;
                        case NODE_TYPE_VIEW:        typ = "View"; break;
                        default:                    typ = "?"; break;
                    };

                    if ((s+scroll) == select)
                        printf("\x1B[7m");
                    else
                        printf("\x1B[0m");

                    printf("\x1B[%d;1H" "%s: '%s' (%s)",s+1+2,ent.node_id.to_string().c_str(),ent.name.c_str(),typ);
                    if (ent.type == NODE_TYPE_FILE) printf(" %s",file_size_human_friendly(ent.size).c_str());
                    printf("\x1B[0K");
                    fflush(stdout);
                }
            }

            printf("\x1B[0m");
            fflush(stdout);

			/* done */
			redraw = 0;
		}

		key = read_in();
		if (key.empty()) break;
		if (key == "\x1B" || key == "\x1B\x1B") break;

		if (key == "\x1B[A") { /* up arrow */
            if (select > 0) {
                select--;
                redraw = 1;
            }
		}
		else if (key == "\x1B[B") { /* down arrow */
            if (!rlist.empty() && (size_t)(select+1) < rlist.size()) {
                select++;
                redraw = 1;
            }
		}
		else if (key == "\x1B[D") { /* left arrow */
		}
		else if (key == "\x1B[C") { /* right arrow */
		}
        else if (key == "\x0D" || key == "\x0A") { /* enter */
            if (select >= 0 && (size_t)select < rlist.size()) {
                parent_node = rlist[select];
                prl_node_db_lookup_by_node_id(parent_node);
                prl_node_db_lookup_children_of_parent(/*&r*/rlist,parent_node);
                select = 0;
                redraw = 1;
            }
        }
        else if (key == "\x08" || key == "\x7F") { /* backspace */
            parent_node.node_id = parent_node.parent_node;
            prl_node_db_lookup_by_node_id(parent_node);
            prl_node_db_lookup_children_of_parent(/*&r*/rlist,parent_node);
            select = 0;
            redraw = 1;
        }
	}

	tcsetattr(0/*STDIN*/,TCSANOW,&omode);

	/* erase screen, home cursor */
	printf("\x1B[2J" "\x1B[H"); fflush(stdout);

    prl_node_db_close();
}

int main(int argc,char **argv) {
	editorLoop();
	return 0;
}

