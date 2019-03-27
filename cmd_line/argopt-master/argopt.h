#ifndef __ARGOPT_H__
#define __ARGOPT_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef  __cplusplus

extern "C" {

#endif

#define ARGOPT_OPTION_FOUND (-1)
#define ARGOPT_AMBIGIOUS_OPTION (-2)
#define ARGOPT_UNKNOWN_OPTION (-3)
#define ARGOPT_MISSING_PARAM (-4)
#define ARGOPT_NOT_OPTION (-5)
#define ARGOPT_DONE (-6)

    enum need_param_t {
        NO_PARAM,
        NEED_PARAM,
        OPTIONAL_PARAM
    };

    struct option_t
    {
        const char *name;
        const char *help;
        const enum need_param_t param;
        int *flag;
        int val;
    };

    extern int
    argopt(int argc,
           char *argv[],
           const struct option_t *const options,
           int *index,
           char **parameter);

    extern void
    argopt_help(FILE *output,
                const char *const notice,
                const char *const prog,
                const struct option_t *const options);

    extern void
    argopt_completions(FILE *output,
                       const char *const notice,
                       const char *const opt,
                       const struct option_t *const options);


#ifdef  __cplusplus

}

#endif
#endif /* __ARGOPT_H__ */
// eof

