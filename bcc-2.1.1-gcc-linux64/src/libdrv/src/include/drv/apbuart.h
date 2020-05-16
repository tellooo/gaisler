/*
 * Copyright (c) 2018, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#ifndef DRV_APBUART_H
#define DRV_APBUART_H

#include <stdint.h>
#include <drv/fifo.h>
#include <drv/auto.h>
#include <drv/osal.h>

#define APBUART_MODE_UNCONFIGURED -1
#define APBUART_MODE_NONINT 0
#define APBUART_MODE_INT 1

#define APBUART_PAR_NONE 0
#define APBUART_PAR_EVEN 2
#define APBUART_PAR_ODD 3

struct apbuart_config {
        /* UART baud, bits per second */
        int baud;
        /* APBUART_PAR_NONE, APBUART_PAR_EVEN or APBUART_PAR_ODD */
        int parity;
        /* APBUART_MODE_NONINT or APBUART_MODE_INT */
        int mode;
        /* Enable flow-control */
        int flow;
        /* Buffer area for TX SW FIFO */
        uint8_t *txfifobuf;
        /* Number of bytes allocated for TX SW FIFO */
        int txfifobuflen;
        /* Buffer area for RX SW FIFO */
        uint8_t *rxfifobuf;
        /* Number of bytes allocated for RX SW FIFO */
        int rxfifobuflen;
};

struct apbuart_devcfg;
struct apbuart_priv;

/*
 * Retrieve number of APBUART devices registered to the driver
 *
 * return: Number of APBUART devices registered.
 */
int apbuart_dev_count(void);

/* return: number of devices registered */
int apbuart_autoinit(void);
/* Register one device */
int apbuart_register(struct apbuart_devcfg *devcfg);
/* Register an array of devices */
int apbuart_init(struct apbuart_devcfg *devcfgs[]);

/*
 * Open an APBUART device
 *
 * The APBUART device is identified by index. Index value must be equal to or
 * greater than zero, and smaller than value returned by apbuart_dev_count. The
 * returned value is used as input argument to all functions operating on the
 * device. It is not possible to open an already opened device index.
 *
 * dev_no: Device identification number.
 *
 * return:
 * - NULL: Failed to open device.
 * - others: APBUART device handle to use as input parameter to all device API
 * functions for the opened device.
 */
struct apbuart_priv *apbuart_open(int dev_no);

/*
 * Close an APBUART device
 *
 * The transmitter and receiver are disabled.
 *
 * d: Device handle returned by apbuart_open.
 *
 * return:
 * - DRV_OK: Successfully closed device.
 * - others: Device closed, but failed to unregister interrupt handler.
 */
int apbuart_close(struct apbuart_priv *priv);

/*
 * Get APBUART status register
 *
 * d: Device handle returned by apbuart_open.
 *
 * return: Copy of UART status register for device d.
 */
uint32_t apbuart_get_status(struct apbuart_priv *priv);

/*
 * Set APBUART status register
 *
 * d: Device handle returned by apbuart_open.
 * status: Value to write to APBUART status register.
 */
void apbuart_set_status(struct apbuart_priv *priv, uint32_t status);

/*
 * Configure APBUART device
 *
 * The cfg input layout is described by struct apbuart_config. IF interrupt
 * mode is configured, then the driver will reigster an ISR with help of OSAL.
 *
 * d: Device handle returned by apbuart_open.
 * cfg: Pointer to configuration structure.
 *
 * return:
 * - DRV_OK: Device configured successfully.
 * - others: Failed to register UART ISR: device configuration aborted.
 */
int apbuart_config(struct apbuart_priv *priv, const struct apbuart_config *cfg);

/*
 * Send one data byte
 *
 * The function will try to transmit one data byte on the UART. The operation is
 * non-blocking and returns 0 if the transmit FIFO is full.
 *
 * d: Device handle returned by apbuart_open.
 * data: Data byte to send.
 *
 * return: Number of bytes copied to transmit FIFO.
 * - 0: The byte was not sent.
 * - 1: The byte was sent.
 */
int apbuart_outbyte(struct apbuart_priv *priv, uint8_t data);

/*
 * Receive one data byte
 *
 * The function tries to receive one byte of data from the UART receive FIFO.
 * The operation is non-blocking and returns -1 if the transmit FIFO is empty.
 *
 * d: Device handle returned by apbuart_open.
 *
 * return: The received data byte, as uint8_t casted to an int. If no data byte
 * was available then -1 is returned.
 */
int apbuart_inbyte(struct apbuart_priv *priv);

/*
 * Send zero or more data bytes
 *
 * This function sends up to count data bytes from buf to the UART associated
 * with the device handle d. Number of bytes actually sent can be less than
 * count if the hardware and software TX FIFOs become full. The operation is
 * non-blocking.
 *
 * d: Device handle returned by apbuart_open.
 * buf: Data bytes to send.
 * count: Number of bytes to send.
 *
 * return: Number of bytes actually transmitted, which may be less than count
 * if FIFOs become full.
 */
int apbuart_write(struct apbuart_priv *priv, const uint8_t *buf, int count);

/*
 * Receive zero or more data bytes
 *
 * This function receives up to count bytes from the UARTs associated with the
 * device handle d and stores the data at location buf. Number of bytes
 * actually received can be less than count. The operation is non-blocking.
 *
 * d: Device handle returned by apbuart_open.
 * buf: Receive buffer.
 * count: Number of bytes to receive.
 *
 * return: Number of bytes actually received, which may be less than count
 * if FIFOs become empty.
 */
int apbuart_read(struct apbuart_priv *priv, uint8_t *buf, int count);

/*
 * Drain UART transmission
 *
 * This function waits for UART hardware transmission to finish.
 *
 * d: Device handle returned by apbuart_open.
 *
 * return: DRV_OK
 */
int apbuart_drain(struct apbuart_priv *priv);

/*
 * Returns device registers
 *
 * dev_no: the device number to return registers for
 *
 * return:
 * - Pointer to the device registers
 * - NULL if invalid device number
 */
const struct drv_devreg *apbuart_get_devreg(int dev_no);

/*
 * Configure APBUART debug mode tunneling
 */
int apbuart_set_debug(struct apbuart_priv *priv, int en);

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct apbuart_priv {
        volatile struct apbuart_regs *regs;
        int interrupt;
        int mode;
        struct fifo txfifo;
        struct fifo rxfifo;
        /* debug mode configuration */
        uint32_t debug;
        /* Bus frequency, Hertz */
        unsigned int apbfreq;
        uint8_t open;
        struct osal_isr_ctx isr_ctx;
        SPIN_DECLARE(devlock)
};

/*
 * Device configuration
 *
 * This structure is used for registering the base addr of a device and its
 * interrupt number. It also contains the private device structure.
 */

struct apbuart_devcfg {
        struct drv_devreg regs;
        struct apbuart_priv priv;
};

#endif

