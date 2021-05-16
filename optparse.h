
#pragma once

#include <stdbool.h>
#include <stdlib.h>

/**
*
* sample usage:
*

    int main(int argc, char* argv[])
    ...
        optparser_t prs;

        // for now, optparser focuses on switches, i.e., boolean values.
        bool mything;

        // first, initiate the parser.
        // see the comments for optparse_init for the meaning of the arguments here.
        optparse_init(&prs, argc, argv, 1, 255, 255);

        // now, loop through the args with optparse_parse():
        while(true)
        {
            int c;
            // -1 means either finished parsing, or an error occured.
            // the latter is fairly unlikely, and optparser will internally
            // terminate, since an error here is not expected to be recovered from.
            if((c = optparse_parse(&prs)) == -1)
            {
                break;
            }
            // if c is 0, then it's a positional argument, and not an option!
            if(c != 0)
            {
                switch(c)
                {
                    case 't':
                        mything = true;
                        break;
                    // handling unexpected options in plain C style.
                    default:
                        // do something about an unexpected option
                        break;
                }
            }
        }
        // now, prs.nargc is the count of positional arguments.
        // they start at 0.
        // prs.nargv is the array of positional args.
        for(i=0; i<prs.nargc; i++)
        {
            char* arg = prs.nargv[i];
            ...
        }
        // finally, cleanup.
        optparse_finish(&prs);
    }
*/

typedef struct optparser_t optparser_t;
struct optparser_t
{
    /* used in optparse_parse to track the current index */
    int track;

    /* the most amount of positional args to store */
    int maxargv;

    /* the longest a positional argument can be */
    int maxlen;

    /*  new argc - i.e., how many positional elements are available */
    int nargc;
    
    /* new argv - i.e., positional arguments  */
    char** nargv;

    /* the original argc, as passed to main*/
    int oargc;

    /* the original argv, as passed to main */
    char** oargv;

    /* when encountering "--", then parsing is stopped, and all remainder is just appended to nargv */
    bool stopparsing;

    /* if an error occured, this is the index at which the error occured.*/
    int unparsedidx;
};

/**
* initiate the parser.
* returns true, if initialization was successful, false otherwise.
*
* @param argc
*   the argc param as passed to main().
*
* @param argv
*   the argv param as passed to main().
*
* @param begin
*   the first index to argc for parsing. normally, it should be 1.
*
* @param maxargv
*   how many positional arguments to preallocate?
*   reasonable value is somewhere between 128 - 1024.
*
* @param maxlen
*   the longest a positional argument may be.
*   since this needs to be malloc'd, a reasonable value is 256.
*
*/
static bool optparse_init(optparser_t* prs, int argc, char** argv, int begin, size_t maxargv, size_t maxlen)
{
    prs->nargc = 0;
    prs->track = begin;
    prs->oargc = argc;
    prs->oargv = argv;
    prs->maxargv = maxargv;
    prs->maxlen = maxlen;
    prs->unparsedidx = 0;
    prs->stopparsing = false;
    /* prevent memory read errors by allocating one additional element. */
    prs->nargv = (char**)malloc(prs->maxargv + 1);
    if(prs->nargv == NULL)
    {
        return false;
    }
    return true;
}

/**
* the actual parser.
*
* the return value depends on what it encounters:
*
*   0:      a positional argument, which is appended to nargv.
*   -1:     end of argv, or an error occured. either way, this value marks exiting the loop.
*  1..256:  a option. for example, if argv[n] is "-p", then the return value is 'p' (decimal 112).
*
* if argv[n] is "--", then future parsing is disabled; this is what getopt() (and similar libs) does.
* so, argv[]={"foo", "-p", "bar", "--", "quux", "-f", "-d"} would result in
* optparse_parse() returning {0, 'p', 0, 0, 0, 0, 0, -1}, and
* nargv being {"foo", "bar", "quux", "-f", "-d"}.
* the string "--" is never included, obviously.
*
*
* @param islong
*   will be set to true if the argument is a long option, i.e., "--foo".
*   that's all it does for now; actual long parsing isn't included (yet!)
*/
static int optparse_parse(optparser_t* prs, bool* islong)
{
    char* arg;
    *islong = false;
    if((prs->track >= prs->oargc) || (prs->track >= prs->maxargv))
    {
        prs->unparsedidx = prs->track;
        return -1;
    }
    arg = prs->oargv[prs->track];
    prs->track++;
    if((prs->stopparsing == false) && (arg[0] == '-'))
    {
        if(arg[1] == '-')
        {
            if(arg[2] == 0)
            {
                /* encountered '--', meaning to stop processing arguments */
                prs->stopparsing = true;
                return 0;
            }
            else
            {
                *islong = true;
                fprintf(stderr, "optparse: cannot parse long options yet\n");
                return -1;
            }
        }
        else
        {
            return arg[1];
        }
    }
    else
    {
        prs->nargv[prs->nargc] = (char*)malloc(prs->maxlen + 1);
        memset(prs->nargv[prs->nargc], 0, prs->maxlen);
        memcpy(prs->nargv[prs->nargc], arg, strlen(arg));
        prs->nargc++;
    }
    return 0;
}


/**
* deallocate positional args, and nargv.
*/
static void optparse_finish(optparser_t* prs)
{
    int i;
    for(i=0; i<prs->nargc; i++)
    {
        free(prs->nargv[i]);
        prs->nargv[i] = NULL;
    }
    free(prs->nargv);
}


