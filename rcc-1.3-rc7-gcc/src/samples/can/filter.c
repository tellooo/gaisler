
#include <stdio.h>
#include <stdlib.h>
/*
#define BID 0x3ffc0000
#define EID ((1<<18)-1)
*/
#define BID 0x000007ff
#define EID 0x1ffff800

int main(void)
{

	unsigned int code, mask, id, idmask, result;
		
	code = 0x00400;
	mask = 0xf0400;
	/*
	code = ((code & 0x7ff) << 18) | ((code & 0x1ffff800) >> 11);
  mask = ((mask & 0x7ff) << 18) | ((mask & 0x1ffff800) >> 11);
	*/
	/* STD */
	/*
	id   = 0x344;
	idmask = BID;
	*/
	/*
	id   = 0x744;
	idmask = BID;
	*/
	/* Extended */
	
	id   = 0x8fff400;
	idmask = EID|BID;
	
	id   = 0x8f0f400;
	idmask = EID|BID;
	
	result = (id ^ code) & mask & idmask;
	printf("RESULT: 0x%x, 0x%x, 0x%x\n",result,(id ^ code),(id ^ code) & mask);

	if ( (id ^ code) & mask & idmask ) {
		printf("Message filtered out\n");
	} else {
		printf("Message accepted\n");
	}

	return 0;
}
