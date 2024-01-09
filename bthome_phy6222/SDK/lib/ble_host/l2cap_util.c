/*************************************************************************************************
    Filename:       l2cap_util.c
    Revised:
    Revision:

    Description:    This file contains the L2CAP utility funtions. It includes the
                  L2CAP Encapsulation and Fragmentation/Recombination components.

	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "osal_bufmgr.h"
#include "osal_cbtimer.h"

#include "hci.h"
#include "linkdb.h"

#include "l2cap_internal.h"

#include "ll_def.h"
#include "ll_common.h"
#include "att.h"
#include "global_config.h"

/*********************************************************************
    MACROS
*/

// Return the next available Signaling identifier.
// Note: Signaling identifier of 0 is not valid.
#define NEXT_SIG_ID()                    ( ++l2capId == 0 ? l2capId = 1 : l2capId )

/*********************************************************************
    CONSTANTS
*/
// Length of Command Reject fixed field: reason (2)
#define CMD_REJECT_FIXED_SIZE            2

// Length of Information Request: Info type (2)
#define INFO_REQ_SIZE                    2

// Length of Information Response fixed fields: Info type (2) + Result (2)
#define INFO_RSP_FIXED_SIZE              ( 2 + 2 )

// Length of Connection Parameter Update Request: Int min (2) + Int max (2) + Latency (2) + Timeout (2)
#define PARAM_UPDATE_REQ_SIZE            ( 2 + 2 + 2 + 2 )

// Length of Connection Parameter Update Response: Status (2)
#define PARAM_UPDATE_RSP_SIZE            2

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
l2capReassemblePkt_t l2capReassemblePkt[MAX_NUM_LL_CONN];
l2capSegmentBuff_t   l2capSegmentPkt[MAX_NUM_LL_CONN];
l2capSARDbugCnt_t g_sarDbgCnt;
uint8 l2capExtendFragments = TRUE;
/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// L2CAP Signaling identifier
static uint8 l2capId = 0;

/*********************************************************************
    LOCAL FUNCTIONS
*/
l2capChannel_t* l2capAllocChannel( uint8 taskId );
static void l2capBuildSignalHdr( l2capSignalHdr_t* pHdr, uint8* pData );
static bStatus_t l2capAllocConnChannel( uint16 connHandle, uint8 taskId, l2capChannel_t** p2pChannel );
static void l2capStartTimer( l2capChannel_t* pChannel, uint16 timeout );
static bStatus_t L2CAP_SendDataPkt( uint16 connHandle, uint16 pktLen, uint8* pData );
static void l2capHandleTimerCB( uint8* pData );

/*********************************************************************
    @fn      l2capParseSignalHdr

    @brief   Parse the L2CAP Signaling header.

    @param   pHdr - pointer to signaling header
    @param   pData - pointer to message to be parsed

    @return  none
*/
void l2capParseSignalHdr( l2capSignalHdr_t* pHdr, uint8* pData )
{
    // Parse the signaling header
    pHdr->opcode = pData[0];
    pHdr->id = pData[1];
    pHdr->len = BUILD_UINT16( pData[2], pData[3] );
}

/*********************************************************************
    @fn      l2capBuildSignalHdr

    @brief   Build the L2CAP Signaling header.

    @param   pHdr - pointer to signaling header
    @param   pData - pointer to message to be built

    @return  none
*/
static void l2capBuildSignalHdr( l2capSignalHdr_t* pHdr, uint8* pData )
{
    // Build the signaling header
    pData[0] = pHdr->opcode;
    pData[1] = pHdr->id;
    pData[2] = LO_UINT16( pHdr->len );
    pData[3] = HI_UINT16( pHdr->len );
}

/*********************************************************************
    @fn      l2capSendReq

    @brief   Send an L2CAP Signaling Request.

    @param   connHandle - connection to use
    @param   opcode - type of command
    @param   pReq - request to be sent
    @param   pfnBuildCmd - function to build request
    @param   state - state of channel
    @param   taskId - task to be notified about result

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Data can not fit into one packet.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            FAILURE: Internal error occured.
            bleMemAllocError: Memory allocation error occurred.
            bleNotConnected: Connection is down.
            bleNoResources: No available resource
*/
bStatus_t l2capSendReq( uint16 connHandle, uint8 opcode, uint8* pReq,
                        pfnL2CAPBuildCmd_t pfnBuildCmd, uint8 state, uint8 taskId )
{
    l2capChannel_t* pChannel;
    uint8 status;
    status = l2capAllocConnChannel( connHandle, taskId, &pChannel );

    if ( status == SUCCESS )
    {
        // Set up the channel info
        pChannel->state = state;
        pChannel->id = NEXT_SIG_ID();
        status = l2capSendCmd( connHandle, opcode, pChannel->id, pReq, pfnBuildCmd );

        if ( status == SUCCESS )
        {
            l2capStartTimer( pChannel, L2CAP_RTX_TIMEOUT );
        }
        else
        {
            l2capFreeChannel( pChannel );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      l2capSendCmd

    @brief   Send an L2CAP Signaling command.

    @param   connHandle - connection to use
    @param   opcode - type of command
    @param   id - identifier
    @param   pCmd - command data
    @param   pfnBuildCmd - function to build command

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Data can not fit into one packet.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t l2capSendCmd( uint16 connHandle, uint8 opcode, uint8 id,
                        uint8* pCmd, pfnL2CAPBuildCmd_t pfnBuildCmd )
{
    uint8* pBuf;
    bStatus_t status;
    // Allocate space for the message
    pBuf = (uint8*)L2CAP_bm_alloc( L2CAP_SIG_MTU_SIZE );

    if ( pBuf != NULL )
    {
        l2capPacket_t pkt;
        l2capSignalHdr_t hdr;
        // Set up the signaling header
        hdr.opcode = opcode;
        hdr.id = id;

        // First build the data field of the command
        if ( pfnBuildCmd != NULL )
        {
            // length of data field
            hdr.len = (*pfnBuildCmd)( &(pBuf[SIGNAL_HDR_SIZE]), pCmd );
        }
        else
        {
            // length of data field
            hdr.len = 0;
        }

        // Build the signaling header
        l2capBuildSignalHdr( &hdr, pBuf );
        // Set up the L2CAP packet
        pkt.CID = L2CAP_CID_SIG; // destination CID
        pkt.pPayload = pBuf;
        pkt.len = hdr.len + SIGNAL_HDR_SIZE; // length of pPayload
        // Encapsulate and send the command
        status = l2capEncapSendData( connHandle, &pkt );

        if ( status != SUCCESS )
        {
            // Free the buffer
            osal_bm_free( pBuf );
        }
    }
    else
    {
        status = bleMemAllocError;
    }

    return ( status );
}

/*********************************************************************
    @fn      l2capEncapSendData

    @brief   Encapsulate and send an L2CAP data packet over a physical connection.

            Note: Packet 'pPayload' must be allocated using L2CAP_bm_alloc().

    @param   connHandle - connection handle to use
    @param   pPkt - pointer to packet to be sent (must contain destination CID)

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Data can not fit into one packet.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t l2capEncapSendData( uint16 connHandle, l2capPacket_t* pPkt )
{
    uint8* pBuf;
    uint8* pHdr;
    uint8 totalLen;
    // Get a pointer to basic L2CAP header
    pBuf = osal_bm_adjust_header( pPkt->pPayload, L2CAP_HDR_SIZE );
    // Fill in L2CAP header fields
    pHdr = pBuf;
    *pHdr++ = LO_UINT16( pPkt->len ); // Payload length
    *pHdr++ = HI_UINT16( pPkt->len );
    // Destination Channel ID
    *pHdr++ = LO_UINT16( pPkt->CID );
    *pHdr++ = HI_UINT16( pPkt->CID );
    // Add PDU header length to total packet length
    totalLen = pPkt->len + L2CAP_HDR_SIZE;

    // See if data can fit into one packet
    if ( totalLen <= L2CAP_PDU_SIZE )
    {
        uint8 status;

        // This is the first packet
        if ( connHandle == LOOPBACK_CONNHANDLE )
        {
            // Loop back the data packet
            status = L2CAP_SendDataPkt( connHandle, totalLen, pBuf );
        }
        else
        {
            #if 0
            // Send the data packet to the Controller
            // Note: Host to Controller Data Flow Control is not supported yet!
            //       Host should use the LE Read Buffer Size command to determine
            //       the maximum size and the total number of HCI Data Packets.
            status = HCI_SendDataPkt( connHandle, FIRST_PKT_HOST_TO_CTRL, totalLen, pBuf );

            // Map HCI status codes to Host status codes
            switch ( status )
            {
            case HCI_ERROR_CODE_UNKNOWN_CONN_ID:
                status = bleNotConnected;
                break;

            case HCI_ERROR_CODE_INVALID_HCI_CMD_PARAMS:
                status = INVALIDPARAMETER;
                break;

            case HCI_ERROR_CODE_MEM_CAP_EXCEEDED:
                status = MSG_BUFFER_NOT_AVAIL;
                break;

            default:
                // Should never get here!
                break;
            }

            #else
            #if 0
            //==========================================================
            // add by ZQ for l2capSAR 20181026
            //==========================================================
            uint8 llBufLimit = g_llPduLen.local.MaxTxOctets;//LL_MAX_LINK_DATA_LEN;

            //See if need to do fragment
            if(totalLen<=llBufLimit)
            {
                status=L2CAP_Fragment_SendDataPkt(  connHandle,  FIRST_PKT_HOST_TO_CTRL, totalLen, pBuf );
            }
            else
            {
                uint8* fBuf;
                uint8* pBufTmp = pBuf;
                uint8 fpLen = totalLen / llBufLimit;
                uint8 resLen = totalLen-fpLen*llBufLimit;

                for(uint8 i=0; i<fpLen; i++)
                {
                    fBuf=(uint8*) L2CAP_Fragment_bm_alloc(llBufLimit);

                    if(fBuf!=NULL)
                    {
                        osal_memcpy(fBuf, pBufTmp, llBufLimit);
                        status=L2CAP_Fragment_SendDataPkt(  connHandle,
                                                            (i==0)?FIRST_PKT_HOST_TO_CTRL:CONTINUING_PKT,
                                                            llBufLimit,
                                                            fBuf);
                        pBufTmp+=llBufLimit;
                        osal_bm_free(fBuf);
                        LOG("[SAR_TX] %08x %d \n",fBuf,status);
                    }
                    else
                    {
                        status = bleMemAllocError;
                        LOG("[SAR_TX Err] %08x \n",fBuf);
                    }

                    if(status!=SUCCESS)
                        break;
                }

                //do fragment for the residual pkt
                if(status==SUCCESS)
                {
                    if(resLen>0)
                    {
                        fBuf=(uint8*) L2CAP_Fragment_bm_alloc(resLen);
                        osal_memcpy(fBuf, pBufTmp, resLen);
                        status=L2CAP_Fragment_SendDataPkt(  connHandle,  CONTINUING_PKT, resLen, fBuf );
                        osal_bm_free(fBuf);
                        LOG("[SAR_TX] %08x %d \n",fBuf,status);
                    }

                    //free the l2cap buf
                    if(status==SUCCESS)
                    {
                        osal_bm_free(pBuf);
                    }
                }
            }

            #else

//        LOG("%08x [EBUF_IN ] (D%d I%d F%d)\n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                 l2capSegmentPkt.depth,
//                                                 l2capSegmentPkt.idx,
//                                                 l2capSegmentPkt.fragment);
            //==========================================================
            // add by ZQ for l2capSAR pingpang buffer 20181104
            //==========================================================
            //--------------------------------------------------------------------
            //receiver l2cap to buffer
            //--------------------------------------------------------------------
            //status is determinded by the reciver buff
            //if the pkt is buffered, the status should be success
            //otherwise, it should be return as HCI_ERROR_CODE_MEM_CAP_EXCEEDED
            //when fragment is existed, new l2cPkt will not be buffed

            if(l2capSegmentPkt[connHandle].fragment==FALSE)
            {
                status = l2capPktToSegmentBuff(connHandle,&l2capSegmentPkt[connHandle],totalLen, pBuf );

                if(status!=SUCCESS)
                {
                    LOG("[SBUF FULL] ERR %d 0x%02x\n",g_sarDbgCnt.segmentMemAlocErr,status);
                }
            }
            else
            {
                //fragment pkt being retx in next connIntv
                status = HCI_ERROR_CODE_MEM_CAP_EXCEEDED;
                g_sarDbgCnt.segmentErrCnt++;
                LOG("[SBUF FRAG FULL]\n");
            }

//        LOG("%08x [SBUF_IN ] (D%d I%d F%d)  0x%02x\n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                 l2capSegmentPkt.depth,
//                                                 l2capSegmentPkt.idx,
//                                                 l2capSegmentPkt.fragment,
//                                                 status);

            //--------------------------------------------------------------------
            //process buff
            //--------------------------------------------------------------------
            //uint8 sendSatus=MSG_BUFFER_NOT_AVAIL;

            //only send the l2cap pkt form header here
            //the fragment pkt will be processed in ll_salveEndCase...
            if(l2capSegmentPkt[connHandle].fragment==FALSE)
            {
                l2capSegmentBuffToLinkLayer(connHandle,&l2capSegmentPkt[connHandle]);
            }

//        LOG("%08x [SBUF_OUT] (D%d I%d F%d)  0x%02x\n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                 l2capSegmentPkt.depth,
//                                                 l2capSegmentPkt.idx,
//                                                 l2capSegmentPkt.fragment,
//                                                 sendSatus);
            #endif
            #endif
        }

        return ( status );
    }

    // Fragment data and send each fragment separately -- not supported yet!
    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      l2capParsePacket

    @brief   Parse an incoming L2CAP packet received over a physical connection.

    @param   pPkt - pointer to L2CAP packet to be built.
    @param   pHciMsg - pointer to received HCI message.

    @return  TRUE: Data buffer must be freed.
            FALSE: Data buffer must not be freed.
*/

uint16 currSeqNum;
uint8 l2capParsePacket( l2capPacket_t* pPkt, hciDataEvent_t* pHciMsg )
{
    uint16 connHandle = pHciMsg->connHandle;
    // Initialize the payload
    pPkt->pPayload = NULL;

    // See if this is the first segment
    if ( pHciMsg->pbFlag == FIRST_PKT_CTRL_TO_HOST )
    {
        // We need the first 4 bytes of the packet to proceed
        if ( pHciMsg->len >= L2CAP_HDR_SIZE )
        {
            // Get payload length and channel id
            uint16 len = BUILD_UINT16( pHciMsg->pData[0], pHciMsg->pData[1] );
            uint16 CID = BUILD_UINT16( pHciMsg->pData[2], pHciMsg->pData[3] );
//            LOG("%08x [L2C_RX]L%04x C%04x ATT%02x %02x %02x %02x %02x\n",getMcuPrecisionCount(),len,CID,
//                                                    pHciMsg->pData[4],
//                                                    pHciMsg->pData[5],
//                                                    pHciMsg->pData[6],
//                                                    pHciMsg->pData[7],
//                                                    pHciMsg->pData[8]);
            currSeqNum=BUILD_UINT16( pHciMsg->pData[8], pHciMsg->pData[7] );

            // See if the packet was unsegmented
            if ( ( len + L2CAP_HDR_SIZE ) == pHciMsg->len )
            {
                // Make sure this is a valid channel; we only support fixed channels for now
                if ( !FIX_CHANNEL( CID ) )
                {
                    // Indicate to the caller to free the data buffer
                    LOG("[SAR_RX ERR] NoSar c %x\n",CID);
                    return ( TRUE );
                }

                if(CID==L2CAP_CID_ATT && l2capReassemblePkt[connHandle].cIdx>0)
                {
                    LOG("[SAR_RX ERR %d] %d %d\n",__LINE__,currSeqNum,l2capReassemblePkt[connHandle].cIdx);
                    L2CAP_ReassemblePkt_Reset(connHandle);
                    g_sarDbgCnt.reassembleErrInComp++;
                }

                // We received the entire packet (unfragmented)
                pPkt->len = len;
                pPkt->CID = CID;
                // Get a pointer to payload
                pPkt->pPayload = osal_bm_adjust_header( pHciMsg->pData, -L2CAP_HDR_SIZE );
//                LOG("%08x [ABUF_IN] NoSar ( %08x L %d C %d) \n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                         pPkt->pPayload ,len,CID);
                // Indicate to the caller not to free the data buffer yet
                return ( FALSE );
            }
            else
            {
                // This is the start of a new fragmented packet -- fragmentation is NOW supported !
                //==========================================================
                // add by ZQ for l2capSAR 20181026
                //==========================================================
                // Make sure this is a valid channel; we only support fixed channels for now
                if ( !FIX_CHANNEL( CID ) )
                {
                    // Indicate to the caller to free the data buffer
                    LOG("[SAR_RX ERR %d] %d %d\n",__LINE__,currSeqNum,CID);
                    g_sarDbgCnt.reassembleErrCID++;
                    return ( TRUE );
                }

                if(CID==L2CAP_CID_ATT && l2capReassemblePkt[connHandle].cIdx>0)
                {
                    LOG("[SAR_RX ERR %d] %d %d\n",__LINE__,currSeqNum,l2capReassemblePkt[connHandle].cIdx);
                    L2CAP_ReassemblePkt_Reset(connHandle);
                    g_sarDbgCnt.reassembleErrInComp++;
                }

                l2capReassemblePkt[connHandle].cIdx         = 0;
                l2capReassemblePkt[connHandle].pkt.len      = len;
                l2capReassemblePkt[connHandle].pkt.CID      = CID;
                l2capReassemblePkt[connHandle].pkt.pPayload = L2CAP_bm_alloc(len);

                if(l2capReassemblePkt[connHandle].pkt.pPayload!=NULL)
                {
                    //for the first segment pkt, need copy from the l2cap header
                    osal_memcpy(l2capReassemblePkt[connHandle].pkt.pPayload,
                                osal_bm_adjust_header( pHciMsg->pData, -L2CAP_HDR_SIZE ),
                                pHciMsg->len-L2CAP_HDR_SIZE);
                    l2capReassemblePkt[connHandle].cIdx+=(pHciMsg->len-L2CAP_HDR_SIZE);
                    g_sarDbgCnt.reassembleInCnt++;
//                    LOG("%08x [ABUF_IN] ST ( %08x L %d C %d) \n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                         l2capReassemblePkt[connHandle].pkt.pPayload,len,CID);
                }
                else
                {
                    g_sarDbgCnt.resssambleMemAlocErr++;
//                    LOG("%08x [ABUF_IN ] FULL %d \n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,g_sarDbgCnt.resssambleMemAlocErr);
                }

                // Indicate to the caller to free the data buffer
                return ( TRUE );
            }
        }
    }
    else
    {
        //==========================================================
        // add by ZQ for l2capSAR 20181026
        //==========================================================
        if(     l2capReassemblePkt[connHandle].cIdx>0
                &&  l2capReassemblePkt[connHandle].pkt.pPayload!=NULL
                &&  pHciMsg->pbFlag==CONTINUING_PKT )
        {
            l2capReassemblePkt[connHandle].cIdx+=pHciMsg->len;

            if(l2capReassemblePkt[connHandle].cIdx<=l2capReassemblePkt[connHandle].pkt.len)
            {
                //for the residual segment pkt, need copy all
                osal_memcpy(l2capReassemblePkt[connHandle].pkt.pPayload+l2capReassemblePkt[connHandle].cIdx-pHciMsg->len,
                            pHciMsg->pData,pHciMsg->len);

//                LOG("%08x [ABUF_IN] Idx++ %d %02x\n",g_osal_mem_allo_cnt-g_osal_mem_free_cnt,
//                                                    l2capReassemblePkt[connHandle].cIdx,
//                                                    pHciMsg->pData[0]);

                if(l2capReassemblePkt[connHandle].cIdx==l2capReassemblePkt[connHandle].pkt.len)
                {
                    // We received the entire packet (fragmented)
                    pPkt->len = l2capReassemblePkt[connHandle].pkt.len;
                    pPkt->CID = l2capReassemblePkt[connHandle].pkt.CID;
                    // Get a pointer to payload
                    pPkt->pPayload = l2capReassemblePkt[connHandle].pkt.pPayload;
                    l2capReassemblePkt[connHandle].cIdx = 0;
                    g_sarDbgCnt.reassembleOutCnt++;
                    // Indicate to the caller not to free the data buffer yet
                    return ( TRUE );
                }
            }
            else
            {
                //error case
//                 LOG("[SAR_RX ERR %d %d] %d p%08x L %d C %d\n",__LINE__,currSeqNum,
//                                                 g_sarDbgCnt.reassembleErrIdx,
//                                                 l2capReassemblePkt.pkt.pPayload,
//                                                 l2capReassemblePkt.pkt.len,
//                                                 l2capReassemblePkt.cIdx);
                g_sarDbgCnt.reassembleErrIdx++;
                L2CAP_ReassemblePkt_Reset(connHandle);
            }
        }
        else
        {
            //error case
            LOG("[SAR_RX ERR %d %d] %d p%08x L %d C %d\n",__LINE__,currSeqNum,
                g_sarDbgCnt.reassembleErrMiss,
                l2capReassemblePkt[connHandle].pkt.pPayload,
                l2capReassemblePkt[connHandle].pkt.len,
                l2capReassemblePkt[connHandle].cIdx);
            g_sarDbgCnt.reassembleErrMiss++;
            L2CAP_ReassemblePkt_Reset(connHandle);
        }
    }

    // Indicate to the caller to free the data buffer
    return ( TRUE );
}

/*********************************************************************
    @fn      L2CAP_BuildCmdReject

    @brief   Build Command Reject.

    @param   pBuf - pointer to buffer to hold command data
    @param   pCmd - pointer to command data

    @return  length of the command data
*/
uint16 L2CAP_BuildCmdReject( uint8* pBuf, uint8* pCmd )
{
    uint16 len = CMD_REJECT_FIXED_SIZE;
    l2capCmdReject_t* pReject = (l2capCmdReject_t*)pCmd;
    // Reason
    *pBuf++ = LO_UINT16( pReject->reason );
    *pBuf++ = HI_UINT16( pReject->reason );

    // Reason data
    if ( pReject->reason == L2CAP_REJECT_SIGNAL_MTU_EXCEED )
    {
        // Signaling MTU
        *pBuf++ = LO_UINT16( pReject->maxSignalMTU );
        *pBuf   = HI_UINT16( pReject->maxSignalMTU );
        len += 2;
    }
    else if ( pReject->reason == L2CAP_REJECT_INVALID_CID )
    {
        // Local CID
        *pBuf++ = LO_UINT16( pReject->invalidLocalCID );
        *pBuf++ = HI_UINT16( pReject->invalidLocalCID );
        // Remote CID
        *pBuf++ = LO_UINT16( pReject->invalidRemoteCID );
        *pBuf   = HI_UINT16( pReject->invalidRemoteCID );
        len += 4;
    }

    // else no Reason Data for L2CAP_REJECT_CMD_NOT_UNDERSTOOD
    return ( len );
}

/*********************************************************************
    @fn      l2capParseCmdReject

    @brief   Parse Command Reject message.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
            FAILURE: Command length is invalid.
*/
bStatus_t l2capParseCmdReject( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    if ( len >= CMD_REJECT_FIXED_SIZE )
    {
        l2capCmdReject_t* pReject = &pCmd->cmdReject;
        // Reason
        pReject->reason = BUILD_UINT16( pData[0], pData[1] );

        // Reason data
        if ( pReject->reason == L2CAP_REJECT_SIGNAL_MTU_EXCEED )
        {
            // Signaling MTU
            pReject->maxSignalMTU = BUILD_UINT16( pData[2], pData[3] );
        }
        else if ( pReject->reason == L2CAP_REJECT_INVALID_CID )
        {
            // Local CID
            pReject->invalidLocalCID = BUILD_UINT16( pData[2], pData[3] );
            // Remote CID
            pReject->invalidRemoteCID = BUILD_UINT16( pData[4], pData[5] );
        }

        // else no Reason Data for L2CAP_REJECT_CMD_NOT_UNDERSTOOD
        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      l2capBuildEchoReq

    @brief   Build Echo Request.

    @param   pBuf - pointer to buffer to hold command data
    @param   pCmd - pointer to command data

    @return  length of the command data
*/
uint16 l2capBuildEchoReq( uint8* pBuf, uint8* pCmd )
{
    uint16 len;
    l2capEchoReq_t* pReq = (l2capEchoReq_t*)pCmd;

    if ( pReq->len > SIGNAL_DATA_SIZE )
    {
        len = SIGNAL_DATA_SIZE;
    }
    else
    {
        len = pReq->len;
    }

    // Copy data field over
    if ( len > 0 )
    {
        VOID osal_memcpy( pBuf, pReq->pData, len );
    }

    return ( len );
}

/*********************************************************************
    @fn      l2capBuildEchoRsp

    @brief   Build Echo Response message.

    @param   pBuf - pointer to buffer to hold command data
    @param   pCmd - pointer to command data

    @return  length of the command data
*/
uint16 l2capBuildEchoRsp( uint8* pBuf, uint8* pCmd )
{
    l2capEchoRsp_t* pRsp = (l2capEchoRsp_t*)pCmd;
    uint16 len = pRsp->len;

    if ( len > 0 )
    {
        if ( len > SIGNAL_DATA_SIZE )
        {
            // Reset it to the max
            len = SIGNAL_DATA_SIZE;
        }

        // Copy data field over
        VOID osal_memcpy( pBuf, pRsp->pData, len );
    }

    return ( len );
}

/*********************************************************************
    @fn      l2capParseEchoRsp

    @brief   Parse Echo Response message.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
*/
bStatus_t l2capParseEchoRsp( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    l2capEchoRsp_t* pRsp = &pCmd->echoRsp;
    pRsp->len = len;

    // Allocate buffer and copy data over
    if ( len > 0 )
    {
        pRsp->pData = osal_mem_alloc( len );

        if ( pRsp->pData != NULL )
        {
            VOID osal_memcpy( pRsp->pData, pData, len );
        }
        else
        {
            pRsp->len = 0;
        }
    }
    else
    {
        pRsp->pData = NULL;
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn      l2capBuildInfoReq

    @brief   Build Information Request.

    @param   pBuf - pointer to buffer to hold command data
    @param   pData - pointer to command data

    @return  length of the command data
*/
uint16 l2capBuildInfoReq( uint8* pBuf, uint8* pData )
{
    l2capInfoReq_t* pReq = (l2capInfoReq_t*)pData;
    // Info type
    *pBuf++ = LO_UINT16( pReq->infoType );
    *pBuf   = HI_UINT16( pReq->infoType );
    return ( INFO_REQ_SIZE );
}

/*********************************************************************
    @fn      L2CAP_ParseInfoReq

    @brief   Parse Information Request message.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
            FAILURE: Command length is invalid.
*/
bStatus_t L2CAP_ParseInfoReq( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    // Info type
    if ( len == INFO_REQ_SIZE )
    {
        l2capInfoReq_t* pReq = &pCmd->infoReq;
        pReq->infoType = BUILD_UINT16( pData[0], pData[1] );
        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      L2CAP_BuildInfoRsp

    @brief   Build Information Response.

    @param   pBuf - pointer to buffer to hold command data
    @param   pCmd - pointer to command data

    @return  length of the command data
*/
uint16 L2CAP_BuildInfoRsp( uint8* pBuf, uint8* pCmd )
{
    uint16 len = INFO_RSP_FIXED_SIZE;
    l2capInfoRsp_t* pRsp = (l2capInfoRsp_t*)pCmd;
    // Info type
    *pBuf++ = LO_UINT16( pRsp->infoType );
    *pBuf++ = HI_UINT16( pRsp->infoType );
    // Result
    *pBuf++ = LO_UINT16( pRsp->result );
    *pBuf++ = HI_UINT16( pRsp->result );

    if ( pRsp->result == L2CAP_INFO_SUCCESS )
    {
        // Build the contents of Data field
        if ( pRsp->infoType == L2CAP_INFO_EXTENDED_FEATURES )
        {
            // Extended Features mask
            VOID osal_buffer_uint32( pBuf, pRsp->info.extendedFeatures );
            len += L2CAP_EXTENDED_FEATURES_SIZE;
        }
        else if ( pRsp->infoType == L2CAP_INFO_FIXED_CHANNELS )
        {
            // Fixed Channels mask
            VOID osal_memcpy( pBuf, pRsp->info.fixedChannels, L2CAP_FIXED_CHANNELS_SIZE );
            len += L2CAP_FIXED_CHANNELS_SIZE;
        }

        // else no Data for L2CAP_INFO_CONNLESS_MTU
    }

    return ( len );
}

/*********************************************************************
    @fn      l2capParseInfoRsp

    @brief   Parse Information Response message.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
            FAILURE: Command length is invalid.
*/
bStatus_t l2capParseInfoRsp( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    // First check for fixed fields: Info type (2) and Result (2)
    if ( len >= INFO_RSP_FIXED_SIZE )
    {
        l2capInfoRsp_t* pRsp = &pCmd->infoRsp;
        // Info type
        pRsp->infoType = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
        // Result
        pRsp->result = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;

        if ( pRsp->result == L2CAP_INFO_SUCCESS )
        {
            // Parse the contents of Data field
            if ( pRsp->infoType == L2CAP_INFO_EXTENDED_FEATURES )
            {
                // Check for variable fields: Extended Features
                if ( len == ( INFO_RSP_FIXED_SIZE + L2CAP_EXTENDED_FEATURES_SIZE ) )
                {
                    // Extended Features mask
                    pRsp->info.extendedFeatures = osal_build_uint32( pData, L2CAP_EXTENDED_FEATURES_SIZE );
                    return ( SUCCESS );
                }
            }
            else if ( pRsp->infoType == L2CAP_INFO_FIXED_CHANNELS )
            {
                // Check for variable fields: Fixed Channels
                if ( len == ( INFO_RSP_FIXED_SIZE + L2CAP_FIXED_CHANNELS_SIZE ) )
                {
                    // Fixed Channels mask
                    VOID osal_memcpy( pRsp->info.fixedChannels, pData, L2CAP_FIXED_CHANNELS_SIZE );
                    return ( SUCCESS );
                }
            }

            // else no Data for L2CAP_INFO_CONNLESS_MTU
        }
        else if ( pRsp->result == L2CAP_INFO_NOT_SUPPORTED )
        {
            // No Data field
            if ( len == INFO_RSP_FIXED_SIZE )
            {
                return ( SUCCESS );
            }
        }
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      l2capbuildParamUpdateReq

    @brief   Build Connection Parameter Update Request.

    @param   pBuf - pointer to buffer to hold command data
    @param   pData - pointer to command data

    @return  length of the command data
*/
uint16 l2capBuildParamUpdateReq( uint8* pBuf, uint8* pData )
{
    l2capParamUpdateReq_t* pCmd = (l2capParamUpdateReq_t*)pData;
    // Interval min
    *pBuf++ = LO_UINT16( pCmd->intervalMin );
    *pBuf++ = HI_UINT16( pCmd->intervalMin );
    // Interval max
    *pBuf++ = LO_UINT16( pCmd->intervalMax );
    *pBuf++ = HI_UINT16( pCmd->intervalMax );
    // Slave Latency
    *pBuf++ = LO_UINT16( pCmd->slaveLatency );
    *pBuf++ = HI_UINT16( pCmd->slaveLatency );
    // Timeout Multiplier
    *pBuf++ = LO_UINT16( pCmd->timeoutMultiplier );
    *pBuf   = HI_UINT16( pCmd->timeoutMultiplier );
    return ( PARAM_UPDATE_REQ_SIZE );
}

/*********************************************************************
    @fn      L2CAP_ParseParamUpdateReq

    @brief   Parse Connection Parameter Update Request.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
            FAILURE: Command length is invalid.
*/
bStatus_t L2CAP_ParseParamUpdateReq( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    if ( len == PARAM_UPDATE_REQ_SIZE )
    {
        l2capParamUpdateReq_t* pReq = &pCmd->updateReq;
        // Interval min
        pReq->intervalMin = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
        // Interval max
        pReq->intervalMax = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
        // Slave Latency
        pReq->slaveLatency = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
        // Timeout Multiplier
        pReq->timeoutMultiplier = BUILD_UINT16( pData[0], pData[1] );
        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      L2CAP_BuildParamUpdateRsp

    @brief   Build Connection Parameter Update Response.

    @param   pBuf - pointer to buffer to hold command data
    @param   pData - pointer to command data

    @return  length of the command data
*/
uint16 L2CAP_BuildParamUpdateRsp( uint8* pBuf, uint8* pData )
{
    l2capParamUpdateRsp_t* pRsp = (l2capParamUpdateRsp_t*)pData;
    // Result
    *pBuf++ = LO_UINT16( pRsp->result );
    *pBuf   = HI_UINT16( pRsp->result );
    return ( PARAM_UPDATE_RSP_SIZE );
}

/*********************************************************************
    @fn      l2capParseParamUpdateRsp

    @brief   Parse Connection Parameter Update Response.

    @param   pCmd - pointer to command data to be built
    @param   pData - pointer to incoming command data to be parsed
    @param   len - length of incoming command data

    @return  SUCCESS: Command was parsed successfully.
            FAILURE: Command length is invalid.
*/
bStatus_t l2capParseParamUpdateRsp( l2capSignalCmd_t* pCmd, uint8* pData, uint16 len )
{
    if ( len == PARAM_UPDATE_RSP_SIZE )
    {
        l2capParamUpdateRsp_t* pRsp = &pCmd->updateRsp;
        // Result
        pRsp->result = BUILD_UINT16( pData[0], pData[1] );
        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn          l2capNotifyData

    @brief       Forward an incoming data message to upper layer application
                or protocol.

                Note: Application must free packet 'pPayload' using osal_bm_free().

    @param       taskId - application task
    @param       connHandle - connection data was received on
    @param       pPkt - pointer to received packet

    @return      SUCCESS: Message was sent.
                FAILURE: Message was not sent.
*/
bStatus_t l2capNotifyData( uint8 taskId, uint16 connHandle, l2capPacket_t* pPkt )
{
    l2capDataEvent_t* pMsg;
    pMsg = (l2capDataEvent_t*)osal_msg_allocate( sizeof( l2capDataEvent_t ) );

    if ( pMsg != NULL )
    {
        // Set up the OSAL message header
        pMsg->hdr.event = L2CAP_DATA_EVENT;
        pMsg->hdr.status = SUCCESS;
        pMsg->connHandle = connHandle;

        if ( pPkt != NULL )
        {
            VOID osal_memcpy( &(pMsg->pkt), pPkt, sizeof( l2capPacket_t ) );
        }
        else
        {
            VOID osal_memset( &(pMsg->pkt), 0, sizeof( l2capPacket_t ) );
        }

        // Forward the data message up to the application
        VOID osal_msg_send( taskId, (uint8*)pMsg );
        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn          l2capNotifySignal

    @brief       Send a Signaling command to upper layer application/protocol.

    @param       taskId - application task
    @param       connHandle - connection event belongs to
    @param       status - status
    @param       opcode - type of signaling command
    @param       id - identifier (applicable to requests only)
    @param       pCmd - pointer to command data

    @return      none
*/
void l2capNotifySignal( uint8 taskId, uint16 connHandle, uint8 status,
                        uint8 opcode, uint8 id, l2capSignalCmd_t* pCmd )
{
    l2capSignalEvent_t* pEvent;
    pEvent = (l2capSignalEvent_t*)osal_msg_allocate( sizeof( l2capSignalEvent_t ) );

    if ( pEvent != NULL )
    {
        // Set up the OSAL message header
        pEvent->hdr.event = L2CAP_SIGNAL_EVENT;
        pEvent->hdr.status = status;
        pEvent->connHandle = connHandle;
        pEvent->id = id;
        pEvent->opcode = opcode;

        if ( pCmd != NULL )
        {
            VOID osal_memcpy( &(pEvent->cmd), pCmd, sizeof( l2capSignalCmd_t ) );
        }
        else
        {
            VOID osal_memset( &(pEvent->cmd), 0, sizeof( l2capSignalCmd_t ) );
        }

        // Forward the signaling message up to the application
        VOID osal_msg_send( taskId, (uint8*)pEvent );
    }
}

/*********************************************************************
    @fn      l2capAllocChannel

    @brief   Allocate a channel.

    @param   taskId - task the channel to be allocated for

    @return  Pointer to channel, if allocated. NULL, otherwise.
*/
l2capChannel_t* l2capAllocChannel( uint8 taskId )
{
    uint8 i;

    for ( i = 0; i < L2CAP_NUM_CHANNELS; i++ )
    {
        if ( l2capChannels[i].CID == L2CAP_CID_NULL )
        {
            l2capChannels[i].state = L2CAP_CLOSED;
            l2capChannels[i].CID = L2CAP_BASE_DYNAMIC_CID + i;
            l2capChannels[i].taskId = taskId;
            return ( &l2capChannels[i] );
        }
    }

    return ( (l2capChannel_t*)NULL );
}

/*********************************************************************
    @fn      l2capAllocConnChannel

    @brief   Allocate a channel for a given physical connection.

    @param   connHandle - connection the channel to be allocated for
    @param   taskId - task the channel to be allocated for
    @param   p2pChannel - pointer to pointer to channel info (to be returned)

    @return  SUUCESS: Channel allocated.
            FAILURE: p2pChannel is NULL.
            bleNoResources: No available resource.
            bleNotConnected: Connection is down.
*/
static bStatus_t l2capAllocConnChannel( uint16 connHandle, uint8 taskId, l2capChannel_t** p2pChannel )
{
    uint8 status;

    // Make sure we can always return a pointer to the allocated channel
    if ( p2pChannel != NULL )
    {
        // Make sure the physical connection is up
        if ( linkDB_Up( connHandle ) )
        {
            l2capChannel_t* pChannel = l2capAllocChannel( taskId );

            if ( pChannel != NULL )
            {
                // Channel was allocated
                pChannel->connHandle = connHandle;
                *p2pChannel = pChannel;
                status = SUCCESS;
            }
            else
            {
                status = bleNoResources;
            }
        }
        else
        {
            status = bleNotConnected;
        }
    }
    else
    {
        status = FAILURE;
    }

    return ( status );
}

/*********************************************************************
    @fn      l2capFreeChannel

    @brief   Free a channel.

    @param   pChannel - pointer to channel to be freed

    @return  none
*/
void l2capFreeChannel( l2capChannel_t* pChannel )
{
    pChannel->CID = L2CAP_CID_NULL;
    pChannel->state = L2CAP_CLOSED;
}

/*********************************************************************
    @fn      l2capFindLocalId

    @brief   Find a channel using the local identifier.

    @param   id - local identfier to look for

    @return  Pointer to channel, if found. NULL, otherwise.
*/
l2capChannel_t* l2capFindLocalId( uint8 id )
{
    uint8 i;

    for ( i = 0; i < L2CAP_NUM_CHANNELS; i++ )
    {
        if ( ( l2capChannels[i].CID != L2CAP_CID_NULL ) && ( l2capChannels[i].id == id ) )
        {
            // Entry found
            return ( &l2capChannels[i] );
        }
    }

    return ( (l2capChannel_t*)NULL );
}

/*********************************************************************
    @fn      l2capStartTimer

    @brief   Start a timer for a give channel.

    @param   pChannel - channel to start a timer for
    @param   timeout - timeout in seconds

    @return  none
*/
static void l2capStartTimer( l2capChannel_t* pChannel, uint16 timeout )
{
    // Timeout is in msec
    osal_CbTimerStart( l2capHandleTimerCB, (uint8*)pChannel,
                       (timeout * 1000), &pChannel->timerId );
}

/*********************************************************************
    @fn      l2capStopTimer

    @brief   Stop an active timer for a given channel.

    @param   pChannel - channel with active timer

    @return  none
*/
void l2capStopTimer( l2capChannel_t* pChannel )
{
    // Stop the timer
    VOID osal_CbTimerStop( pChannel->timerId );
    // Reset timer id
    pChannel->timerId = INVALID_TIMER_ID;
}

/*********************************************************************
    @fn      l2capHandleTimerCB

    @brief   Handle a callback for a timer that has just expired.

    @param   pData - pointer to timer data

    @return  none
*/
static void l2capHandleTimerCB( uint8* pData )
{
    l2capChannel_t* pChannel = (l2capChannel_t*)pData;

    // Response timer has expired
    if ( ( pChannel != NULL ) && ( pChannel->CID != L2CAP_CID_NULL ) )
    {
        uint8 opcode;
        // Find out the response type
        #if 0 // No longer supported

        if ( pChannel->state == L2CAP_W4_ECHO_RSP )
        {
            opcode = L2CAP_ECHO_RSP;
        }
        else if ( pChannel->state == L2CAP_W4_INFO_RSP )
        {
            opcode = L2CAP_INFO_RSP;
        }
        else
        #endif
        {
            opcode = L2CAP_PARAM_UPDATE_RSP;
        }

        // Notify the application about the timeout
        l2capNotifySignal( pChannel->taskId, pChannel->connHandle, bleTimeout, opcode, 0, NULL );
        // Reset timer id
        pChannel->timerId = INVALID_TIMER_ID;
        // Free the channel
        l2capFreeChannel( pChannel );
    }
}

/*********************************************************************
    @fn      l2capHandleRxError

    @brief   Handle an incoming packet error.

    @param   connHandle - connection error occurred on

    @return  none
*/
void l2capHandleRxError( uint16 connHandle )
{
    // Close the channel if it's dynamic -- not supported for now!
    VOID connHandle;
}

/*********************************************************************
    @fn      L2CAP_bm_alloc

    @brief   L2CAP implementation of the allocator functionality.

            Note: This function should only be called by L2CAP and
                  the upper layer protocol/application.

    @param   size - number of bytes to allocate from the heap.

    @return  pointer to the heap allocation; NULL if error or failure.
*/
void* L2CAP_bm_alloc( uint16 size )
{
    uint8* pBuf;
    pBuf = HCI_bm_alloc( size + L2CAP_HDR_SIZE );

    if ( pBuf != NULL )
    {
        // return pointer to user payload
        return ( osal_bm_adjust_header( pBuf, -L2CAP_HDR_SIZE ) );
    }

    return ( (void*)NULL );
}

/*********************************************************************
    @fn          L2CAP_SendDataPkt

    @brief       Loop back a data packet.

    @param       connID - Connection handle
    @param       pktLen - Number of bytes of data to transmit
    @param       pData - Pointer to data buffer to transmit

    @return  SUCCESS: Request was sent successfully.
            bleMemAllocError: No buffer is available.
*/
static bStatus_t L2CAP_SendDataPkt( uint16 connHandle, uint16 pktLen, uint8* pData )
{
    hciDataEvent_t* pMsg;
    // Loop it back to L2CAP task for now
    pMsg = (hciDataEvent_t*)osal_msg_allocate( sizeof( hciDataEvent_t ) );

    if ( pMsg != NULL )
    {
        // Set up the OSAL message header
        pMsg->hdr.event = HCI_DATA_EVENT;
        pMsg->pbFlag = FIRST_PKT_CTRL_TO_HOST;
        pMsg->connHandle = connHandle;
        pMsg->pData = pData;
        pMsg->len = pktLen;
        // send message through task message
        VOID osal_msg_send( l2capTaskID, (uint8*)pMsg );
        return ( SUCCESS );
    }

    return ( bleMemAllocError );
}

/*********************************************************************
    @fn      L2CAP_Fragment_bm_alloc

    @brief   L2CAP implementation of the fragment allocator functionality.

            Note: This function should only be called by L2CAP and
                  the upper layer protocol/application.

    @param   size - number of bytes to allocate from the heap.

    @return  pointer to the heap allocation; NULL if error or failure.
*/
void* L2CAP_Fragment_bm_alloc( uint16 size )
{
    return( LL_TX_bm_alloc( size ) );
}



__ATTR_SECTION_SRAM__ uint8 L2CAP_Fragment_SendDataPkt( uint16 connHandle, uint8 fragFlg,uint16 pktLen, uint8* pBuf )
{
    // Send the data packet to the Controller
    // Note: Host to Controller Data Flow Control is not supported yet!
    //       Host should use the LE Read Buffer Size command to determine
    //       the maximum size and the total number of HCI Data Packets.
    uint8 status = HCI_SendDataPkt( connHandle, fragFlg, pktLen, pBuf );

//LOG("%s,status %d\n",__func__,status);
    // Map HCI status codes to Host status codes
    switch ( status )
    {
    case HCI_ERROR_CODE_UNKNOWN_CONN_ID:
        status = bleNotConnected;
        break;

    case HCI_ERROR_CODE_INVALID_HCI_CMD_PARAMS:
        status = INVALIDPARAMETER;
        break;

    case HCI_ERROR_CODE_MEM_CAP_EXCEEDED:
        status = MSG_BUFFER_NOT_AVAIL;
        g_sarDbgCnt.segmentSentToLinkLayerErr++;
        break;

    default:
        // Should never get here!
        break;
    }

    return status;
}

void l2capSarBufReset(void)
{
    int i, j;

    for ( i = 0; i < MAX_NUM_LL_CONN; i ++)
    {
        l2capReassemblePkt[i].cIdx   =0;
        l2capReassemblePkt[i].pkt.len=0;
        l2capReassemblePkt[i].pkt.CID=L2CAP_CID_NULL;

        if(l2capReassemblePkt[i].pkt.pPayload!=NULL)
        {
            osal_bm_free(l2capReassemblePkt[i].pkt.pPayload);
            l2capReassemblePkt[i].pkt.pPayload=NULL;
        }

        l2capSegmentPkt[i].depth=0;
        l2capSegmentPkt[i].idx=0;
        l2capSegmentPkt[i].fragment =0;

        if(l2capSegmentPkt[i].pBufScr!=NULL)
        {
            osal_bm_free(l2capSegmentPkt[i].pBufScr);
            l2capSegmentPkt[i].pBufScr=NULL;
        }

        for(j = 0; j < 10; j++)
        {
            if(l2capSegmentPkt[i].pkt[j].ptr!=NULL)
                osal_bm_free(l2capSegmentPkt[i].pkt[j].ptr);
        }
    }

    LOG("[SAR_FREE]\n");
    return;
}

void L2CAP_ReassemblePkt_Reset(uint16 connHandle)
{
    if (connHandle >= MAX_NUM_LL_CONN)
        return;

    l2capReassemblePkt[connHandle].cIdx   =0;
    l2capReassemblePkt[connHandle].pkt.len=0;
    l2capReassemblePkt[connHandle].pkt.CID=L2CAP_CID_NULL;

    if(l2capReassemblePkt[connHandle].pkt.pPayload!=NULL)
    {
        osal_bm_free(l2capReassemblePkt[connHandle].pkt.pPayload);
        l2capReassemblePkt[connHandle].pkt.pPayload=NULL;
    }

    LOG("[REA_FREE]\n");
    return;
}

void L2CAP_SegmentPkt_Reset(uint16 connHandle)
{
    if (connHandle >= MAX_NUM_LL_CONN)
        return;

    l2capSegmentPkt[connHandle].depth=0;
    l2capSegmentPkt[connHandle].idx=0;
    l2capSegmentPkt[connHandle].fragment =0;

    if(l2capSegmentPkt[connHandle].pBufScr!=NULL)
    {
        osal_bm_free(l2capSegmentPkt[connHandle].pBufScr);
        l2capSegmentPkt[connHandle].pBufScr=NULL;
    }

    for(uint8 j = 0; j < 10; j++)
    {
        if(l2capSegmentPkt[connHandle].pkt[j].ptr!=NULL)
            osal_bm_free(l2capSegmentPkt[connHandle].pkt[j].ptr);
    }

    LOG("[SEG_FREE]\n");
    return;
}


// TODO: whether we need this function
uint8 l2capPktToSegmentBuff(uint16 connHandle, l2capSegmentBuff_t* pSegBuf, uint8 blen,uint8* pBuf)
{
    uint8 llBufLimit =  conn_param[connHandle].llPduLen.local.MaxTxOctets;//LL_MAX_LINK_DATA_LEN;

    if(llBufLimit>ATT_GetCurrentMTUSize(connHandle)+L2CAP_HDR_SIZE)
        llBufLimit = ATT_GetCurrentMTUSize(connHandle)+L2CAP_HDR_SIZE;

//  LOG("%s,blen %d,llBufLimit %d\n",__func__,blen,llBufLimit);
    if(blen<=llBufLimit)
    {
        #if (0)
        //similar logical for NoSegment
        pSegBuf->depth= 1;
        pSegBuf->pBufScr = pBuf;
        pSegBuf->pkt[0].len = blen;
        pSegBuf->pkt[0].ptr= (uint8*) L2CAP_Fragment_bm_alloc(pSegBuf->pkt[0].len);
        LOG("[SEG_PRT]%08x \n",pSegBuf->pkt[0].ptr);//pBuf = pSegBuf->pkt[0].ptr-sizeof(txData_t)-LL_PKT_HDR_LEN) (-4-2)

        if(pSegBuf->pkt[0].ptr!=NULL)
        {
            osal_memcpy(pSegBuf->pkt[0].ptr, pBuf, pSegBuf->pkt[0].len);
        }
        else
        {
            pSegBuf->pBufScr = NULL;
            pSegBuf->depth= 0;
            return bleMemAllocError;
        }

        #else
        //20181124 ZQ
        //no segment happend ,just send data to LL_buf, no memory copy
        return( L2CAP_Fragment_SendDataPkt(connHandle,LL_DATA_FIRST_PKT_HOST_TO_CTRL,blen,pBuf));
        #endif
    }
    else
    {
        pSegBuf->depth= blen /llBufLimit;
        pSegBuf->pBufScr = pBuf;
        uint8 resLen = blen - pSegBuf->depth*llBufLimit;
        uint8 i;
        uint8* pBufTmp= pBuf;

        if(resLen>0)
            pSegBuf->depth+=1;

//        if(     l2capExtendFragments==FALSE
//            &&  pSegBuf->depth > getTxBufferFree())
//        {
//            pSegBuf->depth= 0;
//            pSegBuf->pBufScr = NULL;
//            return HCI_ERROR_CODE_MEM_CAP_EXCEEDED;
//        }

        for(i=0; i<pSegBuf->depth; i++)
        {
            pSegBuf->pkt[i].len = (0==resLen)             ? llBufLimit :
                                  ((i== pSegBuf->depth-1) ? resLen     : llBufLimit);
            pSegBuf->pkt[i].ptr = (uint8*) L2CAP_Fragment_bm_alloc(pSegBuf->pkt[i].len);
            LOG("[SEG_PRT]%08x %02x\n",pSegBuf->pkt[i].ptr,*pBufTmp);

            if(pSegBuf->pkt[i].ptr!=NULL)
            {
                osal_memcpy(pSegBuf->pkt[i].ptr, pBufTmp, pSegBuf->pkt[i].len);
                pBufTmp+=pSegBuf->pkt[i].len;
                g_sarDbgCnt.segmentInCnt++;
            }
            else
            {
                pSegBuf->depth= 0;
                pSegBuf->pBufScr = NULL;

                //free 0->i-1 bm_buff
                for(uint8 k=0; k<i; k++)
                    osal_bm_free(pSegBuf->pkt[k].ptr);

                g_sarDbgCnt.segmentMemAlocErr++;
                return bleMemAllocError;
            }
        }

        //free source buf
        if(pSegBuf->pBufScr!=NULL)
        {
            osal_bm_free(pSegBuf->pBufScr);
            pSegBuf->pBufScr = NULL;
        }
    }

    return SUCCESS;
}


__ATTR_SECTION_SRAM__ uint8 l2capSegmentBuffToLinkLayer(uint16 connHandle, l2capSegmentBuff_t* pSegBuf)
{
    uint8 status;

    while(pSegBuf->depth>0)
    {
        status = L2CAP_Fragment_SendDataPkt(connHandle,
                                            (0==pSegBuf->idx) ? LL_DATA_FIRST_PKT_HOST_TO_CTRL : CONTINUING_PKT,
                                            pSegBuf->pkt[pSegBuf->idx].len,
                                            pSegBuf->pkt[pSegBuf->idx].ptr);

        if(status==SUCCESS)
        {
            //osal_bm_free(pSegBuf->pkt[pSegBuf->idx].ptr);
            //when success,pkt[].ptr will be free in llProcessTxData()
            //should not be free again
            pSegBuf->pkt[pSegBuf->idx].ptr=NULL;
            pSegBuf->idx++;
            pSegBuf->depth--;
            g_sarDbgCnt.segmentOutCnt++;
        }
        else
        {
            pSegBuf->fragment=TRUE;
            return status;
        }
    }

    //--------------------------------------------
    //sbuf shold be sented
    //reset idx
    pSegBuf->idx=0;
//    //free source buf
//    if(pSegBuf->pBufScr!=NULL)
//    {
//        osal_bm_free(pSegBuf->pBufScr);
//        pSegBuf->pBufScr = NULL;
//    }
    //no fragment pkt remained
    pSegBuf->fragment=FALSE;
    return SUCCESS;
}

__ATTR_SECTION_SRAM__ void l2capPocessFragmentTxData(uint16 connHandle)
{
    if(l2capSegmentPkt[connHandle].fragment == TRUE)
    {
        l2capSegmentBuffToLinkLayer(connHandle, &l2capSegmentPkt[connHandle]);
        g_sarDbgCnt.fragmentSendCounter++;
    }

    return;
}

void L2CAP_ExtendFramgents_Config(uint8 flag)
{
    l2capExtendFragments= flag;
    return;
}


/****************************************************************************
****************************************************************************/
