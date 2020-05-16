#include <stdlib.h>
#include <stdio.h>

#include <bcc/bcc.h>

#ifndef NODE_INDEX
#define NODE_INDEX 0
#endif
#ifndef AMP_NODES
#define AMP_NODES 1
#endif


#define nodeprintf(...) if (1) { \
        printf("amp%d:%s:%d: ", NODE_INDEX, __func__, __LINE__); \
        printf(__VA_ARGS__); \
}


int main(void)
{
        int count;
        int myid;

        nodeprintf("ENTRY\n");
        count = bcc_get_cpu_count();
        nodeprintf("bcc_get_cpu_count() => %d\n", count);
        nodeprintf("\n");

        nodeprintf("I am supposed to have cpuid=%d\n", NODE_INDEX);
        myid = bcc_get_cpuid();
        nodeprintf("bcc_get_cpuid() => %d\n", myid);
        if (myid != NODE_INDEX) {
                nodeprintf("FAIL: cpuid=%d (expected %d)", myid, NODE_INDEX);
                exit(1);
        }

        if ((myid < (count-1)) && (myid < (AMP_NODES-1))) {
                nodeprintf("I will now call bcc_start_processor(%d)\n", myid + 1);
                nodeprintf("\n");

                bcc_start_processor(myid + 1);
                while (1);
        } else {
                nodeprintf("I am the node with the highest CPUID for this AMP application.\n");
                nodeprintf("\n");
        }

        return 0;
}

