#include <stdio.h>
#include <bcc/bcc_param.h>

char *myargs[] = { "zero", "one", "two", "three", NULL };
char *some_other_strings[] = { "some", "other", "strings", NULL};

/* Arguments specified at compile/link time. */
int __bcc_argc = ((sizeof myargs) / (sizeof myargs[0])) - 1;
char *((*__bcc_argvp)[]) = &myargs;

int main(int argc, char **argv)
{
        printf("argc=%d\n", argc);
        printf("argv=%p\n", argv);

        for (int i = 0; i < argc; i++) {
                printf("argv[%d] = %p: %s\n", i, argv[i], argv[i]);
        }
        printf("argv[%d] = %p\n", argc, argv[argc]);

        return 0;
}

