
/**
    \file MS_limits.h

    This file lists all the Tunable constants used in
    EtherMind Mesh Stack modules.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LIMITS_
#define _H_MS_LIMITS_


/* ----------------------------------------------------------------------- */
/* =============================  Bearer  ================================ */
/* ----------------------------------------------------------------------- */
#define MS_NUM_NETWORK_INTERFACES                       2
#define MS_NUM_PROVISIONING_INTERFACES                  2


/* ----------------------------------------------------------------------- */
/* =============================  Network  =============================== */
/* ----------------------------------------------------------------------- */
/*
    In a 'flooding' mesh implementation, one of the methods used to restrict
    unlimited flooding, is using message cache.
    This parameter specifies the size of the Network message cache.

    Minimum Value: 2
    Maximum Value: can be anything.
*/
#define MS_NET_CACHE_SIZE                              30       //10->30 by ZQ

/*
    Maximum number of subnets the device can store information about.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_SUBNETS                                 3

/*
    Maximum number of device keys the device can store information about.
    As a configuration client, there should be one additional space to
    contain device key of configuration server.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_DEV_KEYS                                5

/*
    Maximum number of addresses present in each proxy filter list.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_PROXY_FILTER_LIST_SIZE                      5

/*
    The distance between the network sequence numbers, for every persistent
    storage write. If the device is powered cycled, it will resume transmission
    using the sequence number from start of next block.

    Minimum Value: can be anything. A smaller value will reduce the flash lifetime.
    Maximum Value: can be anything.
*/
#define MS_NET_SEQ_NUMBER_BLOCK_SIZE                    512//   2048

/*
    The timeout in millisecond for proxy advertisement with Network ID for
    each subnet

    Minimum Value: can be anything. Larger value will have lesser timeout load.
    Maximum Value: can be anything.
*/
#define PROXY_SUBNET_NETID_ADV_TIMEOUT               (300 | EM_TIMEOUT_MILLISEC)  //100

/*
    The timeout in millisecond for proxy advertisement with Node Identity for
    each subnet

    Minimum Value: can be anything. Larger value will have lesser timeout load.
    Maximum Value: can be anything.
*/
#define PROXY_SUBNET_NODEID_ADV_TIMEOUT              (300 | EM_TIMEOUT_MILLISEC) //100

/*
    The time period for proxy advertisement with Node Identity

    Minimum Value: Default 60 seconds as in specification
    Maximum Value: can be anything.
*/
#define PROXY_NODEID_ADV_TIMEOUT                     (60000 | EM_TIMEOUT_MILLISEC)


/* ----------------------------------------------------------------------- */
/* =============================  Transport  ============================= */
/* ----------------------------------------------------------------------- */
/*
    This parameter specifies the maximum number of Low Power Nodes (LPNs)
    to which friendship can be established as a Friend.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_LPNS                                     1

/*
    Replay Protection cache is required to protect against relay attacks.
    This parameter specifies the size of the Replay Protection cache.

    Minimum Value: 2
    Maximum Value: can be anything.
*/
#define MS_REPLAY_CACHE_SIZE                           30

/*
    Reassembled SAR Rx cache is to avoid handling of segmented frames
    which are already received and acked by the local implementation.
    Saves the unnecessary effort of reassmbly and complex handling by
    the upper layers to handle reception of same payload again from
    the same source device.

    Minimum Value: 2
    Maximum Value: can be anything.
*/
#define MS_REASSEMBLED_CACHE_SIZE                     30

/*
    The number of times to retry the FriendPoll message when
    the FriendUpdate message is not received for the first attempt
    of FriendPoll in the current sequence. When this count expires,
    the friendship will be terminated with the Friend node.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_FRND_POLL_RETRY_COUNT                      10

/*
    Number of Segmentation and Reassembly contexts.
    Used during both reception and transmission and also for associated
    LPNs.

    Minimum Value: 2
    Maximum Value: can be anything.
*/
#define LTRN_SAR_CTX_MAX                              16

/*
    Segment Transmission Timeout.

    Minimum Value: 200 MS
    Maximum Value: can be anything.
*/
#define LTRN_RTX_TIMEOUT                200 /* Millisecond */

/*
    Segment Transmission Count.

    Minimum Value: 2
    Maximum Value: can be anything.
*/
#define LTRN_RTX_COUNT_UNICASS              5
#define LTRN_RTX_COUNT_GROUP                3


/*
    Ack Timeout.
    Minimum Value: 200 MS
    Maximum Value: can be anything.
*/
/* TODO: Use ack timeout configured based on the TTL in the received frame */
#define LTRN_ACK_TIMEOUT             50 /* Millisecond */


/*
    Incomplete Timeout.

    Minimum Value: 10 Seconds (10000 MS)
    Maximum Value: can be anything.
*/
/* TODO: Use ack timeout configured based on the TTL in the received frame */
#define LTRN_INCOMPLETE_TIMEOUT     (20*1000) /* Millisecond */


/*
    Receive window is the time in ms for which the Friend will be transmitting the
    response to any request from the LPN after the Receive delay time of
    getting any request

    Minimum Value: 100
    Maximum Value: As required
*/
#define MS_FRND_RECEIVE_WINDOW                         100

/*
    This parameter defines the maximum number of messages that the friend
    is capabale to queue for a single Low Power Node

    Minimum Value: 2
    Maximum Value: As required
*/
#define MS_FRIEND_MESSAGEQUEUE_SIZE                    8

/*
    This parameter defines the maximum number of subscription addresses
    that the friend is capabale to store for a single Low Power Node

    Minimum Value: 1
    Maximum Value: As required
*/
#define MS_FRIEND_SUBSCRIPTION_LIST_SIZE               8

/*
    This parameter defines the initial timeout in milliseconds
    to be used to track the Friend clear Confirmation after
    sending of a Friend Clear message

    Minimum Value: 1000
    Maximum Value: As required
*/
#define LPN_CLEAR_RETRY_TIMEOUT_INITIAL                1000

/*
    This parameter defines the timeout in milliseconds
    to be used to retry friend request attempts in case an
    offer is not received.

    Minimum Value: 1100
    Maximum Value: As required
*/
#define MS_TRN_FRNDREQ_RETRY_TIMEOUT                   1200

/* ----------------------------------------------------------------------- */
/* =============================  Access ================================= */
/* ----------------------------------------------------------------------- */
/*
    This parameter specifies the maximum number of elements.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_ACCESS_ELEMENT_COUNT                        3

/*
    This parameter specifies the maximum number of models.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_ACCESS_MODEL_COUNT                          20//   5

/*
    Maximum number of Applications (keys) the device can store information about.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_APPS                                    3

/*
    Maximum number of Virtual Addresses the device can store information about.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_VIRTUAL_ADDRS                           1

/*
    Maximum number of Non-Virtual Addresses the device can store information about.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_NON_VIRTUAL_ADDRS                       32

/*
    Maximum number of Addresses the device can store information about.
    This includes both the Virtual and non-virtual addresses.

    Note: This depends on the other configurations.
    Defined here for easy reference. Do not change manually.
*/
#define MS_MAX_ADDRS                                (MS_MAX_VIRTUAL_ADDRS + MS_MAX_NON_VIRTUAL_ADDRS)

#if (MS_MAX_ADDRS  != (MS_MAX_VIRTUAL_ADDRS + MS_MAX_NON_VIRTUAL_ADDRS))
    #error "MS_MAX_ADDRS  != (MS_MAX_VIRTUAL_ADDRS + MS_MAX_NON_VIRTUAL_ADDRS)"
#endif /* (MS_MAX_ADDRS  != (MS_MAX_VIRTUAL_ADDRS + MS_MAX_NON_VIRTUAL_ADDRS)) */

/*
    Maximum number of Transition Timers.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_NUM_TRANSITION_TIMERS                    5

/*
    Maximum number of Periodic Step Timers.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_MAX_NUM_PERIODIC_STEP_TIMERS                 5


/* ----------------------------------------------------------------------- */
/* ==========================  Health Model ============================== */
/* ----------------------------------------------------------------------- */
/*
    This parameter specifies the maximum number of Health Servers
    to be supported.

    Minimum Value: 1
    Maximum Value: can be anything.
*/
#define MS_HEALTH_SERVER_MAX                           2


#define MS_DEFAULT_COMPANY_ID                        0x0059	

/*
    This parameter specifies a 16-bit vendor-assigned product identifier.
*/
#define MS_DEFAULT_PID                                 0x000f

/*
    This parameter specifies a 16-bit vendor-assigned product version identifier.
*/
#define MS_DEFAULT_VID                                 0x0003

#endif /* _H_MS_LIMITS_ */

