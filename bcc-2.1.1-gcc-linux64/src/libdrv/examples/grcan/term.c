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

/*
 * 0: scan devices using ambapp
 * 1: use GR716 static driver tables
 */
#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #include <bcc/gr716/pin.h>
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linenoise.h>

#include <drv/grcan.h>
#include <drv/gr716/grcan.h>
#include <drv/nelem.h>

#define DEVS_MAX 4
int nospw = 0;
struct grcan_priv *DEVS[DEVS_MAX];

static const char *PROMPT = "grcan> ";

enum {
        DEVNO_NONE      = -1,
        DEVNO_BAD       = -2,
};

struct lcmd {
        const char *name;
        const char *syntax;
        const char *desc;
        int (*func)(char **words, int word, int devno);
};

extern struct lcmd lcmds[];

/* CAN HELP DEBUG FUNCTIONS */
char *msgstr_type[2] = { "STD", "EXT" };
char *msgstr_rtr[2] = { "", " RTR" };

/* PRINT A CAN MESSAGE FROM DATA STRUCTURE */
void print_msg(int i, struct grcan_canmsg * msg)
{
        int j;
        char data_str_buf[64];
        int ofs;

        if (!msg)
                return;

        if (i > 0) {
                printf("MSG[%d]: %s%s length: %d, id: 0x%x\n", i,
                       msgstr_type[(int)msg->extended],
                       msgstr_rtr[(int)msg->rtr], msg->len, msg->id);
                /* print data */
                if (msg->len > 0) {
                        ofs = sprintf(data_str_buf, "MSGDATA[%d]: ", i);
                        for (j = 0; j < msg->len; j++) {
                                ofs +=
                                    sprintf(data_str_buf + ofs, "0x%02x ",
                                            msg->data[j]);
                        }
                        printf("%s  ", data_str_buf);
                        ofs = 0;
                        for (j = 0; j < msg->len; j++) {
                                if (isalnum(msg->data[j]))
                                        ofs +=
                                            sprintf(data_str_buf + ofs, "%c",
                                                    msg->data[j]);
                                else
                                        ofs += sprintf(data_str_buf + ofs, ".");
                        }
                        printf("%s\n", data_str_buf);
                }
        } else {
                printf("MSG: %s%s length: %d, id: 0x%x\n",
                       msgstr_type[(int)msg->extended],
                       msgstr_rtr[(int)msg->rtr], msg->len, msg->id);
                /* print data */
                if (msg->len > 0) {
                        ofs = sprintf(data_str_buf, "MSGDATA: ");
                        for (j = 0; j < msg->len; j++) {
                                ofs +=
                                    sprintf(data_str_buf + ofs, "0x%02x ",
                                            msg->data[j]);
                        }
                        printf("%s  ", data_str_buf);
                        ofs = 0;
                        for (j = 0; j < msg->len; j++) {
                                if (isalnum(msg->data[j]))
                                        ofs +=
                                            sprintf(data_str_buf + ofs, "%c",
                                                    msg->data[j]);
                                else
                                        ofs += sprintf(data_str_buf + ofs, ".");
                        }
                        printf("%s\n", data_str_buf);
                }
        }
}

static const char *grcan_ret_str(int i)
{
        switch (i) {
                case GRCAN_RET_OK:              return "OK";
                case GRCAN_RET_INVARG:          return "INVARG";
                case GRCAN_RET_NOTSTARTED:      return "NOTSTARTED";
                case GRCAN_RET_TIMEOUT:         return "TIMEOUT";
                case GRCAN_RET_BUSOFF:          return "BUSOFF";
                case GRCAN_RET_AHBERR:          return "AHBERR";
                case GRCAN_RET_INVSTATE:        return "INVSTATE";
        }
        return "???";
}

static const char *STATE_TO_STR[] = {
        [STATE_STOPPED] = "STOPPED",
        [STATE_STARTED] = "STARTED",
        [STATE_BUSOFF]  = "BUSOFF",
        [STATE_AHBERR]  = "AHBERR",
        "<INVALID>",
};

const char *grcan_state_to_name(unsigned int i)
{
        if (3 < i) { i = 4; }
        return STATE_TO_STR[i];
}

static int func_x(char **words, int word, int devno)
{
        struct grcan_canmsg msg = { 0 };
        char *thestr = "abc123";
        int id = 1;

        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        if (2 < word) {
                id = strtoul(words[2], NULL, 0);
        }

        if (3 < word) {
                thestr = words[3];
        }

        size_t len = strlen(thestr);
        if (8 < len) {
                len = 8;
        }
        memcpy(&msg.data[0], thestr, len);
        msg.len = len;

        struct grcan_priv *dev = DEVS[devno];
        msg.id = id;
        int ret;

        printf(" grcan%d: scheduling messages:\n", devno);
        print_msg(0, &msg);
        ret = grcan_write(dev, &msg, 1);
        if (ret == 0) {
                printf("INFO: no message scheduled.\n");
        } else if (ret < 0) {
                printf("ERROR: grcan_write() says '%s'\n", grcan_ret_str(ret));
        }

        return 0;
}

/* NOTE: could do the read in chunks */
static int func_r(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                struct grcan_priv *dev = DEVS[devno];
                int j = 0;

                printf(" grcan%d: --- receive begin ---\n", devno);
                while (1) {
                        struct grcan_canmsg msg;
                        int ret;
                        ret = grcan_read(dev, &msg, 1);
                        if (ret == 0) {
                                break;
                        } else if (ret < 0) {
                                printf("ERROR: grcan_read() says '%s'\n", grcan_ret_str(ret));
                                break;
                        }
                        print_msg(j+1, &msg);
                        j++;
                }
                printf(" grcan%d: --- receive end   ---\n", devno);
                devno++;
        }

        return 0;
}

static int func_start(char **words, int word, int devno)
{
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        struct grcan_priv *dev = DEVS[devno];
        int ret;

        ret = grcan_start(dev);
        if (ret != GRCAN_RET_OK) {
                printf("ERROR: grcan_start() says '%s'\n", grcan_ret_str(ret));
        }

        return 0;
}

static int func_stop(char **words, int word, int devno)
{
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        struct grcan_priv *dev = DEVS[devno];
        int ret;

        ret = grcan_stop(dev);
        if (ret != GRCAN_RET_OK) {
                printf("ERROR: grcan_stop() says '%s'\n", grcan_ret_str(ret));
        }

        return 0;
}

static int func_speed(char **words, int word, int devno)
{
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        if (word < 3) {
                printf("%s: command need HZ parameter\n", words[0]);
                return 1;
        }

        unsigned int hz = strtoul(words[2], NULL, 0);
        struct grcan_priv *dev = DEVS[devno];
        int ret;

        ret = grcan_set_speed(dev, hz);
        if (ret != GRCAN_RET_OK) {
                printf("ERROR: grcan_set_speed() says '%s'\n", grcan_ret_str(ret));
        }

        return 0;
}

enum {
        XFILTER_AFILTER,
        XFILTER_SFILTER,
};

static int func_xfilter(char **words, int word, int devno, int type)
{
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        if (word < 4) {
                printf("%s: command need mask and code parameter\n", words[0]);
                return 1;
        }

        struct grcan_filter filter;
        struct grcan_priv *dev = DEVS[devno];
        int ret;

        filter.mask = strtoul(words[2], NULL, 0);
        filter.code = strtoul(words[3], NULL, 0);
        if (type == XFILTER_AFILTER) {
                ret = grcan_set_afilter(dev, &filter);
                if (ret != GRCAN_RET_OK) {
                        printf("ERROR: grcan_set_afilter() says %d\n", ret);
                }
        } else {
                ret = grcan_set_sfilter(dev, &filter);
                if (ret != GRCAN_RET_OK) {
                        printf("ERROR: grcan_set_sfilter() says %d\n", ret);
                }
        }

        return 0;
}

static int func_afilter(char **words, int word, int devno)
{
        return func_xfilter(words, word, devno, XFILTER_AFILTER);
}

static int func_sfilter(char **words, int word, int devno)
{
        return func_xfilter(words, word, devno, XFILTER_SFILTER);
}

static int func_state(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                struct grcan_priv *dev = DEVS[devno];
                int ret;
                ret = grcan_get_state(dev);
                printf("grcan%d: STATE_%s\n", devno, grcan_state_to_name(ret));
                devno++;
        }

        return 0;
}

static int func_status1(struct grcan_priv *dev, int devno)
{
        unsigned int s;
        int ret = grcan_get_status(dev, &s);
        if (ret) {
                printf("ERROR: grcan_get_status() says %d\n", ret);
                return 1;
        }
        printf(" grcan%d: status 0x%08x\n", devno, s);
        printf(" grcan%d:    txerrcntr = %4u\n", devno, (s >> 16) & 0xff );
        printf(" grcan%d:    rxerrcntr = %4u\n", devno, (s >>  8) & 0xff );
        printf(" grcan%d:    active    = %4u\n", devno, (s >>  4) & 0x01 );
        printf(" grcan%d:    ahberr    = %4u\n", devno, (s >>  3) & 0x01 );
        printf(" grcan%d:    or        = %4u\n", devno, (s >>  2) & 0x01 );
        printf(" grcan%d:    off       = %4u\n", devno, (s >>  1) & 0x01 );
        printf(" grcan%d:    pass      = %4u\n", devno, (s >>  0) & 0x01 );
        return 0;
}

static int func_status(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                struct grcan_priv *dev = DEVS[devno];
                func_status1(dev, devno);
                printf("\n");
                devno++;
        }

        return 0;
}

static const char *STR_NO_YES[2] = { "NO", "YES" };
static int func_istxdone(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                struct grcan_priv *dev = DEVS[devno];
                int ret;
                ret = grcan_istxdone(dev);
                printf("grcan%d: istxdone: %s\n", devno, STR_NO_YES[!!ret]);
                devno++;
        }

        return 0;
}

static int func_help(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        UNUSED(devno);
        for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                char buf[32];
                snprintf(buf, 32, "%s%s", cmd->name, cmd->syntax);
                buf[31] = '\0';
                printf(" %-22s - %s\n", buf, cmd->desc);
        }
        return 0;
}

int quit = 0;

static int func_quit(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        UNUSED(devno);
        quit = 1;
        return 0;
}

struct lcmd lcmds[] = {
        {
                .name   = "help",
                .syntax = "",
                .desc   = "List commands",
                .func   = func_help,
        },
        {
                .name   = "start",
                .syntax = " DEV",
                .desc   = "Start",
                .func   = func_start,
        },
        {
                .name   = "stop",
                .syntax = " DEV",
                .desc   = "Stop",
                .func   = func_stop,
        },
        {
                .name   = "state",
                .syntax = " [DEV]",
                .desc   = "Driver state",
                .func   = func_state,
        },
        {
                .name   = "status",
                .syntax = " [DEV]",
                .desc   = "Get GRCAN core status register",
                .func   = func_status,
        },
        {
                .name   = "speed",
                .syntax = " DEV HZ",
                .desc   = "Set speed",
                .func   = func_speed,
        },
        {
                .name   = "afilter",
                .syntax = " DEV MASK CODE",
                .desc   = "Set Acceptance filter",
                .func   = func_afilter,
        },
        {
                .name   = "sfilter",
                .syntax = " DEV MASK CODE",
                .desc   = "Set Sync Messages RX/TX filters",
                .func   = func_sfilter,
        },
        {
                .name   = "q",
                .syntax = "",
                .desc   = "Quit",
                .func   = func_quit,
        },
        {
                .name   = "x",
                .syntax = " DEV [ID] [STRING]",
                .desc   = "Transmit one message",
                .func   = func_x,
        },
        {
                .name   = "r",
                .syntax = " [DEV]",
                .desc   = "Receive messages",
                .func   = func_r,
        },
        {
                .name   = "istxdone",
                .syntax = " [DEV]",
                .desc   = "Determine if all TX is done",
                .func   = func_istxdone,
        },
        { .name = NULL },
};

static char *hints(const char *buf, int *color, int *bold) {
        for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                if (0 == strcmp(buf, cmd->name)) {
                        *color = 35;
                        *bold = 0;
                        return (char *) cmd->syntax;
                }
        }
        return NULL;
}

void commandloop(void)
{
        int i;

        char *in_buf = NULL;
        int in_buf_pos = 0;
        int newtoken, word;
        char *words[32];
        int command;

        linenoiseSetHintsCallback(hints);
        linenoiseHistorySetMaxLen(12);
        printf("Starting Packet processing loop. Enter Command:\n\n");
        quit = 0;
        command = 1;
        while (quit == 0) {
                if (command == 1) {
                        command = 0;
                }

                free(in_buf);
                in_buf = linenoise(PROMPT);
                if (NULL == in_buf) {
                        printf("ERROR: linenoise...\n");
                }
                in_buf_pos = strlen(in_buf);
                command = !!in_buf_pos;

                /* if one line of input completed then execute it */
                if (command == 0)
                        continue;

                linenoiseHistoryAdd(in_buf); /* Add to the history. */

                /* Parse buffer into words */
                newtoken = 1;
                word = 0;
                for (i = 0; i < in_buf_pos; i++) {
                        if ((in_buf[i] == ' ') || (in_buf[i] == '\t')) {
                                in_buf[i] = '\0';
                                newtoken = 1;
                                continue;
                        }

                        /* process one word */
                        if (newtoken) {
                                words[word++] = &in_buf[i];
                                newtoken = 0;
                        }
                }
                in_buf_pos = 0;

                /* Parse words to a command */
                if (word < 1) {
                        printf(" invalid command\n");
                        continue;
                }

                /* maybe parse optional devno */
                int devno = DEVNO_NONE;
                if (1 < word ) {
                        char *endptr;
                        devno = strtoul(words[1], &endptr, 0);
                        if (endptr == words[1]) {
                                devno = DEVNO_BAD;
                        } else if (devno < 0 || devno >= nospw) {
                                devno = DEVNO_BAD;
                        }
                }

                int cmdfound = 0;
                for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                        if (0 == strcmp(cmd->name, words[0])) {
                                //printf("match: %s\n", cmd->name);
                                cmd->func(words, word, devno);
                                cmdfound = 1;
                                break;
                        }
                }
                if (!cmdfound) {
                        printf("%s: command not found. See 'help'.\n", words[0]);
                }

                fflush(NULL);
        }
}

/* CAN Channel select */
enum {
        CAN_CHAN_SEL_A,
        CAN_CHAN_SEL_B,
        CAN_CHAN_SEL_NUM
};

static const struct grcan_selection CAN_CHAN_SEL[CAN_CHAN_SEL_NUM] = {
        {
                /* Channel A */
                .selection = 0,
                .enable0 = 0,
                .enable1 = 1,
        },
        {
                /* Channel B */
                .selection = 1,
                .enable0 = 1,
                .enable1 = 0,
        },
};

static const struct grcan_timing CAN_TIMING = {
        /* Set baud rate: 250k @ 30MHz */
        .scaler = 3,
        .ps1 = 8,
        .ps2 = 5,
        .rsj = 1,
        .bpr = 1,
};

void grcan000(void)
{
        int ret;

        if (CFG_TARGET_GR716) {
#ifdef BCC_BSP_gr716
                for (int i = 58; i <= 63; i++) {
                        gr716_set_pinfunc(i, IO_MODE_CAN);
                }
#endif
                grcan_init(GR716_GRCAN_DRV_ALL);
        } else {
                grcan_autoinit();
        }

        nospw = grcan_dev_count();
        if (nospw < 1) {
                printf("Found no GRCAN cores, aborting\n");
                exit(0);
        }
        printf("Found %d GRCAN cores\n", nospw);
        if (nospw > DEVS_MAX) {
                printf("Limiting to %d GRCAN devices\n", DEVS_MAX);
                nospw = DEVS_MAX;
        }

        puts("");
        for (int i=0; i<nospw; i++) {
                struct grcan_priv *dev;
                dev = grcan_open(i);
                if (!dev) {
                        printf("Failed to open grcan%d\n", i);
                        exit(0);
                }
                DEVS[i] = dev;
                printf("grcan%d: opened\n", i);
        }

        for (int i=0; i<nospw; i++) {
                int chan;
	              const struct grcan_selection *selection;

#ifdef CFG_INTLOOPBACK
                /* all controllers use the same internal channel (A) */
                chan = CAN_CHAN_SEL_A;
#else
                /* different internal channels for different controllers */
                chan = (CAN_CHAN_SEL_A + i) % 2;
#endif
                selection = &CAN_CHAN_SEL[chan];
                ret = grcan_set_selection(DEVS[i], selection);
                if (ret) {
                        printf("grcan_set_selection() failed: %d\n", ret);
                        exit(0);
                }
        }

        for (int i=0; i<nospw; i++) {
                ret = grcan_set_btrs(DEVS[i], &CAN_TIMING);
                if (ret) {
                        printf("grcan_set_btrs() failed: %d\n", ret);
                        exit(0);
                }
        }

        puts("");
        fflush(NULL);

        /* Start */
        for (int i=0; i<nospw; i++) {
                struct grcan_priv *dev;
                dev = DEVS[i];
                ret = grcan_start(dev);
                assert(DRV_OK == ret);
                printf("grcan%d: started\n", i);
                if (DRV_OK != ret) {
                        printf("Failed to start grcan%d\n", i);
                        exit(0);
                }
        }
        puts("");
        fflush(NULL);

        commandloop();

        /* Close */
        for (int i=0; i<nospw; i++) {
                struct grcan_priv *dev;
                dev = DEVS[i];
                ret = grcan_close(dev);
                assert(DRV_OK == ret);
                if (DRV_OK != ret) {
                        printf("Failed to close grcan%d\n", i);
                        exit(0);
                }
        }
}

int main(void)
{
        puts("GRCAN example begin");
        grcan000();
        puts("PASS");
        puts("GRCAN example end");
        return 0;
}

