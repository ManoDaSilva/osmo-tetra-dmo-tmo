#include "timing.h"
#include "slotter.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct slotter_state *slotter1;

struct timing_state *timing_init()
{
	struct timing_state *s;
	s = calloc(1, sizeof(*s));
	s->slotter = slotter1;

	s->slot_time = 1e9 * 255 / 18000;
	s->ahead_time = 12.5e6;

	return s;
}


int timing_rx_burst(struct timing_state *s, const uint8_t *bits, int len, uint64_t ts)
{
	printf("%20lu: RX\n", ts);

	// Find the nearest timeslot by comparing to the transmit counters
	int64_t td_tx = ts - s->tx_slot;
	float slotdiff = (float)td_tx / (float)s->slot_time;
	int intdiff = roundf(slotdiff);
	int rx_slot = intdiff + s->tx_slot;
	// Modulo in C...
	rx_slot = ((rx_slot % TIMING_SLOTS) + TIMING_SLOTS) % TIMING_SLOTS;

	struct timing_slot tslot = {
		.time = ts,
		.diff = td_tx - (int64_t)intdiff * s->slot_time,
		.tn =  (rx_slot % 4)       + 1,
		.fn = ((rx_slot / 4) % 18) + 1,
		.mn =  (rx_slot / (4*18))  + 1,
		.hn = 0
	};

	slotter_rx_burst(s->slotter, bits, len, &tslot);

	return 0;
}


int timing_tx_burst(struct timing_state *s, uint8_t *bits, int maxlen, uint64_t *ts)
{
	int retlen = -1;
	uint64_t tnow = *ts;
	if (s->tx_time == 0) {
		// Initialize timing on first tick
		s->tx_time = tnow + s->ahead_time + s->slot_time;
	}

	const uint64_t tx_time = s->tx_time;
	int64_t tdiff = tnow - tx_time;
	if (tdiff >= -s->ahead_time) {
		const unsigned slot = s->tx_slot;

		struct timing_slot tslot = {
			.time = tx_time,
			.diff = 0,
			.tn =  (slot % 4)       + 1,
			.fn = ((slot / 4) % 18) + 1,
			.mn =  (slot / (4*18))  + 1,
			.hn = 0
		};

		retlen = slotter_tx_burst(s->slotter, bits, maxlen, &tslot);
		*ts = tx_time;

		// Go to the next slot
		s->tx_time = tx_time + s->slot_time;
		s->tx_slot = (slot + 1) % TIMING_SLOTS;
	}
	return retlen;
}


int timing_resync(struct timing_state *s, struct timing_slot *slot)
{
	unsigned next_slot =
		18 * 4 * (slot->mn - 1) +
		     4 * (slot->fn - 1) +
		         (slot->tn - 1);
	uint64_t next_time = slot->time;

	/* Find the first slot that doesn't cause tx_time to jump backwards */
	const int tx_time = s->tx_time;
	do {
		// Go to the next slot
		next_time += s->slot_time;
		next_slot = (next_slot + 1) % TIMING_SLOTS;
	} while ((int64_t)(next_time - tx_time) < 0);

	s->tx_time = next_time;
	s->tx_slot = next_slot;
	return 0;
}
