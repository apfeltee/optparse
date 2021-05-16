

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "optparse.h"

void print_help(const char* arg0)
{
    printf("help for %s goes here\n", arg0);
}

int main(int argc, char** argv)
{
    optparser_t prs;
    int i;
    int c;
    bool islong;
    bool pretty;
    (void)pretty;
    pretty = false;
    optparse_init(&prs, argc, argv, 1, 255, 255);
    while(true)
    {
        if((c = optparse_parse(&prs, &islong)) == -1)
        {
            break;
        }
        if(c != 0)
        {
            switch(c)
            {
                case 'h':
                    print_help(argv[0]);
                    return 0;
                case 'p':
                    printf("will prettyfy\n");
                    pretty = true;
                    break;
                case 'v':
                    printf("verbose mode\n");
                    break;
                default:
                    fprintf(stderr, "unrecognized option '-%c'\n", c);
                    return 1;
                    break;
            }
        }
    }

    printf("positional args:\n");
    for(i=0; i<prs.nargc; i++)
    {
        printf("nargv[%d] = \"%s\"\n", i, prs.nargv[i]);
    }
    optparse_finish(&prs);
    return 0;
}
