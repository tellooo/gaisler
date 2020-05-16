#include <stdio.h>
#include <stdlib.h>

#include <grlib/grspw_router.h>

void *routers[2] = {NULL, NULL};
struct router_hw_info router_hw;
int router_nports;

struct router_config router_cfg =
{
	.flags = ROUTER_FLG_CFG | ROUTER_FLG_IID /*| ROUTER_FLG_TPRES | ROUTER_FLG_TRLD*/,
	.config = 0x10, /* Self-Address Enable */
	.iid = 1,
/*	.timer_prescaler = 0x123,
	.timer_reload = {
	}*/
};

struct router_routing_table routing_table =
{
	.flags = ROUTER_ROUTE_FLG_MAP | ROUTER_ROUTE_FLG_CTRL,
	.acontrol = {
        .control_logical = {
		/* 020..027 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 020..02f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 030..037 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 030..03f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 040..047 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 040..04f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 050..057 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 050..05f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 060..067 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 060..06f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 070..077 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 070..07f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 080..087 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 080..08f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 090..097 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 090..09f */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 0a0..0a7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0a0..0af */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0b0..0b7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0b0..0bf */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 0c0..0c7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0c0..0cf */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0d0..0d7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0d0..0df */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

		/* 0e0..0e7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0e8..0ef */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0f0..0f7 */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
		/* 0f8..0ff */ 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
        },
	},
	.portmap = {
        .pmap = {/* Loopback... */
		            0x00000002, 0x00000004, 0x00000008,
		0x00000010, 0x00000020, 0x00000040, 0x00000080,
		0x00000100, 0x00000200, 0x00000400, 0x00000800,
		0x00001000, 0x00002000, 0x00004000, 0x00008000,
		0x00010000, 0x00020000, 0x00040000, 0x00080000,
		0x00100000, 0x00200000, 0x00400000, 0x00800000,
		0x01000000, 0x02000000, 0x04000000, 0x08000000,
		0x10000000, 0x20000000, 0x40000000, 0x80000000,
	    },
	    .pmap_logical =	{
		/* 020..027 */ 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
		/* 028..02f */ 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
		/* 030..037 */ 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004,
		/* 038..03f */ 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000008,

		/* 040..047 */ 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x00000010,
		/* 048..04f */ 0x00000020, 0x00000020, 0x00000020, 0x00000020, 0x00000020, 0x00000020, 0x00000020, 0x00000020,
		/* 050..057 */ 0x00000040, 0x00000040, 0x00000040, 0x00000040, 0x00000040, 0x00000040, 0x00000040, 0x00000040,
		/* 058..05f */ 0x00000080, 0x00000080, 0x00000080, 0x00000080, 0x00000080, 0x00000080, 0x00000080, 0x00000080,

		/* 060..067 */ 0x00000100, 0x00000100, 0x00000100, 0x00000100, 0x00000100, 0x00000100, 0x00000100, 0x00000100,
		/* 068..06f */ 0x00000200, 0x00000200, 0x00000200, 0x00000200, 0x00000200, 0x00000200, 0x00000200, 0x00000200,
		/* 070..077 */ 0x00000400, 0x00000400, 0x00000400, 0x00000400, 0x00000400, 0x00000400, 0x00000400, 0x00000400,
		/* 078..07f */ 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800,

		/* 080..087 */ 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000,
		/* 088..08f */ 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000,
		/* 090..097 */ 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000,
		/* 098..09f */ 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,

		/* 0a0..0a7 */ 0x00010000, 0x00010000, 0x00010000, 0x00010000, 0x00010000, 0x00010000, 0x00010000, 0x00010000,
		/* 0a8..0af */ 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000,
		/* 0b0..0b7 */ 0x00040000, 0x00040000, 0x00040000, 0x00040000, 0x00040000, 0x00040000, 0x00040000, 0x00040000,
		/* 0b8..0bf */ 0x00080000, 0x00080000, 0x00080000, 0x00080000, 0x00080000, 0x00080000, 0x00080000, 0x00080000,

		/* 0c0..0c7 */ 0x00100000, 0x00100000, 0x00100000, 0x00100000, 0x00100000, 0x00100000, 0x00100000, 0x00100000,
		/* 0c8..0cf */ 0x00200000, 0x00200000, 0x00200000, 0x00200000, 0x00200000, 0x00200000, 0x00200000, 0x00200000,
		/* 0d0..0d7 */ 0x00400000, 0x00400000, 0x00400000, 0x00400000, 0x00400000, 0x00400000, 0x00400000, 0x00400000,
		/* 0d8..0df */ 0x00800000, 0x00800000, 0x00800000, 0x00800000, 0x00800000, 0x00800000, 0x00800000, 0x00800000,

		/* 0e0..0e7 */ 0x01000000, 0x01000000, 0x01000000, 0x01000000, 0x01000000, 0x01000000, 0x01000000, 0x01000000,
		/* 0e8..0ef */ 0x02000000, 0x02000000, 0x02000000, 0x02000000, 0x02000000, 0x02000000, 0x02000000, 0x02000000,
		/* 0f0..0f7 */ 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
		/* 0f8..0ff */ 0x08000000, 0x08000000, 0x10000000, 0x10000000, 0x20000000, 0x20000000, 0x40000000, 0x80000000,
        },
	},
};

char *linkstate_names[7] =
{
	/* 000 */ "Error Reset",
	/* 001 */ "Error Wait",
	/* 010 */ "Ready",
	/* 011 */ "Started",
	/* 100 */ "Connecting",
	/* 101 */ "Run",
	"N/A",
};

char *type_names[4] =
{
	"CFG ",
	"SpW ",
	"AMBA",
	"FIFO"
};

enum port_type {
	PT_CFG = 0,
	PT_SPW = 1,
	PT_AMBA = 2,
	PT_FIFO = 3,
};

int router_route_print(struct router_route * route)
{
	int i;

	if (route == NULL) {
		printf("ROUTER route wrong\n");
		return -1;
	}

	if ((route->options & ROUTER_ROUTE_ENABLE) == 0) {
		printf("ROUTE from address %d [0x%02x] disabled.", 
				route->from_address, (unsigned int) route->from_address);
		return 0;
	}

	/*DBG("ROUTE from address 0x%02x read, PCOUNT: %d [%d,%d,%d,%d,...], "
	 *		"CTRL: 0x%08x\n", (unsigned int) route->from_address, 
	 *		route->count, route->to_port[0], route->to_port[1], 
	 *		route->to_port[2], route->to_port[3], 
	 *		(unsigned int) route->options);*/

	if (route->count ==0) {
		printf("ROUTE from address %d [0x%02x]", 
				route->from_address, (unsigned int) route->from_address);
	}else{
		printf("ROUTE from address %d [0x%02x] to port[s]: ", 
				route->from_address, (unsigned int) route->from_address);
		for (i=0; i < route->count; i++) {
			printf("%d", route->to_port[i]);
			if (i != (route->count - 1) ) {
				printf(", ");
			}
		}
	}
	printf(". CTRL: ");
	if (route->options & ROUTER_ROUTE_PACKETDISTRIBUTION_ENABLE) {
		printf("Packet distribution enabled, ");
	}
	if (route->options & ROUTER_ROUTE_SPILLIFNOTREADY_ENABLE) {
		printf("Spill-if-not-ready enabled, ");
	}
	if (route->options & ROUTER_ROUTE_HEADERDELETION_ENABLE) {
		printf("Header deletion enabled, ");
	}
	if (route->options & ROUTER_ROUTE_PRIORITY_HIGH) {
		printf("High priority.\n");
	}else{
		printf("Low priority.\n");
	}

	return 0;
}

static void router_print_hwinfo(struct router_hw_info *hwinfo)
{
	puts("Hardware Configuration of SpaceWire Router:");
	printf(" Number of SpW ports:           %d\n", hwinfo->nports_spw);
	printf(" Number of AMBA ports:          %d\n", hwinfo->nports_amba);
	printf(" Number of FIFO ports:          %d\n", hwinfo->nports_fifo);
	printf(" Timers available:              %s\n", hwinfo->timers_avail ? "YES" : "NO");
	printf(" Plug and Play available:       %s\n", hwinfo->pnp_avail ? "YES" : "NO");
	printf(" MAJOR Version:                 %d\n", hwinfo->ver_major);
	printf(" MINOR Version:                 %d\n", hwinfo->ver_minor);
	printf(" PATCH Version:                 %d\n", hwinfo->ver_patch);
	printf(" Current Instance ID:           %d\n", hwinfo->iid);
}

int router_configure(int index, void **rtr)
{
	uint32_t ctrl, sts, cfgsts;
	int i, linkstate, type;
	char *typestr;
	void *router;

	printf("Configuring Router\n");

	router = *rtr = router_open(index);
	if ( router == NULL ){
		printf("Failed to open SpW Router\n");
		return -1;
	}

	if ( router_hwinfo_get(router, &router_hw) != ROUTER_ERR_OK ) {
		return -2;
	}
	router_print_hwinfo(&router_hw);

	if ( router_write_disable(router) != ROUTER_ERR_OK ) {
		return -3;
	}

	if ( router_config_set(router, &router_cfg) != ROUTER_ERR_OK ) {
		return -4;
	}

	if ( router_routing_table_set(router, &routing_table) != ROUTER_ERR_OK ) {
		return -5;
	}

	if ( router_cfgsts_get(router, &cfgsts) != ROUTER_ERR_OK ) {
		return -6;
	}
	printf("ROUTER CFG/STS: 0x%08x\n", (unsigned int) cfgsts);

	router_nports = router_hw.nports_spw + router_hw.nports_amba +
			router_hw.nports_fifo;
	/* Print Current Link/Port Control/Status */
	for (i=0; i<router_nports+1; i++) {
		if ( router_port_ctrl_get(router, i, &ctrl) != ROUTER_ERR_OK ) {
			return -7;
		}

		if ( router_port_status(router, i, &sts, 0x0) != ROUTER_ERR_OK ) {
			return -8;
		}

		if ( i == 0 ) {
			type = PT_CFG;
		} else if ( i < (1+router_hw.nports_spw) ) {
			type = PT_SPW;
		} else if ( i < (1+router_hw.nports_spw+router_hw.nports_amba) ) {
			type = PT_AMBA;
		} else {
			type = PT_FIFO;
		}
		typestr = type_names[type];

		if ( type != PT_SPW )
			linkstate = 6; /* N/A */
		else
			linkstate = (sts >> 12) & 0x7;

		printf("PORT[%02d]:  TYPE=%s CTRL=0x%08x  STATUS=0x%08x  LINKSTATE=%s\n",
			i, typestr, (unsigned int) ctrl, (unsigned int) sts, linkstate_names[linkstate]);
	}

	/* Enable and Start all Links */
	printf("Activating all Links/Ports\n");
	for (i=0; i<router_nports+1; i++) {

		if ( i == 0 ) {
			type = PT_CFG;
		} else if ( i < (1+router_hw.nports_spw) ) {
			type = PT_SPW;
		} else if ( i < (1+router_hw.nports_spw+router_hw.nports_amba) ) {
			type = PT_AMBA;
		} else {
			type = PT_FIFO;
		}

		switch (type) {
		default:
		case PT_CFG:
			break;
		case PT_SPW: /* SpaceWire Port */
			if ( router_port_ctrl_set(router, i, 0xffffffff, 0x0100002e) != ROUTER_ERR_OK ) {
				return -10;
			}
			break;
		case PT_AMBA: /* AMBA Port */
			if ( router_port_ctrl_set(router, i, 0xffffffff, 0x00000028) != ROUTER_ERR_OK ) {
				return -11;
			}
			break;
		case PT_FIFO: /* FIFO Port */
			if ( router_port_ctrl_set(router, i, 0xffffffff, 0x00000028) != ROUTER_ERR_OK ) {
				return -12;
			}
			break;
		}
	}

	/* Print Current Link Status */
	for (i=1; i<router_hw.nports_spw+1; i++) {
		if ( router_port_ctrl_get(router, i, &ctrl) != ROUTER_ERR_OK ) {
			return -7;
		}

		if ( router_port_status(router, i, &sts, 0x0) != ROUTER_ERR_OK ) {
			return -8;
		}

		if ( i == 0 ) {
			type = PT_CFG;
		} else if ( i < (1+router_hw.nports_spw) ) {
			type = PT_SPW;
		} else if ( i < (1+router_hw.nports_spw+router_hw.nports_amba) ) {
			type = PT_AMBA;
		} else {
			type = PT_FIFO;
		}
		typestr = type_names[type];

		if ( type != PT_SPW )
			linkstate = 6; /* N/A */
		else
			linkstate = (sts >> 12) & 0x7;

		printf("PORT[%02d]:  TYPE=%s   CTRL=0x%08x   STATUS=0x%08x   LINK-STATE=%s\n",
			i, typestr, (unsigned int) ctrl, (unsigned int) sts, linkstate_names[linkstate]);
	}

	return 0;
}

int router_setup_custom(void)
{
	int i, result;

	for (i = 0; i < 2; i++) {
		result = router_configure(i, &routers[i]);
		if (result) {
			if (i == 0 || result < -1) {
				printf("Router_configure: Error %d\n", result);
				return result;
			}
		}
	}

	return 0;
}
