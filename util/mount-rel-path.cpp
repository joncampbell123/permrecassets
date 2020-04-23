
#include <assert.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"

using namespace std;

int main(int argc,char **argv) {
    path_rel_label prl;
    procmount_list pml;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    procmount_list_read(pml);
    printf("Mounts:\n");
    for (size_t i=0;i < pml.mounts.size();i++) {
        procmount_entry &ent = pml.mounts[i];
        printf("  device='%s' mountpoint='%s' filesystem='%s' options='%s'\n",
            ent.device.c_str(),             ent.mountpoint.c_str(),
            ent.filesystem.c_str(),         ent.options.c_str());
    }

    if (!path_to_prl(pml,prl,argv[1])) {
        fprintf(stderr,"Unable to map\n");
        return 1;
    }
    printf("Rel of '%s'\n",argv[1]);
    printf("  abs_path:         '%s'\n",prl.abs_path.c_str());
    printf("  fs_label:         '%s'\n",prl.fs_label.c_str());
    printf("  mountpoint:       '%s'\n",prl.mountpoint.c_str());
    printf("  relpath:          '%s'\n",prl.relpath.c_str());
    printf("  device:           '%s'\n",prl.device.c_str());

    return 0;
}

