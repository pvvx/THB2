
/**
    \file cli_core_transport.c

    This File contains the transport function for the CLI application,
    to exercise various functionalities of the transport layer.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"
#include "MS_trn_api.h"

#ifdef CLI_TRANSPORT

/* From "sec_tbx.h" */
void ms_stbx_va
(
    /* IN */  UCHAR*   l,
    /* IN */  UINT16   llen,
    /* OUT */ UINT16* va
);

/* ------------------------------- Global Variables */
/* Level - Core - Transport */
DECL_CONST CLI_COMMAND cli_core_transport_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Transmit a Transport Packet */
    { "send", "Transmit a Transport Packet", cli_core_transport_send_packet },

    /* Trasmit Friend Request */
    { "frndreq", "Friendship Request broadcast", cli_core_transport_frndreq },

    /* Send Transport Control Message */
    { "ctrlmsg", "Send Transport Control Message", cli_core_transport_ctrlmsg },

    /* Clear Replay Cache */
    { "clrreplaycache", "Clear Replay Cache", cli_core_transport_clrreplaycache },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};

void cli_frnd_cb(MS_SUBNET_HANDLE subnet, UCHAR event_type, UINT16 status);

/* ------------------------------- Functions */
/* Transport */
API_RESULT cli_core_transport(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Core Transport\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_transport_cmd_list, sizeof(cli_core_transport_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* CLI Command Handlers. Level - Core - Transport */
/* Transport Send Packet */
API_RESULT cli_core_transport_send_packet(UINT32 argc, UCHAR* argv[])
{
    MS_NET_ADDR       saddr;
    MS_NET_ADDR       daddr;
    MS_BUFFER         buffer;
    API_RESULT        retval;
    UINT8             ttl;
    MS_SUBNET_HANDLE  subnet_handle;
    MS_APPKEY_HANDLE  appkey_handle;
    UCHAR             pdu[] = { 0x01, 0x02, 0x03, 0x04,
                                0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0A, 0x0B, 0x0C,
                                0x0D, 0x0E, 0x0F, 0x10
                              };
    UINT8            label[16];
    UINT8*             a;
    UCHAR            is_segmented;
    CONSOLE_OUT("In Core Transport Send Packet\n");

    if (6 != argc)
    {
        CONSOLE_OUT("Usage: send <Flag [0 - Unsegmented 1- segmented]><SUBNET HANDLE> <APPKEY HANDLE> <TTL> <SRC ADDR> <DST ADDR: Non virtual - 2 octects Virtual - 16 octets>\n");
        return API_FAILURE;
    }

    is_segmented  = (UCHAR)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    subnet_handle = (MS_SUBNET_HANDLE)CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
    appkey_handle = (MS_APPKEY_HANDLE)CLI_strtoi(argv[2], (UINT16)CLI_strlen(argv[2]), 16);
    ttl           = (UINT8)CLI_strtoi(argv[3], (UINT16)CLI_strlen(argv[3]), 16);
    saddr         = (MS_NET_ADDR)CLI_strtoi(argv[4], (UINT16)CLI_strlen(argv[4]), 16);

    if (4 >= CLI_strlen(argv[5]))
    {
        daddr = (MS_NET_ADDR)CLI_strtoi(argv[5], (UINT16)CLI_strlen(argv[5]), 16);
        CONSOLE_OUT("Destination Address: non-Virtual Address: 0x%04X\n", daddr);
        a = NULL;
    }
    else
    {
        CONSOLE_OUT("Destination Address is Virtual Address\n");
        CLI_strtoarray
        (
            argv[5],
            (UINT16)CLI_strlen(argv[5]),
            label,
            MS_LABEL_UUID_LENGTH
        );
        /* Calculate corresponding virtual address */
        ms_stbx_va
        (
            label,
            MS_LABEL_UUID_LENGTH,
            &daddr
        );
        a = &label[0];
        CONSOLE_OUT("Calculated Destination Address:0x%04X\n", daddr);
    }

    /* Set PDU */
    buffer.payload = pdu;
    /**
        If the PDU to be transmitted is Un-segmented then currently we are
        sending "5" Bytes of Access message otherwise it is "16" Bytes.
    */
    buffer.length = (0 == is_segmented) ? 5 : sizeof(pdu);
    retval = MS_trn_send_access_pdu
             (
                 saddr,
                 daddr,
                 a,
                 subnet_handle,
                 appkey_handle,
                 ttl,
                 &buffer,
                 MS_FALSE
             );
    return retval;
}

/* Transport FrndReq */
API_RESULT cli_core_transport_frndreq(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_LPN_SUPPORT
    int val;
    API_RESULT retval;
    MS_SUBNET_HANDLE subnet;
    UCHAR criteria, rx_delay;
    UINT32 poll_to, setup_to;
    CONSOLE_OUT("In Core Transport FrndReq\n");

    if (5 != argc)
    {
        CONSOLE_OUT("Usage: frndreq <subnet> <criteria> <rx_delay> <poll_timeout> <setup_timeout>");
        return API_FAILURE;
    }

    val    = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    subnet = (MS_SUBNET_HANDLE)val;
    val = CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
    criteria = (UCHAR)val;
    val = CLI_strtoi(argv[2], (UINT16)CLI_strlen(argv[2]), 10);
    rx_delay = (UCHAR)val;
    val = CLI_strtoi(argv[3], (UINT16)CLI_strlen(argv[3]), 10);
    poll_to = (UINT32)val;
    val = CLI_strtoi(argv[4], (UINT16)CLI_strlen(argv[4]), 10);
    setup_to = (UINT32)val;
    retval = MS_trn_lpn_setup_friendship
             (
                 subnet,
                 criteria,
                 rx_delay,
                 poll_to,
                 setup_to,
                 cli_frnd_cb
             );
    CONSOLE_OUT ("Retval - 0x%04X\n", retval);
    return retval;
    #else /* MS_LPN_SUPPORT */
    CONSOLE_OUT("MS_LPN_SUPPORT not defined.");
    return API_FAILURE;
    #endif /* MS_LPN_SUPPORT */
}

void cli_frnd_cb(MS_SUBNET_HANDLE subnet, UCHAR event_type, UINT16 status)
{
    API_RESULT retval;
    UINT16 num_subaddr;
    UINT16 subaddr[5];
    CONSOLE_OUT("\nFriendship Event 0x%02X on Subnet 0x%04X - 0x%04X\n",
                event_type, subnet, status);

    switch (event_type)
    {
    case MS_TRN_FRIEND_SETUP_CNF:
        CONSOLE_OUT("Recvd MS_TRN_FRIEND_SETUP_CNF - 0x%04X\n", status);

        if (API_SUCCESS == status)
        {
            /* Get the subscription list */
            num_subaddr = sizeof(subaddr) / sizeof(UINT16);
            MS_access_cm_get_all_model_subscription_list(&num_subaddr, subaddr);

            if (0 < num_subaddr)
            {
                CONSOLE_OUT("Initiating FriendSubscriptionListAdd - %d addr\n", num_subaddr);
                retval = MS_trn_lpn_subscrn_list_add(subaddr, num_subaddr);
                CONSOLE_OUT("Retval - 0x%04X\n", retval);
            }
        }

        break;

    case MS_TRN_FRIEND_SUBSCRNLIST_CNF:
        CONSOLE_OUT("Recvd MS_TRN_FRIEND_SUBSCRNLIST_CNF - 0x%04X\n", status);
        break;

    case MS_TRN_FRIEND_CLEAR_CNF:
        CONSOLE_OUT("Recvd MS_TRN_FRIEND_CLEAR_CNF - 0x%04X\n", status);
        break;

    case MS_TRN_FRIEND_TERMINATE_IND:
        CONSOLE_OUT("Recvd MS_TRN_FRIEND_TERMINATE_IND - 0x%04X\n", status);
        break;

    default:
        break;
    }
}

API_RESULT cli_core_transport_ctrlmsg(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_LPN_SUPPORT
    API_RESULT retval;
    MS_NET_ADDR addr[1];
    UCHAR ctrlmsg_type;
    retval = API_FAILURE;

    if (1 > argc)
    {
        CONSOLE_OUT("Usage: ctrlmsg <0 - FriendSubscriptionListAdd\n  1 - FriendSubscriptionListRemove\n  2- FriendClear>\n");
        return retval;
    }

    ctrlmsg_type = (UCHAR)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);

    if (0 == ctrlmsg_type)
    {
        if (2 != argc)
        {
            CONSOLE_OUT("Usage: ctrlmsg 0 <group_addr>\n");
            return retval;
        }

        addr[0] = (UINT16)CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
        CONSOLE_OUT("In Core Transport FriendSubscriptionListAdd\n");
        retval = MS_trn_lpn_subscrn_list_add(addr, (sizeof(addr) / sizeof(MS_NET_ADDR)));
    }
    else if (1 == ctrlmsg_type)
    {
        if (2 != argc)
        {
            CONSOLE_OUT("Usage: ctrlmsg 1 <group_addr>\n");
            return retval;
        }

        addr[0] = (UINT16)CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
        CONSOLE_OUT("In Core Transport FriendSubscriptionListRemove\n");
        retval = MS_trn_lpn_subscrn_list_remove(addr, (sizeof(addr) / sizeof(MS_NET_ADDR)));
    }
    else
    {
        CONSOLE_OUT("In Core Transport FriendClear\n");
        retval = MS_trn_lpn_clear_friendship();
    }

    CONSOLE_OUT("Retval - 0x%04X\n", retval);
    return retval;
    #else /* MS_LPN_SUPPORT */
    CONSOLE_OUT("MS_LPN_SUPPORT not defined.\n");
    return API_FAILURE;
    #endif /* MS_LPN_SUPPORT */
}

API_RESULT cli_core_transport_clrreplaycache(UINT32 argc, UCHAR* argv[])
{
    /* No parameter */
    ltrn_init_replay_cache();
    CONSOLE_OUT("Replay Cache cleared...\n");
    return API_SUCCESS;
}

#endif /* CLI_TRANSPORT */

