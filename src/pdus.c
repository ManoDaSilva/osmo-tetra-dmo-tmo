#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <osmocom/core/bitvec.h>
#include <osmocom/core/utils.h>

#include "pdus.h"


uint8_t pdu_sync_SCHS[8];		/* 60 bits */
uint8_t pdu_sync_SCHH[16];	    /* 124 bits */

uint8_t pdu_sync[8];		/* 60 bits */
uint8_t pdu_sysinfo[16];	/* 124 bits */
uint8_t pdu_mac_data[16];       /* 124 bits */
uint8_t pdu_acc_ass[2];
uint8_t pdu_acc_ass_18[2];

void dm_sync_pdu_schh(uint16_t address, uint8_t rep_mcc, uint16_t rep_mnc){
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHH;
	bv.data_len = sizeof(pdu_sync_SCHH);

    bitvec_set_uint(&bv, address, 10);	/* Repeater address */
    bitvec_set_uint(&bv, rep_mcc, 10);	    /* Repeater MNI-MCC */
    bitvec_set_uint(&bv, rep_mnc, 14);	    /* Repeater MNI-MNC */
	bitvec_set_uint(&bv, 3, 2);	    /* Validity time unit (3=not restricted) */
    bitvec_set_uint(&bv, 0, 6);	    /* Validity time unit count */
	bitvec_set_uint(&bv, 1, 3);	    /* Max DM-MS power class */
	bitvec_set_uint(&bv, 0, 1);	    /* Reserved */
	bitvec_set_uint(&bv, 0, 4);	    /* Usage restriction type (URT) */
    bitvec_set_uint(&bv, 0, 72);	/* URT addressing  */
	bitvec_set_uint(&bv, 0, 2);	    /* Reserved */

	// printf(" SCH/H PDU: %s\n", osmo_hexdump(pdu_sync_SCHH, sizeof(pdu_sync_SCHH)));
}

void dm_sync_pres_pdu_schs(uint8_t tn, uint8_t fn, uint8_t frame_countdown, uint8_t dn232_dn233, uint8_t dt254){
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHS;
	bv.data_len = sizeof(pdu_sync_SCHS);


	//bitvec_set_uint(&bv, 0, 4);	/* alignment */
	/* According to Table 21.73: SYNC PDU Contents */
	bitvec_set_uint(&bv, 13, 4);	/* System Code  */
	bitvec_set_uint(&bv, 1, 2);	    /* Sync PDU type */
	bitvec_set_uint(&bv, 1, 2);	    /* Communication Type */
    bitvec_set_uint(&bv, 0, 1);	    /* M-DMO flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Reserved */
    bitvec_set_uint(&bv, 0, 1);	    /* Two-frequency repeater flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Repeater operating modes */
    bitvec_set_uint(&bv, 0, 6);	    /* Spacing of uplink */
    bitvec_set_uint(&bv, 0, 1);	    /* Master/slave link flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Channel usage */
    bitvec_set_uint(&bv, 0, 2);	    /* Channel state */
    bitvec_set_uint(&bv, tn, 2);	/* Slot number */
	bitvec_set_uint(&bv, fn, 5);	/* Frame number */
    bitvec_set_uint(&bv, 1, 3);	    /* Power class */
    bitvec_set_uint(&bv, 1, 1);	    /* Power control flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Reserved */
    bitvec_set_uint(&bv, frame_countdown, 2);	/* Frame countdown */
    bitvec_set_uint(&bv, 0, 2);	    /* Reserved / priority */
    bitvec_set_uint(&bv, 0, 6);	    /* Reserved */
    bitvec_set_uint(&bv, dn232_dn233, 4);	    /* Values of DN232 and DN233 */
    bitvec_set_uint(&bv, dt254, 3); /* Value of DT254 */
    bitvec_set_uint(&bv, 0, 1);	    /* Presence signal dual watch sync flag */
    bitvec_set_uint(&bv, 0, 5);	    /* Reserved */

	// printf("DPRES-SYNC SCH/S PDU: %s ", osmo_hexdump(pdu_sync_SCHS, sizeof(pdu_sync_SCHS)));

}



void sync_pdu(uint16_t cc, uint8_t mn, uint8_t fn, uint8_t tn, uint8_t mcc, uint16_t mnc)
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync;
	bv.data_len = sizeof(pdu_sync);

	//fn = (fn % 18) + 1;
	//mn = (mn % 60) + 1;

	//printf("tn: %u fn: %u mn: %u\n", tn, fn, mn);

	// According to Table 21.73: SYNC PDU Contents
	bitvec_set_uint(&bv, 2, 4);	// System Code: ETS 300 392-2 ed. 1
	bitvec_set_uint(&bv, cc, 6);	// Colour Code: Predefined Scrambling
	bitvec_set_uint(&bv, tn, 2);	// Timeslot number
	bitvec_set_uint(&bv, fn, 5);	// Frame number
	bitvec_set_uint(&bv, mn, 6);	// Multiframe number
	bitvec_set_uint(&bv, 0, 2);	// Sharing mode: continuous transmission
	bitvec_set_uint(&bv, 0, 3);	// TS reserved frames: 1 frame reserved per 2 multiframes
	bitvec_set_bit(&bv, 0);		// U-plane DTX: Discontinuous U-plane transmission is not allowed
	bitvec_set_bit(&bv, 0);		// No Frame 18 extension
	bitvec_set_bit(&bv, 0);		// Reserved
	// As defined in Table 18.4.2.1: D-MLE-SYNC
	bitvec_set_uint(&bv, mcc, 10);	// MCC
	bitvec_set_uint(&bv, mnc, 14);	// MNC
	bitvec_set_uint(&bv, 2, 2);	// Neighbor cell broadcast: not supported
	bitvec_set_uint(&bv, 0, 2);	// Cell service level: unknown
	bitvec_set_bit(&bv, 1);		// Late entry information
	//printf("SYNC PDU: %s\n", osmo_hexdump(pdu_sync, sizeof(pdu_sync)));
}

void sysinfo_pdu(uint16_t hn)
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sysinfo;
	bv.data_len = sizeof(pdu_sysinfo);

	// According to Table 21.4.4.1: SYSINFO PDU contents
	bitvec_set_uint(&bv, 2, 2);	// MAC PDU type: Broadcast
	bitvec_set_uint(&bv, 0, 2);	// SYSINFO PDU
	bitvec_set_uint(&bv, ((438275-400000)/25), 12);	// Main carrier
	bitvec_set_uint(&bv, 4, 4);	// Frequency band: 390/400
	bitvec_set_uint(&bv, 0, 2);	// Offset: No offset
	bitvec_set_uint(&bv, 7, 3);	// Duplex Spacing (Table 2 on ETSI TS 100 392-15): outside the standards
	bitvec_set_bit(&bv, 0);		// Normal operation
	bitvec_set_uint(&bv, 0, 2);	// Number of CSCH: none
	bitvec_set_uint(&bv, 5, 3);	// MS_TXPWR_MAX_CELL
	bitvec_set_uint(&bv, 4, 4);	// RXLEV_ACCESS_MIN: -125dBm
	bitvec_set_uint(&bv, 9, 4);	// ACCESS_PARAMETER: -53 dBm
	bitvec_set_uint(&bv, 4, 4);	// RADIO_DOWNLINK_TIMEOUT: Disable
	bitvec_set_bit(&bv, 0);		// Hyperframe number follows
	bitvec_set_uint(&bv, hn, 16);	// Hyperframe number
	bitvec_set_uint(&bv, 2, 2);	// Optional field: Even multiframe

	bitvec_set_uint(&bv, 15, 4);	// IMM: Immediate Access Allowed
	bitvec_set_uint(&bv, 4, 4);	// WT: Response within 4 downlink opportunities
	bitvec_set_uint(&bv, 3, 4);	// Number of random accesses
	bitvec_set_bit(&bv, 0);		// Frame length factor
	bitvec_set_uint(&bv, 0, 4);	// Timeslot pointer
	bitvec_set_uint(&bv, 0, 3);	// Minimum priority

	// TM-SDU (42 bit), Section 18.4.2.2, Table 18.15
	bitvec_set_uint(&bv, 1112, 14);	// Location Area (18.5.9)
	bitvec_set_uint(&bv, 0xFFFF, 16);	// Subscriber Class (18.5.22)

	// BS service details (12 bits)
	bitvec_set_bit(&bv, 0);	        // Registration mandatory on this cell
	bitvec_set_bit(&bv, 0);	        // De-registration mandatory on this cell
	bitvec_set_bit(&bv, 0);	        // Priority cell
	bitvec_set_bit(&bv, 1);	        // Minimum mode service
	bitvec_set_bit(&bv, 0);	        // Migration
	bitvec_set_bit(&bv, 1);	        // System wide services
	bitvec_set_bit(&bv, 1);	        // TETRA voice service
	bitvec_set_bit(&bv, 1);	        // Circuit mode data service
	bitvec_set_bit(&bv, 0);	        // Reserved
	bitvec_set_bit(&bv, 1);	        // SNDCP service
	bitvec_set_bit(&bv, 0);	        // Air interface encryption service
	bitvec_set_bit(&bv, 1);	        // Advanced link supported
	//bitvec_set_uint(&bv, 0xd75, 12);	// same as above
	//printf("SYSINFO PDU: %s\n", osmo_hexdump(pdu_sysinfo, sizeof(pdu_sysinfo)));
}

/* MAC-DATA PDU */
void mac_data_pdu()
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_mac_data;
	bv.data_len = sizeof(pdu_mac_data);

	// According to clause 21.4.2.3, Table 326: MAC-DATA PDU contents
	bitvec_set_uint(&bv, 0, 2);	// PDU type: MAC-DATA PDU
	bitvec_set_bit(&bv, 0);		// Fill bit indication: No fill bit present
	bitvec_set_bit(&bv, 0);		// Encrypted flag: Not encrypted
	bitvec_set_uint(&bv, 0, 2);	// Address type
	bitvec_set_uint(&bv, 0x42000, 24);	// Address

	//printf("MAC-DATA PDU: %s\n", osmo_hexdump(pdu_mac_data, sizeof(pdu_mac_data));
}

/* MAC-RESOURCE PDU */
void mac_resource_pdu(uint8_t *pdu, uint8_t pdu_len)
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu;
	bv.data_len = pdu_len;

	// According to clause 21.4.3.1, Table 329: MAC-RESOURCE PDU contents
	bitvec_set_uint(&bv, 0, 2);	// PDU type: MAC-RESOURCE PDU
	bitvec_set_bit(&bv, 0);		// Fill bit indication: No fill bit present
	bitvec_set_uint(&bv, 0, 2);	// Encrypted flag: Not encrypted
	bitvec_set_bit(&bv, 0);		// Random access flag: Undefined
	bitvec_set_uint(&bv, 0, 6);	// !!! TODO !!! Length indication:
	bitvec_set_uint(&bv, 1, 3);	// Address type: SSI
	bitvec_set_uint(&bv, 0, 30);	// Address: SSI, USSI or SMI
	bitvec_set_bit(&bv, 0);		// Power control flag: No power control element in PDU
	bitvec_set_bit(&bv, 0);		// Slot granting flag: No slot granting elements in PDU
	bitvec_set_bit(&bv, 0);		// Channel allocation flag: No channel allocation element in PDU

	//printf("MAC-RESOURCE PDU: %s\n", osmo_hexdump(pdu_mac_resource, sizeof(pdu_mac_resource));
}

/* ACCESS-ASSIGN PDU contents for frame 1 to 17 */
void acc_pdu(uint8_t af1, uint8_t af2)
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_acc_ass;
	bv.data_len = sizeof(pdu_acc_ass);

	uint8_t du = 3;
	if (af1 && af2) du = 1;

	bitvec_set_uint(&bv, 0, 2);	// alignment (<<2)
	bitvec_set_uint(&bv, du, 2);	// DL/UL: defined by field1/2
	bitvec_set_uint(&bv, af1, 6);	// Access field 1: Unallocated
	bitvec_set_uint(&bv, af2, 6);	// Access field 2: Unallocated

	//printf("ACCESS-ASSIGN PDU: %s\n", osmo_hexdump(pdu_acc_ass, sizeof(pdu_acc_ass)));
}

/* ACCESS-ASSIGN PDU contents for frame 18 */
/* Table 21.83: ACCESS-ASSIGN PDU contents for frame 18 */
void acc_pdu_18()
{
	struct bitvec bv;
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_acc_ass_18;
	bv.data_len = sizeof(pdu_acc_ass_18);

	/*
	bitvec_set_uint(&bv, 0, 2);	// alignment (<<2)
	bitvec_set_uint(&bv, 0, 2);	// DL/UL: common only
	bitvec_set_uint(&bv, 0, 6);	// Access field 1 (not used)
	bitvec_set_uint(&bv, 0, 6);	// Access field 2 (not used)
	*/

	bitvec_set_uint(&bv, 0, 2);	// alignment (<<2)
	bitvec_set_uint(&bv, 0, 2);	// Uplink access rights: common
	bitvec_set_uint(&bv, 1, 6);	// Access field 1
	bitvec_set_uint(&bv, 0, 6);	// Access field 2

	//printf("ACCESS-ASSIGN PDU: %s\n", osmo_hexdump(pdu_acc_ass_18, sizeof(pdu_acc_ass_18)));
}