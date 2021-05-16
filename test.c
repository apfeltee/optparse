

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "optparse.h"

/*
 ./a.out foo bar -I/usr/include quux --load=foo.dll -Ic:/blah/include dorks --load
include: </usr/include>
loading <foo.dll>
include: <c:/blah/include>
error: --load needs an argument
positional args:
nargv[0] = "foo"
nargv[1] = "bar"
nargv[2] = "quux"
nargv[3] = "dorks"

===========================

./a.out foo bar -I/usr/include quux -- --load=foo.dll -Ic:/blah/include dorks --load
include: </usr/include>
positional args:
nargv[0] = "foo"
nargv[1] = "bar"
nargv[2] = "quux"
nargv[3] = "--load=foo.dll"
nargv[4] = "-Ic:/blah/include"
nargv[5] = "dorks"
nargv[6] = "--load"

*/

void print_help(const char* arg0)
{
    printf("help for %s goes here\n", arg0);
}

int main(int argc, char** argv)
{
    optparser_t prs;
    int i;
    int c;
    char* fopt;
    char* fval;
    bool pretty;
    (void)pretty;
    pretty = false;
    optparse_init(&prs, argc, argv, 1, 255, 255);
    while(true)
    {
        if((c = optparse_parse(&prs, &fopt, &fval)) == -1)
        {
            break;
        }
        if(c != 0)
        {
            switch(c)
            {
                /* handle long options: */
                case 1:
                    {
                        if(strcmp(fopt, "--help") == 0)
                        {
                            print_help(argv[0]);
                            return 0;
                        }
                        else if(strcmp(fopt, "--load") == 0)
                        {
                            if(fval != NULL)
                            {
                                printf("loading <%s>\n", fval);
                            }
                            else
                            {
                                printf("error: --load needs an argument\n");
                            }
                        }
                        else
                        {
                            printf("error: unrecognized long option '%s'\n", fopt);
                        }
                    }
                    break;
                /* handle short options */
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
                case 'I':
                    if(fval != NULL)
                    {
                        printf("include: <%s>\n", fval);
                    }
                    else
                    {
                        printf("error: option '-I' needs a value\n");
                    }
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
