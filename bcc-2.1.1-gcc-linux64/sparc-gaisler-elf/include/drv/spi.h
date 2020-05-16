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

#ifndef DRV_SPI_H
#define DRV_SPI_H

#include <stdint.h>
#include <drv/regs/spictrl-regs.h>
#include <drv/auto.h>
#include <drv/osal.h>

/* Values for for spi_config.wordlen */
enum spi_wordlen {
        SPI_WORDLEN_4,  SPI_WORDLEN_5,  SPI_WORDLEN_6,  SPI_WORDLEN_7,
        SPI_WORDLEN_8,  SPI_WORDLEN_9,  SPI_WORDLEN_10, SPI_WORDLEN_11,
        SPI_WORDLEN_12, SPI_WORDLEN_13, SPI_WORDLEN_14, SPI_WORDLEN_15,
        SPI_WORDLEN_16, SPI_WORDLEN_32, SPI_WORDLEN_NUM
};

struct spi_config {
        /* SPI clock frequency, Hz */
        unsigned int freq;
        /* SPI mode 0, 1, 2 or 3 */
        int mode;
        /* Word length, according to SPI_WORDLEN_* */
        enum spi_wordlen wordlen;
        /* SPI controller interrupt mask */
        int intmask;
        /* If true then Send MSb first, otherwise send LSb */
        int msb_first;
        /*
         * Synchronous TX/RX mode.
         * 0 - Allow RX to overrun.
         * 1 - Prevent RX from overrunning.
         */
        int sync;
        /* Automatic slave select, active high mask */
        uint32_t aslave;
        /* Number of SCK clock cycles to insert between consecutive words */
        int clock_gap;
        /* Toggle automatic slave select during clock gap */
        int tac;
        /* Automatic slave select delay */
        int aseldel;
        /* Ignore SPISEL input */
        int igsel;
};

/* Usable default values for all fields */
extern const struct spi_config SPI_CONFIG_DEFAULT;

struct spi_devcfg;
struct spi_priv;

/* return: number of devices registered */
int spi_autoinit(void);
/* Register one device */
int spi_register(struct spi_devcfg *devcfg);
/* Register an array of devices */
int spi_init(struct spi_devcfg *devcfgs[]);

/* Retrieve number of SPI devices registered to the driver */
int spi_dev_count(void);

/* Open an SPI device */
struct spi_priv *spi_open(int dev_no);

/* Close an SPI device */
int spi_close(struct spi_priv *priv);

/* Get event register value */
uint32_t spi_get_event(struct spi_priv *priv);

/* Clear bits in the event register */
void spi_clear_event(struct spi_priv *priv, uint32_t event);

/* Configure SPI device */
int spi_config(struct spi_priv *priv, struct spi_config *cfg);

/* Start SPI device */
int spi_start(struct spi_priv *priv);

/* Stop SPI device */
int spi_stop(struct spi_priv *priv);

/*
 * Write elements to SPICTRL transmit queue.
 *
 * The function tries to write count elements of the configured word length to
 * the transmit queue. Transmission data is indicated by txbuf.  Each word is
 * represented by a type corresponding to the configured SPI word length. Words
 * in txbuf shall be represented with its LSB at bit 0.
 *
 * NOTE: The interpreteation of txbuf depends on the configured SPI word
 * length. For example, if configured word length is 32 bit, then txbuf is
 * interpreted as a pointer to uint32_t. If word length is 16 bit, then txbuf
 * is interpreted as a pointer to uint16_t. etc.
 *
 * If txbuf is NULL then zero valued bits will be shifted out on MOSI. The
 * function returns as soon as the transmit queue is full or the requested
 * number of words have been installed.
 *
 * This function never blocks.
 *
 * Transfer properties are set with the function spi_config().
 *
 * return: number of words written to transmit queue, zero if none.
 */
int spi_write(struct spi_priv *priv, const void *txbuf, int count);

/*
 * Read elements from SPICTRL receive queue.
 *
 * The function tries to read count words of the configured word length from
 * the receive queue. Receive data is written to the lcoation or rxbuf. Each
 * word is reprensted by a type corresponding to the configured SPI word
 * length. Words stored in rxbuf are represented with its LSB at bit 0.
 *
 * NOTE: The interpreteation of rxbuf depends on the configured SPI word
 * length. For example, if configured word length is 32 bit, then rxbuf is
 * interpreted as a pointer to uint32_t. If word length is 16 bit, then rxbuf
 * is interpreted as a pointer to uint16_t. etc.
 *
 * If rxbuf is NULL then the MISO bits are not stored. The function returns as
 * soon as the receive queue is empty or the requested number of words have bee
 * read.
 *
 * This function never blocks.
 *
 * Transfer properties are set with the function spi_config().
 *
 * return: number of words read from receive queue, zero if none.
 */
int spi_read(struct spi_priv *priv, void *rxbuf, int count);

/* Send SPI words */
int spi_write32(struct spi_priv *priv, const uint32_t *txbuf, int count);

/* Receive SPI words */
int spi_read32(struct spi_priv *priv, uint32_t *rxbuf, int count);

/* Select slave index to communicate with, implementation dependent */
int spi_slave_select(struct spi_priv *priv, uint32_t mask);

/*
 * Returns device registers
 *
 * dev_no: the device number to return registers for
 *
 * return:
 * - Pointer to the device registers
 * - NULL if invalid device number
 */
const struct drv_devreg *spi_get_devreg(int dev_no);

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct spi_priv {
        volatile struct spictrl_regs *regs;
        int rxshift;
        int txshift;
        int reverse;
        int sync;
        int rxfree;
        int bytelen;
        int ssen;
        int asela;
        int sssz;
        unsigned int apbfreq;
        uint8_t open;
        SPIN_DECLARE(devlock)
};

struct spi_devcfg {
        struct drv_devreg regs;
        struct spi_priv priv;
};

#endif

