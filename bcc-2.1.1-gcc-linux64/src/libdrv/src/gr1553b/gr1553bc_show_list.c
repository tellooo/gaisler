/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*  GR1553B BC driver
 *
 * OVERVIEW
 * ========
 *  See header file and list header file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drv/gr1553b.h>
#include <drv/gr1553bc.h>
#include <drv/gr1553bc_priv.h>

/*** DEBUGGING HELP FUNCTIONS ***/

void gr1553bc_show_list(struct gr1553bc_list *list, int options)
{
	struct gr1553bc_major *major;
	struct gr1553bc_minor *minor;
	int i, j, minor_cnt, timefree;

	(void)options; /* Remove compiler warning (-Wunused-variable) */

	printf("LIST\n");
	printf("  major cnt: %d\n", list->major_cnt);
	for (i=0; i<32; i++) {
		printf("  RT[%d] timeout: %d\n", i, 14+(list->rt_timeout[i]*4));
	}

	for (i=0; i<list->major_cnt; i++) {
		major = list->majors[i];
		minor_cnt = major->cfg->minor_cnt;
		printf(" MAJOR[%d]\n", i);
		printf("   minor count: %d\n", minor_cnt);

		for (j=0; j<minor_cnt; j++) {
			minor = major->minors[j];

			printf("   MINOR[%d]\n", j);
			printf("     bd:        0x%08x (HW:0x%08x)\n",
				(unsigned int)&minor->bds[0],
				(unsigned int)gr1553bc_bd_cpu2hw(list,
							&minor->bds[0]));
			printf("     slot cnt:  %d\n", minor->cfg->slot_cnt);
			if ( minor->cfg->timeslot ) {
				timefree = gr1553bc_minor_freetime(minor);
				printf("     timefree:  %d\n", timefree);
				printf("     timetotal: %d\n",
					minor->cfg->timeslot);
			} else {
				printf("     no time mgr\n");
			}
		}
	}
}
