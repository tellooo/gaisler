/*
 * Exercise set/get PIL system call
 */
#include <stdio.h>
#include <assert.h>

#include <bcc/bcc.h>

int main(void)
{
        int ret;

	ret = bcc_get_pil();
	printf("bcc_get_pil() => %d\n", ret);
	assert(0 == ret);

	ret = bcc_set_pil(15);
	printf("bcc_set_pil(15) => %d\n", ret);
	assert(0 == ret);

	ret = bcc_set_pil(0);
	printf("bcc_set_pil(0) => %d\n", ret);
	assert(15 == ret);

	ret = bcc_get_pil();
	printf("bcc_get_pil() => %d\n", ret);
	assert(0 == ret);

        return 0;
}

