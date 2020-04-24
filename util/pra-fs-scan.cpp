
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"
#include "lib_prluuid.h"
#include "lib_splitpath.h"

using namespace std;

const prluuid prl_zero_node = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

// TODO: Just make this code C++11 so we can use "using" instead of this mess
typedef std::vector<uint8_t> prl_blob_base;

class prl_blob : public prl_blob_base {
public:
    prl_blob() : prl_blob_base() { }
    prl_blob(const std::string &x) : prl_blob_base() {
        resize(x.size());
        for (size_t i=0;i < x.size();i++) prl_blob_base::operator[](i) = x[i]; /* Not the NUL at the end, though */
    }
    ~prl_blob() { }
    std::string string_value(void) const {
        return std::string((char*)(&prl_blob_base::operator[](0)),prl_blob_base::size());
    }
};

enum prl_node_type {
    NODE_TYPE_NULL=0,                           // 0
    NODE_TYPE_FILE,                             // 1
    NODE_TYPE_DIRECTORY,                        // 2
    NODE_TYPE_ARCHIVE,                          // 3
    NODE_TYPE_ARCHIVE_COLLECTION,               // 4
    NODE_TYPE_VIEW,                             // 5
    NODE_TYPE_SYMLINK,                          // 6

    NODE_TYPE_MAX
};

enum prl_node_flags {
    NODE_FLAG_PRIVATE=(1 << 0),                 // private node, do not show publicly
    NODE_FLAG_USER_MODIFIED_CHARSET=(1 << 1)    // user modified charset, do not auto-modify later
};

/* instead of passing a million parameters for each part, pass a struct instead.
 * addition of fields is just add a new struct member and recompile away. */
struct prl_node_entry {
    prluuid         node_id;
    prluuid         parent_node;
    std::string     name;
    prl_blob        real_name;
    std::string     name_charset;
    uint64_t        size;
    unsigned int    type;
    std::string     mime_string;
    std::string     content_encoding;
    unsigned int    flags;

    prl_node_entry() : size(0),type(0),flags(0) { }
    ~prl_node_entry() { }
};

/* in: name
 * out: ent */
bool prl_node_db_add_archive(prl_node_entry &ent,const std::string &name) {
    /* add node with parent_node == zero_node, name = name, type = ARCHIVE.
     * If already exists, return without changing. */
    return true;
}

/* in: ent.name, ent.parent_node
 * out: ent.* */
bool prl_node_db_add_fsentbyname(prl_node_entry &ent) {
    /* add node with parent_node == parent_node, name = name
     * If already exists, return without changing. */
    return true;
}

int main(int argc,char **argv) {
    path_rel_label prl;
    procmount_list pml;
    struct stat st;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    /* the given location must be a directory. not a file. not a symlink. */
    if (lstat(argv[1],&st)) {
        perror("Cannot stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        perror("Not a directory");
        return 1;
    }

    procmount_list_read(pml);
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

    if (prl.fs_label.empty()) {
        fprintf(stderr,"Filesystem must have a label\n");
        return 1;
    }

    prl_node_entry parent_node;

    if (!prl_node_db_add_archive(/*&return*/parent_node,prl.fs_label)) {
        fprintf(stderr,"Failed to add or update archive node\n");
        return 1;
    }

    /* relative path */
    {
        vector<string> path;
        prl_path_split(path,prl.relpath);
        for (vector<string>::iterator i=path.begin();i!=path.end();i++) {
            prl_node_entry child_node;

            child_node.type = NODE_TYPE_DIRECTORY;/*remember the above code MAKES SURE the path given is a directory*/
            child_node.parent_node = parent_node.node_id;
            child_node.real_name = (*i);
            child_node.name = (*i);

            if (!prl_node_db_add_fsentbyname(/*in&out*/child_node)) {
                fprintf(stderr,"Failed to add or update fs node\n");
                return 1;
            }
        }
    }

    return 0;
}

