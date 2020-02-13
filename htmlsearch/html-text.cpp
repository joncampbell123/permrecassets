
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

class htmlHeader {
public:
    bool            dummy = false;
};

string getTextBODY(htmlHeader &header,xmlNodePtr node,xmlDocPtr doc);

/* node is expected to contain only text */
string getTextGenericHTML(htmlHeader &header,xmlNodePtr node,xmlDocPtr doc) {
    string ret;
    xmlChar *r;

    (void)header;
    (void)doc;

    r = xmlNodeGetContent(node);
    if (r != NULL) {
        ret = (const char*)r;
        xmlFree(r);
    }

    return ret;
}

/* parent node HEAD */
string getTextHTMLParseHeader(htmlHeader &header,xmlNodePtr node,xmlDocPtr doc) {
    string ret;

    (void)header;

    for (;node!=NULL;node=node->next) {
        if (!xmlStrcmp(node->name,(const xmlChar*)"title")) {
            ret += getTextGenericHTML(header,node->children,doc) + "\n";
        }
    }

    return ret;
}

/* parent node UL, node is first UL node child */
string getTextUL(htmlHeader &header,xmlNodePtr node,xmlDocPtr doc) {
    string ret;

    (void)header;
    (void)doc;

    for (;node!=NULL;node=node->next) {
        if (!xmlStrcmp(node->name,(const xmlChar*)"li")) {
            ret += "\t* " + getTextBODY(header,node->children,doc) + "\n";
        }
        else {
            ret += getTextBODY(header,node->children,doc) + "\n";

            fprintf(stderr,"WARNING: Unknown HTML tag '%s'\n",node->name);
        }
    }

    return ret;
}

/* parent node BODY, node is first BODY node child */
string getTextBODY(htmlHeader &header,xmlNodePtr node,xmlDocPtr doc) {
    string ret;

    (void)header;
    (void)doc;

    for (;node!=NULL;node=node->next) {
        if (!xmlStrcmp(node->name,(const xmlChar*)"text")) {
            ret += getTextGenericHTML(header,node,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"p")) {
            ret += "\t" + getTextBODY(header,node->children,doc) + "\n\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"pre")) {
            ret += "\n" + getTextBODY(header,node->children,doc) + "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"code")) {
            ret += "\n" + getTextBODY(header,node->children,doc) + "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"div")) {
            ret += "\n" + getTextBODY(header,node->children,doc) + "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"address")) { /* "Computer Science 314 - The 8237 DMA Controller (2020-02-13 5_41_26 PM).html" nonstd */
            ret += "\n" + getTextBODY(header,node->children,doc) + "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"h1") ||
                 !xmlStrcmp(node->name,(const xmlChar*)"h2") ||
                 !xmlStrcmp(node->name,(const xmlChar*)"h3") ||
                 !xmlStrcmp(node->name,(const xmlChar*)"h4") ||
                 !xmlStrcmp(node->name,(const xmlChar*)"h5")) {
            ret += "\t" + getTextBODY(header,node->children,doc) + "\n\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"br")) {
            ret += "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"hr")) {
            ret += "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"a")) {
            ret += " " + getTextBODY(header,node->children,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"tt")) {
            ret += " " + getTextBODY(header,node->children,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"b")) {
            ret += " " + getTextBODY(header,node->children,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"i")) {
            ret += " " + getTextBODY(header,node->children,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"span")) {
            ret += " " + getTextBODY(header,node->children,doc) + " ";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"ul")) {
            ret += "\n" + getTextUL(header,node->children,doc) + "\n";
        }
        else {
            ret += " " + getTextBODY(header,node->children,doc) + " "; // DEFAULT

            fprintf(stderr,"WARNING: Unknown HTML tag '%s'\n",node->name);
        }
    }

    return ret;
}

/* parent node HTML, node is first HTML node child */
string getTextHTML(xmlNodePtr node,xmlDocPtr doc) {
    htmlHeader header;
    string ret;

    for (;node!=NULL;node=node->next) {
        if (!xmlStrcmp(node->name,(const xmlChar*)"head")) {
            ret += getTextHTMLParseHeader(header,node->children,doc) + "\n";
        }
        else if (!xmlStrcmp(node->name,(const xmlChar*)"body")) {
            ret += getTextBODY(header,node->children,doc) + "\n";
        }
    }

    return ret;
}

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

    xmlDocPtr htmlDoc = htmlParseFile(html_path.c_str(),NULL);
    if (htmlDoc == NULL) {
        fprintf(stderr,"Failed to parse\n");
        return 1;
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(htmlDoc);
    if (rootNode) {
        for (;rootNode!=NULL;rootNode=rootNode->next) {
            if (!xmlStrcmp(rootNode->name,(const xmlChar*)"html")) {
                printf("\n%s\n",getTextHTML(rootNode->children,htmlDoc).c_str());
            }
        }
    }

    xmlFreeDoc(htmlDoc);
    return 0;
}

