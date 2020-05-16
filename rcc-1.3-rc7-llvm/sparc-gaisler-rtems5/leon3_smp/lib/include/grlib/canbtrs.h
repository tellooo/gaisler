/**
 * @file
 * @ingroup can
 * @brief CAN baud-rate paramter operations for OCCAN/GRCAN/GRCANFD controllers
 */

/*
 *  COPYRIGHT (c) 2019.
 *  Cobham Gaisler AB.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#ifndef __GRLIB_CANBTRS_H__
#define __GRLIB_CANBTRS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* CAN Baud-rate parameters, range of valid parameter values */
struct grlib_canbtrs_ranges {
	unsigned int max_scaler;
	char has_bpr;
	unsigned char divfactor;
	unsigned char min_tseg1;
	unsigned char max_tseg1;
	unsigned char min_tseg2;
	unsigned char max_tseg2;
};

struct grlib_canbtrs_timing {
	unsigned char scaler;
	unsigned char ps1;
	unsigned char ps2;
	unsigned char rsj;
	unsigned char bpr;
};

/* Calculate CAN baud-rate generation parameters from requested baud-rate
 *
 * \param baud       The CAN baud rate requested
 * \param core_hz    Input clock [Hz] to CAN core
 * \param sampl_pt   CAN sample point in %, 80 means 80%
 * \param br         CAN Baud-rate parameters limitations
 * \param timing     result is placed here
 */
int grlib_canbtrs_calc_timing(
	unsigned int baud,
	unsigned int core_hz,
	unsigned int sampl_pt,
	struct grlib_canbtrs_ranges *br,
	struct grlib_canbtrs_timing *timing
	);

#ifdef __cplusplus
}
#endif

#endif
