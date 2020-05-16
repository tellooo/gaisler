/*
 *  GRLIB common CAN baud-rate to timing parameters conversion
 *
 *  COPYRIGHT (c) 2019.
 *  Cobham Gaisler AB.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#include <grlib/canbtrs.h>

/*#define GRLIB_CANBTRS_DEBUG*/

/* Calculate CAN baud-rate generation parameters from requested baud-rate */
int grlib_canbtrs_calc_timing(
	unsigned int baud,
	unsigned int core_hz,
	unsigned int sampl_pt,
	struct grlib_canbtrs_ranges *br,
	struct grlib_canbtrs_timing *timing
	)
{
	int best_error = 2000000000, best_tseg=0, best_scaler=0;
	int tseg=0, tseg1=0, tseg2=0, sc, error;

	/* Default to 80% sample point */
	if ((sampl_pt < 50) || (sampl_pt > 99))
		sampl_pt = 80;

	/* step though all TSEG1+TSEG2 values possible */
	for (tseg = (br->min_tseg1 + br->min_tseg2);
	     tseg <= (br->max_tseg1 + br->max_tseg2);
	     tseg++) {
		/* calculate scaler */
		sc = core_hz / ((br->divfactor + tseg) * baud);
		if (sc <= 0 || sc > br->max_scaler)
			continue;
		if (br->has_bpr &&
		    (((sc > 256 * 1) && (sc <= 256 * 2) && (sc & 0x1)) ||
		    ((sc > 256 * 2) && (sc <= 256 * 4) && (sc & 0x3)) ||
		    ((sc > 256 * 4) && (sc <= 256 * 8) && (sc & 0x7))))
			continue;

		error = baud - core_hz / (sc * (br->divfactor + tseg));
#ifdef GRLIB_CANBTRS_DEBUG
		printf("  baud=%d, tseg=%d, sc=%d, error=%d\n",
		       baud, tseg, sc, error);
#endif
		if (error < 0)
			error = -error;

		/* tseg is increasing, so we accept higher tseg with the same
		 * baudrate to get better sampling point.
		 */
		if (error <= best_error) {
			best_error = error;
			best_tseg = tseg;
			best_scaler = sc;
#ifdef GRLIB_CANBTRS_DEBUG
			printf("  ! best baud=%d\n",
			       core_hz/(sc * (br->divfactor + tseg)));
#endif
		}
	}

	/* return an error if 5% off baud-rate */
	if (best_error && (baud / best_error <= 5)) {
		return -2;
	} else if (!timing) {
		return 0; /* nothing to store result in, but a valid bitrate can be calculated */
	}

	tseg2 = (best_tseg + br->divfactor) -
	        ((sampl_pt * (best_tseg + br->divfactor)) / 100);
	if (tseg2 < br->min_tseg2) {
		tseg2 = br->min_tseg2;
	} else if (tseg2 > br->max_tseg2) {
		tseg2 = br->max_tseg2;
	}

	tseg1 = best_tseg - tseg2;
	if (tseg1 > br->max_tseg1) {
		tseg1 = br->max_tseg1;
		tseg2 = best_tseg - tseg1;
	} else if (tseg1 < br->min_tseg1) {
		tseg1 = br->min_tseg1;
		tseg2 = best_tseg - tseg1;
	}

	/* Get scaler and BPR from pseudo SCALER clock */
	if (best_scaler <= 256) {
		timing->scaler = best_scaler - 1;
		timing->bpr = 0;
	} else if (best_scaler <= 256 * 2) {
		timing->scaler = ((best_scaler + 1) >> 1) - 1;
		timing->bpr = 1;
	} else if (best_scaler <= 256 * 4) {
		timing->scaler = ((best_scaler + 1) >> 2) - 1;
		timing->bpr = 2;
	} else {
		timing->scaler = ((best_scaler + 1) >> 3) - 1;
		timing->bpr = 3;
	}

	timing->ps1    = tseg1;
	timing->ps2    = tseg2;
	timing->rsj    = 1;

#ifdef GRLIB_CANBTRS_DEBUG
	printf("  ! result: sc=%d,bpr=%d,ps1=%d,ps2=%d\n", timing->scaler, timing->bpr, timing->ps1, timing->ps2);
#endif

	return 0;
}
