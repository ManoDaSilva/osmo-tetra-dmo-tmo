/*  This class contains functions for PDU generation */

#include "tetra_common.h"
#include "tetra_dll_pdu.h"
#include "hamtetra_pdu_generator.h"
#include <lower_mac/tetra_lower_mac.h>

int build_pdu_dpres_sync_schs(uint8_t fn, uint8_t tn, enum tdma_master_slave_link_flag dir, uint8_t frame_countdown, uint8_t channel_state, uint8_t *out)
{
    uint8_t pdu_sync_SCHS[8];		/* 60 bits */
    struct bitvec bv;

    memset(&pdu_sync_SCHS, 0, sizeof(pdu_sync_SCHS));
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHS;
	bv.data_len = sizeof(pdu_sync_SCHS);

    uint8_t dn232_dn233 = DN233 | (DN232 << 2);

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
    bitvec_set_uint(&bv, dir, 1);	    /* Master/slave link flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Channel usage */
    bitvec_set_uint(&bv, channel_state, 2);	    /* Channel state */
    bitvec_set_uint(&bv, tn, 2);	/* Slot number */
	bitvec_set_uint(&bv, fn, 5);	/* Frame number */
    bitvec_set_uint(&bv, 1, 3);	    /* Power class */
    bitvec_set_uint(&bv, 1, 1);	    /* Power control flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Reserved */
    bitvec_set_uint(&bv, frame_countdown, 2);	/* Frame countdown */
    bitvec_set_uint(&bv, 0, 2);	    /* Reserved / priority */
    bitvec_set_uint(&bv, 0, 6);	    /* Reserved */
    bitvec_set_uint(&bv, dn232_dn233, 4);	    /* Values of DN232 and DN233 */
    bitvec_set_uint(&bv, DT254, 3); /* Value of DT254 */
    bitvec_set_uint(&bv, 0, 1);	    /* Presence signal dual watch sync flag */
    bitvec_set_uint(&bv, 0, 5);	    /* Reserved */

	// printf("DPRES-SYNC SCH/S PDU: %s ", osmo_hexdump(pdu_sync_SCHS, sizeof(pdu_sync_SCHS)));
   	uint8_t sb_type2[80];
   	uint8_t sb_type5[120];

	memset(sb_type2, 0, sizeof(sb_type2));
   	osmo_pbit2ubit(sb_type2, pdu_sync_SCHS, 60);
    build_encoded_block_sch(DPSAP_T_SCH_S, sb_type2, out);

    return 0;
}

int build_pdu_dpres_sync_schh(uint8_t fn, uint8_t tn, uint8_t frame_countdown, uint8_t *out)
{
    uint8_t pdu_sync_SCHH[16];	    /* 124 bits */
    struct bitvec bv;

    memset(&pdu_sync_SCHH, 0, sizeof(pdu_sync_SCHH));
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHH;
	bv.data_len = sizeof(pdu_sync_SCHH);

    bitvec_set_uint(&bv, REP_ADDRESS, 10);	/* Repeater address */
    bitvec_set_uint(&bv, REP_MCC, 10);	    /* Repeater MNI-MCC */
    bitvec_set_uint(&bv, REP_MNC, 14);	    /* Repeater MNI-MNC */
	bitvec_set_uint(&bv, 3, 2);	    /* Validity time unit (3=not restricted) */
    bitvec_set_uint(&bv, 0, 6);	    /* Validity time unit count */
	bitvec_set_uint(&bv, 1, 3);	    /* Max DM-MS power class */
	bitvec_set_uint(&bv, 0, 1);	    /* Reserved */
	bitvec_set_uint(&bv, 0, 4);	    /* Usage restriction type (URT) */
    bitvec_set_uint(&bv, 0, 72);	/* URT addressing  */
	bitvec_set_uint(&bv, 0, 2);	    /* Reserved */

	// printf(" SCH/H PDU: %s\n", osmo_hexdump(pdu_sync_SCHH, sizeof(pdu_sync_SCHH)));
   	uint8_t si_type2[140];
	uint8_t si_type5[216];

	memset(si_type2, 0, sizeof(si_type2));
	osmo_pbit2ubit(si_type2, pdu_sync_SCHH, 124);
    build_encoded_block_sch(DPSAP_T_SCH_H, si_type2, out);

    return 0;
}


int build_pdu_dpress_sync(uint8_t fn, uint8_t tn, enum tdma_master_slave_link_flag dir, uint8_t frame_countdown, uint8_t channel_state, uint8_t *out)
{
	uint8_t sb_type5[120];
	uint8_t si_type5[216];

    build_pdu_dpres_sync_schs(fn, tn, dir, frame_countdown, channel_state, sb_type5);
    build_pdu_dpres_sync_schh(fn, tn, frame_countdown, si_type5);

    int len = build_dm_sync_burst(out, sb_type5, si_type5);
    printf("DPRES-SYNC burst: %s\n", osmo_ubit_dump(out, len));
    return len;
}

int build_pdu_dpress_sync_gate(uint8_t fn, uint8_t tn, enum tdma_master_slave_link_flag dir, uint8_t frame_countdown, uint8_t *out)
{
    uint8_t pdu_sync_SCHS[8];		/* 60 bits */
    uint8_t pdu_sync_SCHH[16];	    /* 124 bits */

    struct bitvec bv;

    memset(&pdu_sync_SCHS, 0, sizeof(pdu_sync_SCHS));
    memset(&pdu_sync_SCHH, 0, sizeof(pdu_sync_SCHH));
	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHS;
	bv.data_len = sizeof(pdu_sync_SCHS);

    uint8_t dn232_dn233 = DN233 | (DN232 << 2);

	//bitvec_set_uint(&bv, 0, 4);	/* alignment */
	/* According to Table 21.73: SYNC PDU Contents */
	bitvec_set_uint(&bv, 13, 4);	/* System Code  */
	bitvec_set_uint(&bv, 1, 2);	    /* Sync PDU type */
	bitvec_set_uint(&bv, 3, 2);	    /* Communication Type DM-REP/GATE */
    bitvec_set_uint(&bv, 0, 1);	    /* M-DMO flag */
    bitvec_set_uint(&bv, 1, 1);	    /* SwMI availability flag */
    bitvec_set_uint(&bv, 1, 1);	    /* DM-REP function flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Two-frequency repeater flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Repeater operating modes */
    bitvec_set_uint(&bv, 0, 6);	    /* Spacing of uplink */
    bitvec_set_uint(&bv, dir, 1);	    /* Master/slave link flag */
    bitvec_set_uint(&bv, 0, 2);	    /* Channel usage */
    bitvec_set_uint(&bv, 3, 2);	    /* Channel state */
    bitvec_set_uint(&bv, tn, 2);	/* Slot number */
	bitvec_set_uint(&bv, fn, 5);	/* Frame number */
    bitvec_set_uint(&bv, 1, 3);	    /* Power class */
    bitvec_set_uint(&bv, 1, 1);	    /* Power control flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Registration phase terminated  */
    bitvec_set_uint(&bv, frame_countdown, 2);	/* Frame countdown */
    bitvec_set_uint(&bv, 0, 2);	    /* Reserved / Timing for DM-REP / priority */
    bitvec_set_uint(&bv, 2, 2);	    /* Registrations permited */
    bitvec_set_uint(&bv, 0, 4);	    /* Registration label */
    bitvec_set_uint(&bv, 4, 4);	    /* Registration phase time remaining */
    // bitvec_set_uint(&bv, dn232_dn233, 4);	    /* Values of DN232 and DN233 */
    bitvec_set_uint(&bv, 1, 3); /* Value of DT254 / registration access parameter */
    // bitvec_set_uint(&bv, DT254, 3); /* Value of DT254 / registration access parameter */
    bitvec_set_uint(&bv, 0, 1);	    /* Registrations forwarded flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Gateway encryption state */
    bitvec_set_uint(&bv, 0, 1);	    /* System wide services not available */
    bitvec_set_uint(&bv, 0, 3);	    /* Reserved */

	// printf("DPRES-SYNC SCH/S PDU: %s ", osmo_hexdump(pdu_sync_SCHS, sizeof(pdu_sync_SCHS)));
   	uint8_t sb_type2[80];
   	uint8_t sb_type5[120];

	memset(sb_type2, 0, sizeof(sb_type2));
   	osmo_pbit2ubit(sb_type2, pdu_sync_SCHS, 60);
    build_encoded_block_sch(DPSAP_T_SCH_S, sb_type2, sb_type5);


	memset(&bv, 0, sizeof(bv));
	bv.data = pdu_sync_SCHH;
	bv.data_len = sizeof(pdu_sync_SCHH);

    bitvec_set_uint(&bv, REP_ADDRESS, 10);	/* Repeater address */
    bitvec_set_uint(&bv, REP_MCC, 10);	    /* Repeater MNI-MCC */
    bitvec_set_uint(&bv, REP_MNC, 14);	    /* Repeater MNI-MNC */
	bitvec_set_uint(&bv, 3, 2);	    /* Validity time unit (3=not restricted) */
    bitvec_set_uint(&bv, 0, 6);	    /* Validity time unit count */
	bitvec_set_uint(&bv, 1, 3);	    /* Max DM-MS power class */
	bitvec_set_uint(&bv, 0, 1);	    /* Reserved */
	bitvec_set_uint(&bv, 0, 4);	    /* Usage restriction type (URT) */
    bitvec_set_uint(&bv, 0, 72);	/* URT addressing  */
	bitvec_set_uint(&bv, 0, 2);	    /* Reserved */

	// printf(" SCH/H PDU: %s\n", osmo_hexdump(pdu_sync_SCHH, sizeof(pdu_sync_SCHH)));
   	uint8_t si_type2[140];
	uint8_t si_type5[216];

	memset(si_type2, 0, sizeof(si_type2));
	osmo_pbit2ubit(si_type2, pdu_sync_SCHH, 124);
    build_encoded_block_sch(DPSAP_T_SCH_H, si_type2, si_type5);

    int len = build_dm_sync_burst(out, sb_type5, si_type5);
    printf("DPRES-SYNC burst: %s\n", osmo_ubit_dump(out, len));
    return len;
}

int build_pdu_dmac_sync_schs(struct tetra_dmo_pdu_dmac_sync *dmac_sync, enum tdma_master_slave_link_flag link_flag, uint8_t fn, uint8_t tn, uint8_t frame_countdown, uint8_t *out)
{
    uint8_t pdu_sync_SCHS[8];		/* 60 bits */
    uint8_t sb_type2[80];

    struct bitvec bv;

    memset(&pdu_sync_SCHS, 0, sizeof(pdu_sync_SCHS));
    memset(&bv, 0, sizeof(bv));
    bv.data = pdu_sync_SCHS;
    bv.data_len = sizeof(pdu_sync_SCHS);

    bitvec_set_uint(&bv, dmac_sync->system_code, 4);	/* System Code  */
    bitvec_set_uint(&bv, 0, 2);	    /* Sync PDU type */
    bitvec_set_uint(&bv, 1, 2);	    /* Communication Type */
    bitvec_set_uint(&bv, link_flag, 1);	    /* Master/slave link flag */
    bitvec_set_uint(&bv, 0, 1);	    /* Gateway generated message flag */
    bitvec_set_uint(&bv, 0, 2);	    /* A/B channel usage */
    bitvec_set_uint(&bv, tn-1, 2);	/* Slot number */
    bitvec_set_uint(&bv, fn, 5);	/* Frame number */
    bitvec_set_uint(&bv, dmac_sync->airint_encryption_state, 2);   /* Air interface encryption state */
    bitvec_set_uint(&bv, 0, 39);	    /* Reserved, encryption not yet supported */

    osmo_pbit2ubit(out, pdu_sync_SCHS, 60);

    return bv.data_len;

}

int build_pdu_dmac_sync_schh(struct tetra_dmo_pdu_dmac_sync *dmac_sync, uint8_t fn, uint8_t tn, uint8_t frame_countdown, uint8_t *out)
{

    uint8_t pdu_sync_SCHH[16];	    /* 124 bits */
    uint8_t si_type2[140];

    struct bitvec bv;
    memset(&pdu_sync_SCHH, 0, sizeof(pdu_sync_SCHH));
    memset(&bv, 0, sizeof(bv));
    bv.data = pdu_sync_SCHH;
    bv.data_len = sizeof(pdu_sync_SCHH);

    bitvec_set_uint(&bv, dmac_sync->repgw_address, 10);	/* Repeater address */
    bitvec_set_uint(&bv, dmac_sync->fillbit_indication, 1);	    /* Fillbit indication */
    bitvec_set_uint(&bv, dmac_sync->fragmentation_flag, 1);	    /* Fragment flag */
    bitvec_set_uint(&bv, frame_countdown, 2);	    /* frame countdown */
    bitvec_set_uint(&bv, dmac_sync->dest_address_type, 2);	    /* destination address type */
    bitvec_set_uint(&bv, dmac_sync->dest_address, 24);	    /* destination address */
    bitvec_set_uint(&bv, dmac_sync->src_address_type, 2);	    /* source address type */
    bitvec_set_uint(&bv, dmac_sync->src_address, 24);	    /* source address */
    bitvec_set_uint(&bv, dmac_sync->mni, 24);	/* MNI  */
    bitvec_set_uint(&bv, dmac_sync->message_type, 5);	    /* Message type */
    // bitvec_set_uint(&bv, dmac_sync->message_fields, pdu_dmac_sync->message_fields_len);	    /* Message dependent elements */
    // bitvec_add_array(&bv, dmac_sync->message_fields, pdu_dmac_sync->message_fields_len, false, 8);
    for (int i=0; i<dmac_sync->message_fields_len; i++) {
        bitvec_set_uint(&bv, dmac_sync->message_fields[i], 1);
    }
    // bitvec_set_uint(&bv, pdu_dmac_sync->dm_sdu, pdu_dmac_sync->dm_sdu_len);	    /* DM-SDU */
    // bitvec_add_array(&bv, pdu_dmac_sync->dm_sdu, pdu_dmac_sync->dm_sdu_len, false, 8);
    for (int i=0; i<dmac_sync->dm_sdu_len; i++) {
        bitvec_set_uint(&bv, dmac_sync->dm_sdu[i], 1);
    }
    if (dmac_sync->fillbit_indication==1) {
        bitvec_set_uint(&bv, 1, 1);
    }

    // fill the rest with zeroes
    for(int i=bv.cur_bit; i<124; i++) {
        bitvec_set_uint(&bv, 0, 1);
    }

    osmo_pbit2ubit(out, pdu_sync_SCHH, 124);

    return bv.data_len;

}


/* Build a full 'Normal continuous downlink burst'
 * from MAC-DATA PDU in SCH/HD and SYSINFO PDU in BNCH 
 9.4.4.2.5
 1 to 12: Normal Training Sequence 3
 13 to 14: Phase adjustment Bits 
 15 to 230: Scrambled Block 1 Bits 
 231 to 244: Scrambled broadcast 
 245 to 266: Normal training sequence 
 267 to 282: Scrambled broadcast bits
 283 to 498: Scrambled Block 2 bits 
 499 to 500: Phase adjustment bits 
 501 to 510: Normal training sequence 3
 */
int build_ncdb(uint8_t *buf)
{
	//Sub-block 1 Init
	uint8_t sb1_type2[144];
	uint8_t sb1_master[216*4];
	uint8_t sb1_type3[216];
	uint8_t sb1_type4[216];
	uint8_t sb1_type5[216];

	//Sub-block 2 Init
	uint8_t sb2_type2[144];
	uint8_t sb2_master[216*4];
	uint8_t sb2_type3[216];
	uint8_t sb2_type4[216];
	uint8_t sb2_type5[216];

	//Broadcast block Init
	uint8_t bb_type5[30];
	uint16_t crc;
	uint8_t *cur;
	uint32_t bb_rm3014, bb_rm3014_be;

	uint32_t scramb_init = tetra_scramb_get_init(TM_MCC, TM_MNC, TM_CC);


	//SUB-BLOCK 1
	memset(sb1_type2, 0, sizeof(sb1_type2));
	cur = sb1_type2;

	// Use MAC-DATA PDU from pdus.c
	cur += osmo_pbit2ubit(sb1_type2, pdu_mac_data, 124);
	//printf("MAC-DATA PDU: %s\n", osmo_ubit_dump(sb1_type2, 124));

	// Run it through CRC16-CCITT
	crc = ~crc16_ccitt_bits(sb1_type2, 124);
	crc = swap16(crc);
	cur += osmo_pbit2ubit(cur, (uint8_t *) &crc, 16);

	// Append 4 tail bits: type-2 bits
	cur += 4;

	/* Run rate 2/3 RCPC code: type-3 bits */
	{
		struct conv_enc_state *ces = calloc(1, sizeof(*ces));
		conv_enc_init(ces);
		conv_enc_input(ces, sb1_type2, 144, sb1_master);
		get_punctured_rate(TETRA_RCPC_PUNCT_2_3, sb1_master, 216, sb1_type3);
		free(ces);
	}

	// Run (216,101) block interleaving: type-4 bits
	block_interleave(216, 101, sb1_type3, sb1_type4);

	memcpy(sb1_type5, sb1_type4, 216);

	// Run scrambling (all-zero): type-5 bits
	tetra_scramb_bits(scramb_init, sb1_type5, 216);
	//printf("Scrambled block 1 bits (SCH/HD): %s\n", osmo_ubit_dump(sb1_type5, 216));


	//SUB-BLOCK 2
	memset(sb2_type2, 0, sizeof(sb2_type2));
	cur = sb2_type2;

	// Use pdu_sysinfo from pdus.c
	cur += osmo_pbit2ubit(sb2_type2, pdu_sysinfo, 124);
	//memcpy(sb2_type2,pdu_sysinfo_entropia,124);
	//cur +=124;
	// Run it through CRC16-CCITT
	crc = ~crc16_ccitt_bits(sb2_type2, 124);
	crc = swap16(crc);
	cur += osmo_pbit2ubit(cur, (uint8_t *) &crc, 16);

	// Append 4 tail bits: type-2 bits
	cur += 4;


	// Run rate 2/3 RCPC code: type-3 bits
	{
		struct conv_enc_state *ces = calloc(1, sizeof(*ces));
		conv_enc_init(ces);
		conv_enc_input(ces, sb2_type2, 144, sb2_master);
		get_punctured_rate(TETRA_RCPC_PUNCT_2_3, sb2_master, 216, sb2_type3);
		free(ces);
	}

	// Run (216,101) block interleaving: type-4 bits
	block_interleave(216, 101, sb2_type3, sb2_type4);

	memcpy(sb2_type5, sb2_type4, 216);

	// Run scrambling (all-zero): type-5 bits
	tetra_scramb_bits(scramb_init, sb2_type5, 216);
	//printf("Scrambled block 2 bits (BNCH): %s\n", osmo_ubit_dump(sb2_type5, 216));
	
	
	//BROADCAST BITS
	// Use pdu_acc_ass from pdus.c
	uint8_t *bb_type1 = (uint8_t *)pdu_acc_ass; // ACCESS-ASSIGN
	// Run it through (30,14) RM code: type-2=3=4 bits
	bb_rm3014 = tetra_rm3014_compute(*(bb_type1) << 8 | *(bb_type1 + 1));
	// convert to big endian
	bb_rm3014_be = htonl(bb_rm3014);
	// shift two bits left as it is only a 30 bit value
	bb_rm3014_be <<= 2;
	osmo_pbit2ubit(bb_type5, (uint8_t *) &bb_rm3014_be, 30);

	// Run scrambling (all-zero): type-5 bits
	tetra_scramb_bits(scramb_init, bb_type5, 30);

	//printf("Scrambled broadcast bits (AACH): %s\n", osmo_ubit_dump(bb_type5, 30));

	// Finally, hand it into the physical layer
	build_norm_c_d_burst(buf, sb1_type5, bb_type5, sb2_type5, 1);

	//printf("Normal continuous downlink burst (NCDB): %s\n", osmo_ubit_dump(buf, BLEN));
	return BLEN;
}

/* Build a full 'Synchronization continuous downlink burst' from SYSINFO-PDU and SYNC-PDU */
/* 9.4.4.2.6
 1 to 12: Normal Training Sequence 3
 13 to 14: Phase adjustment Bits 
 15 to 94: Freq Correction
 95 to 214: Scrambled synchronization block 1 bits
 215 to 252: Synchronization training sequence 
 253 to 282: Scrambled broadcast bits 
 283 to 498: Scrambled block 2 bits 
 499 to 500: Phase adjustment bits 
 501 to 510: Normal training sequence 3 
 */
int build_scdb(uint8_t *buf, const uint8_t fn)
{
	//Sub-block 1 sync Init
	uint8_t sb_type2[80];
	uint8_t sb_master[80*4];
	uint8_t sb_type3[120];
	uint8_t sb_type4[120];
	uint8_t sb_type5[120];

	//Sub-block 2 Init
	uint8_t si_type2[144];
	uint8_t si_master[216*4];
	uint8_t si_type3[216];
	uint8_t si_type4[216];
	uint8_t si_type5[216];

	//Broadcast block Init
	uint8_t bb_type5[30];
	uint16_t crc;
	uint8_t *cur;
	uint32_t bb_rm3014, bb_rm3014_be;

	uint32_t scramb_init = tetra_scramb_get_init(TM_MCC, TM_MNC, TM_CC);

	//SUB-BLOCK 1 (Sync)
	memset(sb_type2, 0, sizeof(sb_type2));
	cur = sb_type2;

	/* Use pdu_sync from pdus.c */
	cur += osmo_pbit2ubit(sb_type2, pdu_sync, 60);

	crc = ~crc16_ccitt_bits(sb_type2, 60);
	crc = swap16(crc);
	cur += osmo_pbit2ubit(cur, (uint8_t *) &crc, 16);

	/* Append 4 tail bits: type-2 bits */
	cur += 4;

	/* Run rate 2/3 RCPC code: type-3 bits*/
	{
		struct conv_enc_state *ces = calloc(1, sizeof(*ces));
		conv_enc_init(ces);
		conv_enc_input(ces, sb_type2, 80, sb_master);
		get_punctured_rate(TETRA_RCPC_PUNCT_2_3, sb_master, 120, sb_type3);
		free(ces);
	}

	/* Run (120,11) block interleaving: type-4 bits */
	block_interleave(120, 11, sb_type3, sb_type4);

	memcpy(sb_type5, sb_type4, 120);

	/* Run scrambling (all-zero): type-5 bits */
	tetra_scramb_bits(SCRAMB_INIT, sb_type5, 120);
	//printf("Scrambled synchronization block 1 bits (BSCH): %s\n", osmo_ubit_dump(sb_type5, 120));

	//Sub-block 2
	memset(si_type2, 0, sizeof(si_type2));
	cur = si_type2;

	/* Use pdu_sysinfo from pdus.c */
	cur += osmo_pbit2ubit(si_type2, pdu_sysinfo, 124);
	//memcpy(si_type2,pdu_sysinfo_entropia,124);
	//cur +=124;

	/* Run it through CRC16-CCITT */
	crc = ~crc16_ccitt_bits(si_type2, 124);
	crc = swap16(crc);
	cur += osmo_pbit2ubit(cur, (uint8_t *) &crc, 16);

	/* Append 4 tail bits: type-2 bits */
	cur += 4;

	/* Run rate 2/3 RCPC code: type-3 bits */
	{
		struct conv_enc_state *ces = calloc(1, sizeof(*ces));
		conv_enc_init(ces);
		conv_enc_input(ces, si_type2, 144, si_master);
		get_punctured_rate(TETRA_RCPC_PUNCT_2_3, si_master, 216, si_type3);
		free(ces);
	}

	/* Run (216,101) block interleaving: type-4 bits */
	block_interleave(216, 101, si_type3, si_type4);

	memcpy(si_type5, si_type4, 216);

	/* Run scrambling (all-zero): type-5 bits */
	tetra_scramb_bits(scramb_init, si_type5, 216);
	//printf("Scrambled block 2 bits (BNCH): %s\n", osmo_ubit_dump(si_type5, 216));

	//BROADCAST BITS
	/* Use pdu_acc_ass/pdu_acc_ass_18 from pdus.c */
	uint8_t *bb_type1 = (uint8_t *)(fn < 18 ? pdu_acc_ass : pdu_acc_ass_18); // ACCESS-ASSIGN
	/* Run it through (30,14) RM code: type-2=3=4 bits */
	bb_rm3014 = tetra_rm3014_compute(*(bb_type1) << 8 | *(bb_type1 + 1));
	/* convert to big endian */
	bb_rm3014_be = htonl(bb_rm3014);
	/* shift two bits left as it is only a 30 bit value */
	bb_rm3014_be <<= 2;
	osmo_pbit2ubit(bb_type5, (uint8_t *) &bb_rm3014_be, 30);

	/* Run scrambling (all-zero): type-5 bits */
	tetra_scramb_bits(scramb_init, bb_type5, 30);
	//printf("Scrambled broadcast bits (AACH): %s\n", osmo_ubit_dump(bb_type5, 30));

	/* Finally, hand it into the physical layer */
	build_sync_c_d_burst(buf, sb_type5, bb_type5, si_type5);

	//printf("Synchronization continuous downlink burst (SCDB): %s\n", osmo_ubit_dump(buf, 255*2));
	return BLEN;
}
