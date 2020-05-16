/**
 *  @file
 *  @brief mq_timedreceive() API Conformance Test
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2018, Himanshu Sekhar Nayak
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <time.h>
#include <mqueue.h>

int test( void );

#define MQ_MAXMSG     1
#define MQ_MSGSIZE    sizeof(int)

int test( void )
{
  mqd_t mqdes;
  struct mq_attr mqstat;
  const char *q_name;
  int message[MQ_MAXMSG];
  struct timespec abs_timeout;
  unsigned int msg_prio;
  int result;

  mqstat.mq_maxmsg  = MQ_MAXMSG;
  mqstat.mq_msgsize = MQ_MSGSIZE;
  abs_timeout.tv_sec  = 0;
  abs_timeout.tv_nsec = 0;
  msg_prio = 1;
  q_name = "queue";

  mqdes = mq_open( q_name, O_CREAT | O_RDWR, 0x777, &mqstat );
  result = mq_timedreceive(
		mqdes, (char *)message, MQ_MSGSIZE, &msg_prio, &abs_timeout );

  return result;
}
