#!/usr/bin/env python -u 

# this testrogram assumes that the program under test
# is connected to it the following way:
# runtest.py-stdin  <- fifo <- testprog-stdout
# runtest.py-stderr -> fifo -> testprog-stdin
# runtest.py-stdout will be the default stdout that
# will echo stdin.

import sys, tty, termios
import select
import sys, time

cmd=sys.argv[1]
print "Reading cmd" + cmd
cmd_file = open(cmd, "r")
cmds = cmd_file.read()
print cmds
cmd_file.close()
commands = [];

# format of inut file:
# <1>@<2>@<3>@<4>
# <1>        :command to issue
# <2>,<3>,<4>:wait expression meaning:
#             "wait for string <4> to appear on stdout,
#              if <3> is given timeout wait after <3> sec - in case of
#               a timeout dont issue <1> of next command -
#              and after having matched <4> delay for <2> sec"
# example:
#                      @0.1@5@->
#cmd                   @0.1@ @[vxWorks *]
#cd /romfs             @0.1@ @[vxWorks *]
#ld < tmSyscallTiming.o@0.5@ @0x
#ld < bmRun.o          @0.5@ @0x
#ld < bmRTPKernel.o    @0.5@ @0x 
#ld < bmKernelToRtp.o  @0.5@ @0x
#C                     @0.5@ @->
#syscallTimingInit     @0.5@ @Group registration OK
#bmRTPKernelInit       @0.5@ @Group registration OK
#cmd                   @0.5@ @[vxWorks *]
#bmEvent.vxe           @0.5@ @[vxWorks *]
#bmTask.vxe            @0.5@ @[vxWorks *]

for c in cmds.split('\n'):
    c.strip();
    if ((len(c) == 0) or c.startswith("#")):
        continue;
    try:
        strcmd, strsleep, timeoutstr, strexp = c.split('@')
        print "Found record: sleep %d, wait: %s timeout: %s expect: %s " % (float(strsleep), str(strcmd), str(timeoutstr), str(strexp));
        strcmd = strcmd.strip();
        strsleep = strsleep.strip();
        strexp = strexp.strip();
        timeoutstr = timeoutstr.strip();
        if (len(timeoutstr)):
            timeout = float(timeoutstr)
        else:
            timeout = 0;
        commands.append([strcmd,strsleep,strexp,timeout]);
    except:
        print "Cannot parse " + c

input = [sys.stdin]
clients = []
running = 1
o = "";
line = "";
expect = commands[0][2];
sleeptill = 0;
timeout = 0;
if (commands[0][3]):
    timeout = time.time()+commands[0][3];

while running:
    inputready,outputready,exceptready = select.select(input+clients,[],[],0.1)

    gottimeout = 0
    if timeout != 0:
        if time.time() >= timeout:
            gottimeout = 1;
            sleeptill = time.time();
            if len(commands) >=2:
                commands[1][0] = "";
            
    if sleeptill != 0:
        if time.time() >= sleeptill:
            sleeptill = 0;
            commands.pop(0)
            if len(commands) == 0:
                sys.stdout.write("Test finished\n");
                sys.exit(0);
            expect = commands[0][2];
            timeout = 0;
            if (commands[0][3]):
                timeout = time.time()+commands[0][3];
            outstr = commands[0][0]
            if len(outstr) > 0:
                sys.stderr.write(outstr+"\n");

    for s in inputready:
        if s == sys.stdin:
            # handle standard input
            ch = sys.stdin.read(1)
            if not ch:
                sys.stdout.write("Stdin disconnected\n");
                sys.exit(1);
            o += str(ch);
            line +=  str(ch);
            sys.stdout.write(str(ch));
        else:
            try:
                ret = int(str)
                # handle all other sockets
            except Error:
                s.close()
                clients.remove(s)
                
    if sleeptill == 0:
        n = line.find(expect);
        if (n != -1):
            line = line[n+len(expect):];
            sleeptill = time.time() + float(commands[0][1]);

               
