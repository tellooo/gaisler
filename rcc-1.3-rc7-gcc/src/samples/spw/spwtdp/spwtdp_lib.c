#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "spwtdp_lib.h"

#undef DEBUG

/* Return number of coarse and fine precision bytes */
int spwtdp_precision_parse(spwtdp_time_t * a, int *coarse, int *fine)
{
	int coarse_precision, fine_precision;
    unsigned int preamble;

    if (a==NULL)
        return -1;

    preamble=a->preamble;
	if (preamble & 0x80) {
		printf("Pfield second extension set: unknown format: 0x%04x\n", preamble);
		return -1;
	}
	if (!((preamble & 0x7000) == 0x2000 || (preamble & 0x7000) == 0x1000)) {
		printf(" PField indicates not unsegmented code: unknown format: 0x%04x\n", preamble);
		return -1;
	}
	/*
	coarse_precision = 32;
	fine_precision = 24;
	*/
	coarse_precision = ((preamble >> 10) & 0x3) + 1;
	if (preamble & 0x80)
		coarse_precision += (preamble >> 5) & 0x3;
	fine_precision = (preamble >> 8) & 0x3;
	if (preamble & 0x80)
		fine_precision += (preamble >> 2) & 0x7;
	if (coarse)
		*coarse = coarse_precision;
	if (fine)
		*fine = fine_precision;
	return 0;
}

/* Return number of coarse and fine precision bytes */
int spwtdp_et_print(spwtdp_time_t * a, const char * msg)
{
	int ccnt, fcnt;

	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -1;

	printf("%s:(C:%d,F:%d)  0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
            msg,
            ccnt, fcnt,
			a->data[0], a->data[1],
			a->data[2], a->data[3],
			a->data[4], a->data[5],
			a->data[6], a->data[7],
			a->data[8]);
	return 0;
}


/* Subtract two Elapsed Time codes:
 * this is implemented under the assumption that a>b
 * r = a - b
 */
int spwtdp_et_sub(
    spwtdp_time_t *a,
    spwtdp_time_t *b,
    spwtdp_time_t *r)
{
	int i, oldrem, rem, ccnt, fcnt;

    if ((a==NULL)||(b==NULL)||(r==NULL))
        return -1;

    if (a->preamble!=b->preamble)
        return -1;

#ifdef DEBUG
    spwtdp_et_print(a, "A");
    spwtdp_et_print(b, "B");
#endif
	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -1;

	rem = 0;
	oldrem=0;
	for (i = ccnt + fcnt - 1; i > 0; i--) {
		oldrem = rem;

		if ((a->data[i] - oldrem) < b->data[i])
			rem = 1;
		else
			rem = 0;

		r->data[i] = (a->data[i] - oldrem + (rem << 8)) - b->data[i];
	}

	if ((a->data[i] - oldrem + (rem << 8)) < b->data[i]) {
		printf("sub_et: error input: %d\n",
		       (a->data[i] - oldrem + (rem << 8)) < b->data[i]);
		return -1; /* input error: a>b was not true */
	}
    r->preamble=a->preamble;
#ifdef DEBUG
    spwtdp_et_print(r,"R");
#endif

	return 0;
}

/* Add two Elapsed Time codes:
 * this is implemented under the assumption: a+b < "what preamble can hold"
 */
int spwtdp_et_add(
    spwtdp_time_t *a,
    spwtdp_time_t *b,
    spwtdp_time_t *r)
{
	int i, oldrem, rem, ccnt, fcnt;
	unsigned short tmp;

    if ((a==NULL)||(b==NULL)||(r==NULL))
        return -1;

    if (a->preamble!=b->preamble)
        return -1;

#ifdef DEBUG
    spwtdp_et_print(a, "A");
    spwtdp_et_print(b, "B");
#endif
	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -1;

	rem = 0;
	for (i = ccnt + fcnt - 1; i > fcnt; i--) {
		oldrem = rem;

		tmp = (unsigned short)a->data[i] + (unsigned short)b->data[i] + oldrem;

		if (tmp > 0xff)
			rem = 1;
		else
			rem = 0;

		r->data[i] = tmp & 0xff;
	}
    r->preamble=a->preamble;
#ifdef DEBUG
    spwtdp_et_print(r,"R");
#endif

	return 0;
}

/* Compare two Elapsed Time codes:
 * returns -1 if b < a
 * returns 1 if b > a
 * returns 0 if b == a
 */
int spwtdp_et_cmp(
    spwtdp_time_t *a,
    spwtdp_time_t *b)	
{
	int i, ccnt, fcnt;

    if ((a==NULL)||(b==NULL))
        return -1;

#ifdef DEBUG
    spwtdp_et_print(a, "A");
    spwtdp_et_print(b, "B");
#endif
	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -2;

	for (i = 0; i < ccnt+fcnt; i++) {
		if (b->data[i] < a->data[i])
			return -1;
		else if (b->data[i] > a->data[i])
			return 1;
	}

	return 0;
}

/* Convert the Elapse Time into two unsigned 64-bit integers, couse and
 * fine time according to the PREAMBLE field format.
 * Retuns zero on success and negative on failure.
 */
int spwtdp_et_to_uint(spwtdp_time_t *a, unsigned long long *coarse, unsigned long long *fine)
{
	int i, ccnt, fcnt;

	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -1;

	if (ccnt > 8)
		return -1;
	if (fcnt > 8)
		return -1;

	if (coarse) {
		*coarse = 0;
		for (i = 0; i < ccnt; i++)
			*coarse |= a->data[i] << ((ccnt-1) - i)*8;
	}
	if (fine) {
		*fine = 0;
		for (i = ccnt; i < ccnt+fcnt; i++)
			*fine |= a->data[i] << ((fcnt-1) - (i-ccnt))*8;
	}

	return 0;
}

/* Return only the fine/coarse time of the ET, parse PREAMBLE field to know format. */
int spwtdp_et_split(spwtdp_time_t *a, uint8_t *coarse, uint8_t *fine)
{
	int i, ccnt, fcnt;

	if (spwtdp_precision_parse(a, &ccnt, &fcnt))
		return -1;

	if (coarse) {
		for (i = 0; i < ccnt; i++)
			coarse[i] = a->data[i];
	}
	if (fine) {
		for (i = ccnt; i < ccnt+fcnt; i++)
			fine[i-ccnt] = a->data[i];
	}

	return 0;
}

