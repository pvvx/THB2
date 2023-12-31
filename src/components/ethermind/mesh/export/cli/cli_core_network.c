
/**
    \file cli_core_network.c

    This File contains the Network function for the CLI application,
    to exercise various functionalities of the Network layer.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

#ifdef CLI_NETWORK
/* ------------------------------- Global Variables */
/* Level - Core - Network */
DECL_CONST CLI_COMMAND cli_core_network_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Transmit a Network Packet */
    { "send", "Transmit a Network Packet", cli_core_network_send_packet },

    { "set_hdr", "Set Network Packet Header", cli_core_network_set_pkt_hdr},

    /* Transmit a secure network beacon */
    /* Transmit a secure network beacon with IV update */
    /* Trasmit secure network beacon with key refresh update */
    /* Use this option when peer device initiated key refresh procedure */
    /* Use this option when local device needs to initiate Key refresh procedure */
    /* Transmit a secure network beacon with both IV update and Key Refresh */
    { "snb", "Transmit a Secure Network Beacon", cli_core_network_snb },

    /* get the Current IV Index */
    {"get_ivindex", "Get Current IV Index", cli_core_network_get_ivindex},

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};

/* Global Network Packet Header */
MS_NET_HEADER    g_hdr;
UCHAR            g_hdr_flag = MS_FALSE;

/* ------------------------------- Functions */
/* Network */
API_RESULT cli_core_network(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Core Network\n");
    cli_cmd_stack_push
    (
        (CLI_COMMAND*)cli_core_network_cmd_list,
        sizeof(cli_core_network_cmd_list) / sizeof(CLI_COMMAND)
    );
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* CLI Command Handlers. Level - Core - Network */
/* Network transmit packet */
API_RESULT cli_core_network_send_packet(UINT32 argc, UCHAR* argv[])
{
    UINT16           dst_addr;
    UCHAR            mode;
    MS_BUFFER        buffer;
    API_RESULT       retval;
    UINT16           len;
    UCHAR*             trans_pdu;
    /* Un-Segmented Data of 5 Bytes */
    DECL_CONST UCHAR unseg_trans_pdu[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    /* Segmented Data of 15 Bytes */
    DECL_CONST UCHAR seg_trans_pdu[]   = { 0x01, 0x02, 0x03, 0x04, 0x05,
                                           0x06, 0x07, 0x08, 0x09, 0x0A,
                                           0x0B, 0x0C, 0x0D, 0x0E, 0x0F
                                         };

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: send <destination addr> <Flag[0: Unsegmented, 1: Segmented]>\n");
        return API_FAILURE;
    }

    dst_addr = (UINT16)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    mode     = (UCHAR) CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);

    if (MS_FALSE == g_hdr_flag)
    {
        /* Initialize */
        EM_mem_set(&g_hdr, 0x0, sizeof(g_hdr));
        MS_access_cm_get_primary_unicast_address(&g_hdr.saddr);
        g_hdr.daddr = dst_addr;
        g_hdr.ttl   = 0x01;
        g_hdr.ctl   = 0x00;
    }

    /* Update Sequence Number irrespective of g_hdr_flag */
    MS_net_alloc_seq_num(&g_hdr.seq_num);

    /* Set PDU */
    if (0 == mode)
    {
        trans_pdu = (UCHAR*)unseg_trans_pdu;
        len       = sizeof(unseg_trans_pdu);
    }
    else
    {
        trans_pdu = (UCHAR*)seg_trans_pdu;
        len       = sizeof(seg_trans_pdu);
    }

    buffer.payload = trans_pdu;
    buffer.length  = len;
    retval = MS_net_send_pdu(&g_hdr, 0x0000, &buffer,MS_FALSE);
    return retval;
}

API_RESULT cli_core_network_set_pkt_hdr(UINT32 argc, UCHAR* argv[])
{
    if (4 != argc)
    {
        CONSOLE_OUT("Usage: set_hdr <TTL> <CTL> <SRC Addr> <DST Addr>\n");
        return API_FAILURE;
    }

    /* Initialize */
    EM_mem_set(&g_hdr, 0x0, sizeof(g_hdr));
    /* Fetch the Sequence Number */
    MS_net_alloc_seq_num(&g_hdr.seq_num);
    /* Populate the Network Packet Header Flags */
    g_hdr.ttl   = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    g_hdr.ctl   = (UCHAR)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);
    g_hdr.saddr = (MS_NET_ADDR) CLI_strtoi(argv[2], (UINT16)CLI_strlen(argv[2]), 16);
    g_hdr.daddr = (MS_NET_ADDR) CLI_strtoi(argv[3], (UINT16)CLI_strlen(argv[3]), 16);
    /* Set the global Header Flag Set as TRUE */
    g_hdr_flag  = MS_TRUE;
    return API_SUCCESS;
}

API_RESULT cli_core_network_get_ivindex(UINT32 argc, UCHAR* argv[])
{
    UINT32 ivindex;
    UINT8 ivstate;
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    /* Get the current IV Index */
    retval = MS_access_cm_get_iv_index(&ivindex, &ivstate);
    CONSOLE_OUT("Current IV Index: 0x%08X\n", ivindex);
    CONSOLE_OUT("Current IV Update state: 0x%02X\n", ivstate);
    return retval;
}

/* Network transmit secure network beacon */
/**
    This CLI command has following sets of parameters:
    Option 1: snb <subnet handle>
    Option 2: snb <subnet handle> <krstate>
    Option 3: snb <subnet handle> <ivstate> <ivindex>
    Also, there could be a scenario as stated below which is
    currently not catered to.
    Option 4: snb <subnet handle> <krstate> <ivstate> <ivindex>

    Depending on the number of parameters, we can choose the mode of SNB
    to be sent appropriately.
*/
API_RESULT cli_core_network_snb(UINT32 argc, UCHAR* argv[])
{
    MS_SUBNET_HANDLE handle;
    UINT32           ivindex;
    UINT8            ivstate;
    UINT8            krstate;
    CONSOLE_OUT("In Core Network SNB\n");

    if ((1 > argc) || (4 < argc))
    {
        CONSOLE_OUT("Mandatory Usage: snb <subnet handle>\n");
        CONSOLE_OUT("Usage: snb <subnet handle> {[Optional]<Key Refresh state 2- Phase2, 3- Phase3>}\n");
        CONSOLE_OUT("Usage: snb <subnet handle> {[Optional]<IV update state 0- Normal 1- IV Update in Progress> <IV index>\n");
        CONSOLE_OUT("Usage: snb <subnet handle> <Key Refresh state> <IV update state> <IV index> ");
        return API_FAILURE;
    }

    /* Extract the Subnet Handle from Param 1 */
    handle = (UINT16) CLI_strtoi(argv[0], (UINT16) CLI_strlen(argv[0]), 16);

    if (2 == argc)
    {
        /* Extract Key Refresh State from Param 2 */
        krstate = ((UINT8)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16) & 0x01)
                  ? MS_ACCESS_KEY_REFRESH_PHASE_3 : MS_ACCESS_KEY_REFRESH_PHASE_2;
        MS_access_cm_set_key_refresh_phase(handle, &krstate);
    }
    else if (3 == argc)
    {
        /* Extract IV state from Param 2 */
        ivstate = (UINT8)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);
        ivstate |= 0x80;
        /* Extract IV Index from Param 3 */
        ivindex = (UINT32)CLI_strtoi(argv[2], (UINT16)CLI_strlen(argv[2]), 16);
        MS_access_cm_set_iv_index(ivindex, ivstate);
    }
    else if (4 == argc)
    {
        /* Extract Key Refresh State from Param 2 */
        krstate = ((UINT8)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16) & 0x01)
                  ? MS_ACCESS_KEY_REFRESH_PHASE_3 : MS_ACCESS_KEY_REFRESH_PHASE_2;
        /* Extract IV state from Param 2 */
        ivstate = (UINT8)CLI_strtoi(argv[2], (UINT8)CLI_strlen(argv[2]), 16);
        ivstate |= 0x80;
        /* Extract IV Index from Param 3 */
        ivindex = (UINT32)CLI_strtoi(argv[3], (UINT16)CLI_strlen(argv[3]), 16);
        MS_access_cm_set_key_refresh_phase(handle, &krstate);
        MS_access_cm_set_iv_index(ivindex, ivstate);
    }

    /* Send the Secure Network Beacon */
    MS_net_broadcast_secure_beacon((MS_SUBNET_HANDLE)handle);

    if ((3 == argc) || (4 == argc))
    {
        /**
            To get the iv state to normal state from In Progess state
            and send secure network beacons with new IVIndex value.
        */
        MS_access_cm_set_iv_index(ivindex, 0);
    }

    return API_SUCCESS;
}

#endif /* CLI_NETWORK */
