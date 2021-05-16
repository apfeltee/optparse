
#pragma once

/*
* copyright 2021-.... github.com/apfeltee
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

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
*       0:   a positional argument, which is appended to nargv.
*      -1:   end of argv, or an error occured. either way, this value marks exiting the loop.
*       1:   a long option. full option will be stored in $fullopt.
*  2..256:   a short option. for example, if argv[n] is "-p", then the return value is 'p' (decimal 112).
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
static int optparse_parse(optparser_t* prs, char** fullopt, char** optval)
{
    /*
    * length was chosen by fair dice roll.
    * guaranteed to theoretically be long enough to contain even unreasonably long options,
    * like those used by LLVM utilities.
    * potentially too short for options like inclusion opts...
    * (-Iz:/dev/foo/bar/baz/quux/.../stuff/things/frobwise/threeth/...)
    */
    enum{ kScratchpadLength = (1024 * 2) };
    int i;
    int vend;
    int slen;
    char* tmp;
    char* arg;
    /*
    * where the string for long options is temporarily stored.
    */
    static char scratchpad[kScratchpadLength + 1];
    *fullopt = NULL;
    *optval = NULL;
    if((prs->track >= prs->oargc) || (prs->track >= prs->maxargv))
    {
        prs->unparsedidx = prs->track;
        return -1;
    }
    arg = prs->oargv[prs->track];
    slen = (int)strlen(arg);
    *fullopt = arg;
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
                /* reset scratchpad */
                memset(scratchpad, 0, kScratchpadLength);
                /* is there a value attached? */
                if((tmp = strchr(arg, '=')) != NULL)
                {
                    /* there's a value, something like "--foo=bar" */
                    for(i=0; i<slen; i++)
                    {
                        if(arg[i] == '=')
                        {
                            vend = i;
                            break;
                        }
                    }
                    /*
                    * the full option is: $lengthOfArgument - ($whereEqualSignAppears + 2)
                    * so, with "--foo=bar":
                    *       $lengthOfArgument=9
                    *       $whereEqualSignAppears=5
                    *
                    */
                    memcpy(scratchpad, arg, slen - (vend + 2));
                    *fullopt = scratchpad;
                    *optval = (arg + (vend + 1));
                }
                return 1;
            }
        }
        else
        {
            /*
            * for short options, the value is just the string minus "-" and the opt char.
            * in cling:
            *   [cling]$ std::cout << ("-I/foo" + 2) << std::endl;
            *   /foo
            */
            if(arg[2] != 0)
            {
                *optval = (arg + 2);
            }
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


