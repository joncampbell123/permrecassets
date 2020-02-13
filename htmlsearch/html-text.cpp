
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <libxml/parser.h>
#include <libxml/HTMLparser.h>

#include <string>

using namespace std;

string              html_path;

int main(int argc,char **argv) {
    struct stat st;

    if (argc < 2) {
        fprintf(stderr,"Specify HTML file\n");
        return 1;
    }

    html_path = argv[1];
    if (stat(html_path.c_str(),&st) != 0) {
        fprintf(stderr,"Cannot stat\n");
        return 1;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr,"Not a file\n");
        return 1;
    }

    return 0;
}

