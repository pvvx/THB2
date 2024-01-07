
/*********************************************************************
    INCLUDES
*/
#include "ppsp_impl.h"
#include "ppsp_serv.h"
#include "error.h"
#include "OSAL.h"
#include "core_queu.h"
#include "log.h"
#include "flash.h"


#define PPSP_IMPL_CFGS_BUSY_STAT_BITS   0x00000001
#define PPSP_IMPL_CFGS_AUTH_STAT_BITS   0x00000002
#define PPSP_IMPL_CFGS_OTAS_STAT_BITS   0x00000004  // ota bgns
#define PPSP_IMPL_CFGS_OTAE_STAT_BITS   0x00000008  // ota ends

#define PPSP_IMPL_CFGS_ALIS_PIDS_COUN   4
#define PPSP_IMPL_CFGS_ALIS_MACS_COUN   6
#define PPSP_IMPL_CFGS_ALIS_SCRT_COUN   16

#define PPSP_IMPL_CFGS_MSGS_CRYP_ENAB   0
#if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    #include "sha256.h"
    #include "tinycrypt/aes.h"
#endif

#define PPSP_IMPL_CFGS_MSGS_HDER_SIZE   4

#define PPSP_IMPL_CFGS_OPCO_MISN_ISSU   0x02    // issu by master, resp by slave
#define PPSP_IMPL_CFGS_OPCO_MISR_RESP   0x03    // resp by slave, issu by master
#define PPSP_IMPL_CFGS_OPCO_MNSI_ISSU   0x04    // issu by slave, resp by master
#define PPSP_IMPL_CFGS_OPCO_MRSI_RESP   0x05    // resp by master, issu by slave
#define PPSP_IMPL_CFGS_OPCO_NRSP_ISSU   0x06    // issu W/O resp

#define PPSP_IMPL_CFGS_OPCO_RAND_ISSU   0x10    // issu RAND by master, resp by slave
#define PPSP_IMPL_CFGS_OPCO_CIPR_RESP   0x11    // resp RAND by slave, issu by master. CIPR = ENCR(RAND, BKEY(RAND,MACS,SCRT))
#define PPSP_IMPL_CFGS_OPCO_VERF_ISSU   0x12    // issu VERF by master, resp by slave. verify result of encryption
#define PPSP_IMPL_CFGS_OPCO_BKEY_RESP   0x13    // resp VERF by slave, issu by master
#define PPSP_IMPL_CFGS_OPCO_NWRK_ISSU   0x14    // issu NWRK by master, resp by slave. networking rsult, unprov/proved
#define PPSP_IMPL_CFGS_OPCO_NWRK_RESP   0x15    // resp NWRK by slave, issu by master. confirm of networking rsult

#define PPSP_IMPL_CFGS_OPCO_VERS_ISSU   0x20    // issu VERS by master, resp by slave
#define PPSP_IMPL_CFGS_OPCO_VERS_RESP   0x21    // resp VERS by slave, issu by master
#define PPSP_IMPL_CFGS_OPCO_UPDA_ISSU   0x22    // issu UPDA by master, resp by slave
#define PPSP_IMPL_CFGS_OPCO_UPDA_RESP   0x23    // resp UPDA by slave, issu by master. confirm, rcvd size, fast mode
#define PPSP_IMPL_CFGS_OPCO_PACK_ISSU   0x2F    // issu PACK by master, issu W/O resp.
#define PPSP_IMPL_CFGS_OPCO_PACK_RESP   0x24    // issu PACK by slave, issu W/O resp.  confirm of total frame
#define PPSP_IMPL_CFGS_OPCO_COMP_ISSU   0x25    // issu COMP by master, resp by slave. complete of transfer
#define PPSP_IMPL_CFGS_OPCO_COMP_RESP   0x26    // resp COMP by slave, issu by master. reply with crc

#define PPSP_IMPL_CFGS_OPCO_USER_ISSU   0xFE    // issu USER, resp by slave
#define PPSP_IMPL_CFGS_OPCO_USER_RESP   0xFF    // issu USER, resp by slave

#define PPSP_IMPL_CFGS_PROG_ADDR_BASE   (0x11000000)
#define PPSP_IMPL_CFGS_PROG_SCTR_SIZE   (0x1000)    // program sector size in byte
#define PPSP_IMPL_CFGS_PROG_ADDR_BGNS   (0x55000)   // program data bgn address of flash
#define PPSP_IMPL_CFGS_PROG_FLSH_SIZE   (0x2B000)   // program total size in byte
#define PPSP_IMPL_CFGS_PROG_ADDR_ENDS   (PPSP_IMPL_CFGS_PROG_ADDR_BGNS+PPSP_IMPL_CFGS_PROG_FLSH_SIZE)    // program data end address of flash

#define PPSP_IMPL_CFGS_PROG_VERS_REVI   (0)
#define PPSP_IMPL_CFGS_PROG_VERS_MINR   (0)
#define PPSP_IMPL_CFGS_PROG_VERS_MAJR   (1)


extern void
ppsp_impl_serv_rcvd_hdlr(uint8 para, uint16 coun);
static ppsp_serv_appl_CBs_t
__ppsp_impl_hdlr_serv =
{
    ppsp_impl_serv_rcvd_hdlr,
};

static uint32
__ppsp_impl_stat_bits_flag = 0x00;
static uint32
__ppsp_impl_proc_tout_coun = 0x00;


static core_sque_t*
__ppsp_impl_msgs_queu_rcvd = NULL;  // received msgs queue
static core_sque_t*
__ppsp_impl_msgs_queu_xfer = NULL;  // transfer msgs queue


static const uint8
__ppsp_impl_opco_prim_list[] =
{
    PPSP_IMPL_CFGS_OPCO_MISN_ISSU,
    PPSP_IMPL_CFGS_OPCO_MNSI_ISSU,
    PPSP_IMPL_CFGS_OPCO_MISR_RESP,
    PPSP_IMPL_CFGS_OPCO_MRSI_RESP,
    PPSP_IMPL_CFGS_OPCO_NRSP_ISSU,

    PPSP_IMPL_CFGS_OPCO_RAND_ISSU,
    PPSP_IMPL_CFGS_OPCO_CIPR_RESP,
    PPSP_IMPL_CFGS_OPCO_VERF_ISSU,
    PPSP_IMPL_CFGS_OPCO_BKEY_RESP,
    PPSP_IMPL_CFGS_OPCO_NWRK_ISSU,
    PPSP_IMPL_CFGS_OPCO_NWRK_RESP,

    PPSP_IMPL_CFGS_OPCO_VERS_ISSU,
    PPSP_IMPL_CFGS_OPCO_VERS_RESP,
    PPSP_IMPL_CFGS_OPCO_UPDA_ISSU,
    PPSP_IMPL_CFGS_OPCO_UPDA_RESP,
    PPSP_IMPL_CFGS_OPCO_PACK_ISSU,
    PPSP_IMPL_CFGS_OPCO_PACK_RESP,
    PPSP_IMPL_CFGS_OPCO_COMP_ISSU,
    PPSP_IMPL_CFGS_OPCO_COMP_RESP,

    PPSP_IMPL_CFGS_OPCO_USER_ISSU,
    PPSP_IMPL_CFGS_OPCO_USER_RESP,
};
static uint8
__ppsp_impl_opco_prim_coun = sizeof(__ppsp_impl_opco_prim_list);

static uint8
__ppsp_impl_opco_user_coun = 0;
static uint8*
__ppsp_impl_opco_user_list = 0;

#if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    static uint8
    __ppsp_impl_auth_keys_data[16];
#endif

/*
    private function prototype
*/
// static uint8
// ppsp_impl_psh_msgs_xfer(const uint8* data, uint16 leng);




/*
    private function implimentation
*/
#define ppsp_impl_get_auth_rslt(rslt)                       \
    {                                                           \
        (rslt)  = ((__ppsp_impl_stat_bits_flag & PPSP_IMPL_CFGS_AUTH_STAT_BITS)?1:0);   \
    }

#define ppsp_impl_set_auth_rslt(flag)                       \
    {                                                           \
        if ( 0 == (flag) )                                      \
            __ppsp_impl_stat_bits_flag &= ~PPSP_IMPL_CFGS_AUTH_STAT_BITS;   \
        else                                                    \
            __ppsp_impl_stat_bits_flag |= PPSP_IMPL_CFGS_AUTH_STAT_BITS;    \
    }

#define ppsp_impl_get_msgs_numb(msgs, numb)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            (numb)  = msgs[0]&0x0F;                             \
    }

#define ppsp_impl_set_msgs_numb(msgs, numb)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            msgs[0] |= (numb)&0x0F;                             \
    }

#define ppsp_impl_get_msgs_encr(msgs, numb)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            (numb) = ((msgs[0]&0x10)>>4);                      \
    }

#define ppsp_impl_set_msgs_encr(msgs, flag)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            msgs[0] |= (((flag)&0x01)<<4);                      \
    }

#define ppsp_impl_get_msgs_opco(msgs, opco)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            (opco) = msgs[1]&0xFF;                              \
    }

#define ppsp_impl_set_msgs_opco(msgs, opco)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            msgs[1] = (opco)&0xFF;                              \
    }

#define ppsp_impl_get_msgs_seqn(msgs, alln, seqn)           \
    {                                                           \
        if ( 0 != msgs ) {                                      \
            alln = (msgs[2]&0xF0)>>4;                           \
            seqn = (msgs[2]&0x0F)>>0;                           \
        }                                                       \
    }

#define ppsp_impl_set_msgs_seqn(msgs, alln, seqn)           \
    {                                                           \
        if ( 0 != msgs )                                        \
            msgs[2] = ((alln&0x0F))<<4 | ((seqn&0x0F)<<0);      \
    }

/*
    desc: get msg payload length, byte 3 of header
*/
#define ppsp_impl_get_msgs_frsz(msgs, frsz)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            (frsz) = msgs[3]&0xFF;                              \
    }

#define ppsp_impl_set_msgs_frsz(msgs, frsz)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            msgs[3] = (frsz)&0xFF;                              \
    }

/*
    desc: get msg payload
*/
#define ppsp_impl_get_msgs_plds(msgs, data)                 \
    {                                                           \
        if ( 0 != msgs )                                        \
            (/* (uint8*) */(data)) = (((uint8*)(msgs))+PPSP_IMPL_CFGS_MSGS_HDER_SIZE);    \
    }

/*
    desc: set msg payload
*/
#define ppsp_impl_set_msgs_plds(msgs, data, coun)           \
    {                                                           \
        if ( 0 != (uint8*)(msgs) )                              \
            osal_memcpy(((uint8*)(msgs))+PPSP_IMPL_CFGS_MSGS_HDER_SIZE, (uint8*)(data), coun);  \
    }

/*
    desc: PKCS#7 padding
*/
#define ppsp_impl_get_pkcs_7pad(bksz, dasz, pval)           \
    {                                                           \
        pval = bksz - (dasz % bksz);                            \
    }

/*
    desc: chk mesg package size
*/
static uint8
ppsp_impl_chk_msgs_leng(const uint8* mesg, uint16 coun)
{
    return ( NULL != mesg && PPSP_IMPL_CFGS_MSGS_HDER_SIZE <= coun );
}

/*
    desc: chk mesg id
*/
static uint8
ppsp_impl_chk_msgs_numb(const uint8* mesg, uint16 coun)
{
    uint8 rslt = 0;

    if ( 0 != mesg )
    {
        uint8 numb;
        ppsp_impl_get_msgs_numb(mesg, numb);
        rslt = (/* (0 <= numb) &&  */(15 >= numb)); // comment for compuler warning
    }
    else
    {
        rslt = 0;
    }

    return ( rslt );
}

/*
    desc: chk mesg opcode
*/
static uint8
ppsp_impl_chk_msgs_opco(const uint8* mesg, uint16 coun)
{
    uint8 rslt = 0;

    if ( NULL != mesg )
    {
        uint8 opco;
        opco = mesg[1] & 0xFF;

        for ( int itr0 = 0; itr0 <  __ppsp_impl_opco_prim_coun; itr0 += 1 )
        {
            if ( opco == __ppsp_impl_opco_prim_list[itr0] )
            {
                rslt = 1;
                break;
            }
        }

        if ( !rslt )
            for ( int itr0 = 0; itr0 <  __ppsp_impl_opco_user_coun; itr0 += 1 )
            {
                if ( opco == __ppsp_impl_opco_user_list[itr0] )
                {
                    rslt = 1;
                    break;
                }
            }
    }

    return ( rslt );
}

/*
    desc: chk mesg segment number + total segment number
*/
static uint8
ppsp_impl_chk_msgs_seqn(const uint8* mesg, uint16 coun)
{
    uint8 rslt = 1;

    if ( NULL != mesg )
    {
        uint8 numb;
        /* numb of segment sequence */
        numb = (mesg[2] & 0x0F) >> 0;
        rslt = (/* 0 <= numb &&  */(15 >= numb));   // comment for compuler warning
    }

    if ( 1 == rslt )
    {
        uint8 numb;
        /* numb of total segments */
        numb = (mesg[2] & 0xF0)  >> 4;
        rslt = (/* 0 <= numb &&  */(15 >= numb));   // comment for compuler warning
    }

    return ( rslt );
}

/*
    desc: create response message
*/
static void*
ppsp_impl_new_msgs_resp(uint8 numb, uint8 opco, uint8* data, uint16 leng)
{
    uint8* msgs = 0;
    /* only consider unsegmented case */
    msgs = osal_mem_alloc(PPSP_IMPL_CFGS_MSGS_HDER_SIZE+leng);

    if ( 0 != msgs )
    {
        osal_memset(msgs, 0, PPSP_IMPL_CFGS_MSGS_HDER_SIZE+leng);
        ppsp_impl_set_msgs_numb(msgs, numb);
        // ppsp_impl_set_msgs_encr(msgs, numb);
        // ppsp_impl_set_msgs_vers(msgs, numb);
        ppsp_impl_set_msgs_opco(msgs, opco);
        ppsp_impl_set_msgs_seqn(msgs, 0, 0);    // frame numb, frame sequ
        ppsp_impl_set_msgs_frsz(msgs, leng);    // frame size
        ppsp_impl_set_msgs_plds(msgs, data, leng); // payload
    }

    return ( msgs );
}


static uint8
ppsp_impl_psh_msgs_rcvd(const uint8* mesg, uint16 coun)
{
    uint8 rslt = 1;

    if ( 0 == mesg )
    {
        LOG("[PANDA][ERR] NULL POINTER !!!");
        rslt = 0;
    }

    // for (uint8 itr0 = 0; itr0 < coun; itr0 += 1)
    //    LOG("[PANDA][INF] rcvd msg: data[%d]=%02x", itr0, mesg[itr0]);

    /* */
    if ( 1 == rslt )
    {
        rslt = ppsp_impl_chk_msgs_leng(mesg, coun);
        // LOG("[PANDA][INF] ppsp_impl_chk_msgs_leng: %s\n\r", rslt ? "OK" :"NG");
        // printf("[PANDA][INF] ppsp_impl_chk_msgs_leng: %s\n\r", rslt ? "OK" :"NG");
    }

    /* */
    if ( 1 == rslt )
    {
        rslt = ppsp_impl_chk_msgs_numb(mesg, coun);
        // LOG("[PANDA][ERR] ppsp_impl_chk_msgs_numb: %s\n\r", rslt ? "OK" :"NG");
        // printf("[PANDA][ERR] ppsp_impl_chk_msgs_numb: %s\n\r", rslt ? "OK" :"NG");
    }

    /* */
    if ( 1 == rslt )
    {
        rslt = ppsp_impl_chk_msgs_opco(mesg, coun);
        // LOG("[PANDA][ERR] ppsp_impl_chk_msgs_opco: %s\n\r", rslt ? "OK" :"NG");
        // printf("[PANDA][ERR] ppsp_impl_chk_msgs_opco: %s\n\r", rslt ? "OK" :"NG");
    }

    /* */
    if ( 1 == rslt )
    {
        rslt = ppsp_impl_chk_msgs_seqn(mesg, coun);
        // LOG("[PANDA][ERR] ppsp_impl_chk_msgs_seqn: %s\n\r", rslt ? "OK" :"NG");
        // printf("[PANDA][ERR] ppsp_impl_chk_msgs_seqn: %s\n\r", rslt ? "OK" :"NG");
    }

    /* */
    if ( 1 == rslt )
    {
        uint8* vmsg = osal_mem_alloc(coun);
        osal_memcpy(vmsg, mesg, coun);
        rslt = core_sque_psh(__ppsp_impl_msgs_queu_rcvd, &vmsg);
        // if segment msg, enqueue();
        // else notify upper layer new msgs
        // __ppsp_impl_msgs_rcvd = (uint8*)data;
    }

    // LOG("[EXI] %s(), rslt:%x \r\n", __func__, rslt);
    return ( rslt );
}


static uint8
ppsp_impl_psh_msgs_xfer(const uint8* data, uint16 leng)
{
    return ( core_sque_psh(__ppsp_impl_msgs_queu_xfer, &data) );
}


static void
ppsp_impl_cvt_hex2Str(const unsigned char* const srcs, unsigned char* dest, int coun, int revs)
{
}

#if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
static void
ppsp_impl_prc_msgs_rand(uint8* msgs_rcvd)
{
    uint8   itr0;
    uint8   msgn;
    uint8   rsiz;
    uint8*  rand;
    ppsp_impl_get_msgs_numb(msgs_rcvd, msgn);   // mesg id
    ppsp_impl_get_msgs_frsz(msgs_rcvd, rsiz);   // rand size
    ppsp_impl_get_msgs_plds(msgs_rcvd, rand);   // rand pointer
    uint8   pids[PPSP_IMPL_CFGS_ALIS_PIDS_COUN];
    uint8   macs[PPSP_IMPL_CFGS_ALIS_MACS_COUN];
    uint8   scrt[PPSP_IMPL_CFGS_ALIS_SCRT_COUN];
    //load PID
    ppsp_impl_get_pids(pids);
    //load MAC
    ppsp_impl_get_macs(macs);
    //load SEC
    ppsp_impl_get_scrt(scrt);

    if ( 1 == ppsp_impl_cal_keys(rand, rsiz, pids, sizeof(pids), macs, sizeof(macs), scrt, sizeof(scrt)) )
    {
        LOG("\n\rKEY:>>> ");

        for ( itr0 = 0; itr0 < sizeof(__ppsp_impl_auth_keys_data); itr0 ++ ) LOG("%02x,", __ppsp_impl_auth_keys_data[itr0]);

        LOG("KEY:<<< \n\r");
        uint8* msgs_xfer = 0;
        uint8  cipr[16]  = { 0x00, };  // ciper
        ppsp_impl_enc_text(rand, cipr);
        LOG("\n\rCIP:>>> ");

        for ( itr0 = 0; itr0 < sizeof(cipr); itr0 ++ ) LOG("%02x,", cipr[itr0]);

        LOG("KEY:<<< \n\r");
        msgs_xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_CIPR_RESP, cipr, sizeof(cipr));

        if ( 0 != msgs_xfer ) ppsp_impl_psh_msgs_xfer(msgs_xfer, 20);
    }
    else
    {
        LOG("[PANDA][ERR] gen_aligenie_auth_key FAIL !!!");
    }
}
#endif

static void
ppsp_impl_prc_msgs_verf(uint8* msgs_rcvd)
{
    uint8   msgn;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs_rcvd, msgn);
    ppsp_impl_get_msgs_frsz(msgs_rcvd, frsz);
    ppsp_impl_get_msgs_plds(msgs_rcvd, plds);

    if ( NULL != plds && 0 == plds[0] )
    {
        uint8* msgs_xfer = 0;
        /* Auth Success */
        ppsp_impl_set_auth_rslt(1);
        msgs_xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_BKEY_RESP, plds, frsz);

        if ( 0 != msgs_xfer ) ppsp_impl_psh_msgs_xfer(msgs_xfer, 20);
    }
}

static void
ppsp_impl_prc_msgs_nwrk(uint8* msgs_rcvd)
{
    uint8   msgn;
    uint8   encr;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs_rcvd, msgn);
    ppsp_impl_get_msgs_encr(msgs_rcvd, encr);
    ppsp_impl_get_msgs_frsz(msgs_rcvd, frsz);
    ppsp_impl_get_msgs_plds(msgs_rcvd, plds);
    uint8   cipr_text[16], text_offs;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        ppsp_impl_dec_cipr(cipr_text, plds);
    }
    else
    #endif
    {
        osal_memcpy(cipr_text, plds, frsz);
    }

    if ( 0 == cipr_text[0] )
    {
        LOG("NETW EXIT DONE");
    }
    else if ( 1 == cipr_text[0] )
    {
        LOG("NETW CFGS DONE");
    }

    // rply text
    cipr_text[0] = 0x01;
    text_offs    = 1;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    uint8   text_size, padd_valu;
    uint8   cipr_data[16];

    if ( 0x01 == encr )
    {
        text_size = sizeof(cipr_text);
        ppsp_impl_get_pkcs_7pad(text_size, text_offs, padd_valu);
        osal_memset(cipr_text+text_offs, padd_valu, text_size-text_offs);
        ppsp_impl_enc_text(cipr_text, cipr_data);
    }

    #endif
    uint8* xfer = 0;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_NWRK_RESP, cipr_data, sizeof(cipr_data));
    }
    else
    #endif
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_NWRK_RESP, cipr_text, text_offs);
    }

    if ( 0 != xfer )
    {
        ppsp_impl_set_msgs_encr(xfer, encr);
        ppsp_impl_psh_msgs_xfer(xfer, 20);
    }
}

/*****************************************************************************/
// ota relative
uint8   __ppsp_impl_upda_type;
uint32  __ppsp_impl_upda_vers;
uint32  __ppsp_impl_upda_fwsz;
uint16  __ppsp_impl_upda_crcs;  // crc from upstream
uint8   __ppsp_impl_upda_flag;

uint8   __ppsp_impl_upda_buff[255];
uint16  __ppsp_impl_upda_offs;  // offset of filled posi
uint16  __ppsp_impl_upda_crcd;  // crc calc on local
uint32  __ppsp_impl_upda_wrsz;
uint32  __ppsp_impl_upda_dnsz;
uint32  __ppsp_impl_upda_seqn;  // expect sequ numb of next


static uint8
ppsp_impl_era_prog_data(uint32 addr)
{
    uint8 rslt = PPlus_ERR_FATAL;

    // flash address range check
    if ( PPSP_IMPL_CFGS_PROG_ADDR_BGNS >  addr || PPSP_IMPL_CFGS_PROG_ADDR_ENDS <= addr )
    {
        rslt = PPlus_ERR_INVALID_ADDR;
        goto RSLT_FAIL_ADDR;
    }

    hal_flash_erase_sector(PPSP_IMPL_CFGS_PROG_ADDR_BASE + (addr&0xFFF000));
    rslt = PPlus_SUCCESS;
RSLT_FAIL_ADDR:
    return ( rslt );
}

static uint8
ppsp_impl_pul_prog_data(uint32 addr, uint32* valu)
{
    uint8 rslt = PPlus_ERR_FATAL;

    // flash address range check
    if ( PPSP_IMPL_CFGS_PROG_ADDR_BGNS >  addr || PPSP_IMPL_CFGS_PROG_ADDR_ENDS <= addr )
    {
        rslt = PPlus_ERR_INVALID_ADDR;
        goto RSLT_FAIL_ADDR;
    }

    // 4 bytes aligned
    if ( addr & 0x000003 )
    {
        rslt = PPlus_ERR_DATA_ALIGN;
        goto RSLT_FAIL_ADDR;
    }

    *valu = read_reg(PPSP_IMPL_CFGS_PROG_ADDR_BASE + addr);
    rslt = PPlus_SUCCESS;
RSLT_FAIL_ADDR:
    return ( rslt );
}

static uint8
ppsp_impl_psh_prog_data(uint32 addr, uint32 valu)
{
    uint8 rslt = PPlus_ERR_FATAL;

    // flash address range check
    if ( PPSP_IMPL_CFGS_PROG_ADDR_BGNS >  addr || PPSP_IMPL_CFGS_PROG_ADDR_ENDS <= addr )
    {
        rslt = PPlus_ERR_INVALID_ADDR;
        goto RSLT_FAIL_ADDR;
    }

    // 4 bytes aligned
    if ( addr & 0x000003 )
    {
        rslt = PPlus_ERR_DATA_ALIGN;
        goto RSLT_FAIL_ADDR;
    }

    if ( 0 == hal_flash_write(PPSP_IMPL_CFGS_PROG_ADDR_BASE + addr, (uint8*)&valu, sizeof(valu)) )
    {
        rslt = PPlus_ERR_FATAL;
        goto RSLT_FAIL_PROG;
    }

    rslt = PPlus_SUCCESS;
RSLT_FAIL_PROG:
RSLT_FAIL_ADDR:
    return ( rslt );
}

static uint16
ppsp_impl_cal_crc16_CCITT_FALSE(uint16 crci, uint8* data, uint32 coun)
{
    uint16 wCRCin = crci;
    uint16 wCPoly = 0x1021;

    while (coun--)
    {
        wCRCin ^= (*(data++) << 8);

        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }

    return (wCRCin);
}

static void
ppsp_impl_prc_msgs_vers(uint8* msgs)
{
    uint8   msgn;
    uint8   encr;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs, msgn);
    ppsp_impl_get_msgs_encr(msgs, encr);
    ppsp_impl_get_msgs_frsz(msgs, frsz);
    ppsp_impl_get_msgs_plds(msgs, plds);

    if ( (0x01 == encr && 0x10 != frsz) || (0x00 == encr && 0x01 != frsz) )
    {
        LOG("%s, !! INVALID MESG CONTENT !!");
        return;
    }

    uint8   cipr_text[16], text_offs;   // plain text, text size, offset, padding byte;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        ppsp_impl_dec_cipr(cipr_text, plds);
    }
    else
    #endif
    {
        osal_memcpy(cipr_text, plds, frsz);
    }

    // rply text
    cipr_text[0] = ((0x00==cipr_text[0]) ? 0x00 : 0xFF); // type
    cipr_text[1] = PPSP_IMPL_CFGS_PROG_VERS_REVI; // revision
    cipr_text[2] = PPSP_IMPL_CFGS_PROG_VERS_MINR; // minor
    cipr_text[3] = PPSP_IMPL_CFGS_PROG_VERS_MAJR; // major
    cipr_text[4] = 0x00; // reserved
    text_offs    = 5;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    uint8   text_size, padd_valu;   // plain text, text size, offset, padding byte;
    uint8   cipr_data[16];           // ciper

    if ( 0x01 == encr )
    {
        text_size = sizeof(cipr_text);
        ppsp_impl_get_pkcs_7pad(text_size, text_offs, padd_valu);
        osal_memset(cipr_text+text_offs, padd_valu, text_size-text_offs);
        ppsp_impl_enc_text(cipr_text, cipr_data);
    }

    #endif
    uint8* xfer = 0;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_VERS_RESP, cipr_data, sizeof(cipr_data));
    }
    else
    #endif
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_VERS_RESP, cipr_text, text_offs);
    }

    if ( 0 != xfer )
    {
        ppsp_impl_set_msgs_encr(xfer, encr);
        ppsp_impl_psh_msgs_xfer(xfer, 20);
    }

    // if ( __ppsp_impl_stat_bits_flag & PPSP_IMPL_CFGS_OTAE_STAT_BITS ) {
    //     __ppsp_impl_stat_bits_flag &= ~PPSP_IMPL_CFGS_OTAE_STAT_BITS;
    //     write_reg(0x4000f034, 0); // flag as an OTAs auto reset
    // }
}

static void
ppsp_impl_prc_msgs_upda(uint8* msgs)
{
    uint8   msgn;
    uint8   encr;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs, msgn);
    ppsp_impl_get_msgs_encr(msgs, encr);
    ppsp_impl_get_msgs_frsz(msgs, frsz);
    ppsp_impl_get_msgs_plds(msgs, plds);

    // LOG("msgn = %x\r\n",msgn);
    // LOG("encr = %x\r\n",encr);
    // LOG("frsz = %x\r\n",frsz);
    if ( (0x01 == encr && 0x10 != frsz) || (0x00 == encr && 0x0C != frsz) )
    {
        LOG("%s, !! INVALID MESG CONTENT !! \r\n");
        return;
    }

    // plain text, text size, offset, padding byte
    uint8   cipr_text[16], padd_offs;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        ppsp_impl_dec_cipr(cipr_text, plds);
    }
    else
    #endif
    {
        osal_memcpy(cipr_text, plds, frsz);
    }

    uint8   copy_offs = 0;
    osal_memcpy(&__ppsp_impl_upda_type, cipr_text+copy_offs, sizeof(__ppsp_impl_upda_type)); // fw type
    copy_offs += sizeof(__ppsp_impl_upda_type);
    osal_memcpy(&__ppsp_impl_upda_vers, cipr_text+copy_offs, sizeof(__ppsp_impl_upda_vers)); // fw version
    copy_offs += sizeof(__ppsp_impl_upda_vers);
    osal_memcpy(&__ppsp_impl_upda_fwsz, cipr_text+copy_offs, sizeof(__ppsp_impl_upda_fwsz)); // fw size in byte
    copy_offs += sizeof(__ppsp_impl_upda_fwsz);
    osal_memcpy(&__ppsp_impl_upda_crcs, cipr_text+copy_offs, sizeof(__ppsp_impl_upda_crcs)); // crc16
    copy_offs += sizeof(__ppsp_impl_upda_crcs);
    osal_memcpy(&__ppsp_impl_upda_flag, cipr_text+copy_offs, sizeof(__ppsp_impl_upda_flag)); // flag: 0:full update, 1:incr update
    LOG("%s, type:%x,vers:%x,size:%x,crcs:%x,flag:%x \r\n",
        __func__, __ppsp_impl_upda_type, __ppsp_impl_upda_vers, __ppsp_impl_upda_fwsz, __ppsp_impl_upda_crcs, __ppsp_impl_upda_flag);

    if ( PPSP_IMPL_CFGS_PROG_FLSH_SIZE < __ppsp_impl_upda_fwsz )
    {
        LOG("%s, !! FIRMWARE SIZE EXCEED LIMIT OF %d BYTES !! \r\n", __func__, PPSP_IMPL_CFGS_PROG_FLSH_SIZE);
        return;
    }

    uint32  sctr = (__ppsp_impl_upda_fwsz/PPSP_IMPL_CFGS_PROG_SCTR_SIZE) + (__ppsp_impl_upda_fwsz%PPSP_IMPL_CFGS_PROG_SCTR_SIZE?1:0);

//    LOG("__ppsp_impl_upda_fwsz = %x\r\n",__ppsp_impl_upda_fwsz);
//      LOG("sctr = %x\r\n",sctr);
    for ( uint8 itr0 = 0; itr0 < sctr; itr0 += 1 )
    {
        // PPSP_IMPL_CFGS_PROG_ADDR_BGNS + sctr * 0x1000
//              LOG("itr0 = %x\r\n",itr0);
        ppsp_impl_era_prog_data(PPSP_IMPL_CFGS_PROG_ADDR_BGNS + (itr0<<12));
    }

    LOG("%s, total sctr:%x@size:%x erased!! \r\n", __func__, sctr, __ppsp_impl_upda_fwsz);
    /* resume of last trans point is not supported */
    // reset to initial
    __ppsp_impl_upda_offs = 0;
    __ppsp_impl_upda_crcd = 0xFFFF;
    __ppsp_impl_upda_wrsz = 0;
    __ppsp_impl_upda_dnsz = 0;
    __ppsp_impl_upda_seqn = 0;
    // LOG("%s, pkcs#7=%x\n", __func__, padd);
    cipr_text[0] = 0x01; // 1: upda allow, 0: upda deny
    osal_memcpy(cipr_text+1, &__ppsp_impl_upda_dnsz, sizeof(__ppsp_impl_upda_dnsz));
    // text[1] = 0x00; // last upda posi
    // text[2] = 0x00; // last upda posi
    // text[3] = 0x00; // last upda posi
    // text[4] = 0x00; // last upda posi
    cipr_text[5] = 0x0F; // maxi frame in pack(0x00~0x0f:1~16)
    padd_offs    = 6;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    uint8   text_size, padd_valu;
    uint8   cipr_data[16];

    if ( 0x01 == encr )
    {
        text_size = sizeof(cipr_text);
        ppsp_impl_get_pkcs_7pad(text_size, padd_offs, padd_valu);
        osal_memset(cipr_text+padd_offs, padd_valu, text_size-padd_offs);
        ppsp_impl_enc_text(cipr_text, cipr_data);
    }

    #endif
    uint8* xfer = 0;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_UPDA_RESP, cipr_data, sizeof(cipr_data));
    }
    else
    #endif
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_UPDA_RESP, cipr_text, padd_offs);
    }

    if ( 0 != xfer )
    {
        ppsp_impl_set_msgs_encr(xfer, encr);
        ppsp_impl_psh_msgs_xfer(xfer, 20);
    }
}

static void
ppsp_impl_prc_msgs_pack(uint8* msgs)
{
    uint8   msgn;
    // uint8   encr;
    uint8   alln;
    uint8   seqn;
    uint8   frsz;   // payload size in byte
    uint8*  plds;   // payloads
    ppsp_impl_get_msgs_numb(msgs, msgn);
    // ppsp_impl_get_msgs_encr(msgs, encr);
    ppsp_impl_get_msgs_seqn(msgs, alln, seqn);
    ppsp_impl_get_msgs_frsz(msgs, frsz);
    ppsp_impl_get_msgs_plds(msgs, plds);
//    LOG("__ppsp_impl_update_cnt_1 = %d\r\n",__ppsp_impl_update_cnt);
//   LOG("alln = %x\r\n",alln);
//   LOG("seqn = %x\r\n",seqn);
//   LOG("frsz = %x\r\n",frsz);
    // if ( 0x01 == encr || 0x10 != frsz ) {
    //     LOG("%s, !! INVALID MESG CONTENT !!");
    //     goto RSLT_FAIL_MESG;
    // }
    // TODO: HANDLE EXCEPTION OF FRAME LOSS, IN THAT CASE, LAST FRAME NUMB SHOULD RESP FOR RETRANSMITION
    /*
        copy offset of this received frame, data copy upto the offset would be copied to a buffer.
        when the buffer is full, data in this buffer should write into flash right a way.
        above procedure could repeat untill all data of the frame, being write into flash.
    */

    if ( seqn != __ppsp_impl_upda_seqn )
    {
        LOG("[PPSP][WARN] !! FRAME LOSS !! \r\n");
        LOG("[PPSP][WARN] !! DNSZ=#X%x !! \r\n", __ppsp_impl_upda_dnsz);
        // THIS FRAME AND ALL FOLLOWS WILL BE DROP
        // AND RSPN LAST SUCC UPDA SIZE
        goto RSLT_RESP_MESG;
    }
    else
    {
        __ppsp_impl_upda_seqn += 1;
    }

    {
        // DEPRESSION OF COMPILER WARNING
        uint16 offs = 0;

        while ( offs < frsz )
        {
            uint16 size = MIN(sizeof(__ppsp_impl_upda_buff)-__ppsp_impl_upda_offs, frsz-offs);
            // LOG("base(dest:%x,srcs:%x) \n", __ppsp_impl_upda_buff, plds);
            // LOG("copy(dest:%x,srcs:%x,size:%d) \n", __ppsp_impl_upda_buff+__ppsp_impl_upda_offs, plds+offs, size);
            osal_memcpy(__ppsp_impl_upda_buff+__ppsp_impl_upda_offs, plds+offs, size);
            __ppsp_impl_upda_offs += size;
            offs += size;

//        LOG("size = %x\r\n",size);
//              LOG("offs = %x\r\n",offs);
//              LOG("frsz = %x\r\n",frsz);
//        LOG("__ppsp_impl_upda_offs = %x\r\n",__ppsp_impl_upda_offs);
//        LOG("sizeof(__ppsp_impl_upda_buff) = %x\r\n",sizeof(__ppsp_impl_upda_buff));
            if ( sizeof(__ppsp_impl_upda_buff) <= __ppsp_impl_upda_offs )
            {
                // flsh pack
                // LOG("push for write\r\n");
                #if 0
                uint32  dwrd = (__ppsp_impl_upda_offs/sizeof(uint32)) + (__ppsp_impl_upda_offs%sizeof(uint32)?1:0);
                uint32 valw, valr;

                for ( int itr0 = 0; itr0 < dwrd; itr0 += 1 )
                {
                    // push for write
                    ppsp_impl_psh_prog_data(
                        PPSP_IMPL_CFGS_PROG_ADDR_BGNS + __ppsp_impl_upda_wrsz + (itr0 * sizeof(uint32)),
                        valw = *((uint32*)__ppsp_impl_upda_buff + itr0));
                }

                #else
                hal_flash_write(PPSP_IMPL_CFGS_PROG_ADDR_BGNS + __ppsp_impl_upda_wrsz, __ppsp_impl_upda_buff, sizeof(__ppsp_impl_upda_buff));
                #endif
                __ppsp_impl_upda_crcd  = ppsp_impl_cal_crc16_CCITT_FALSE(__ppsp_impl_upda_crcd, __ppsp_impl_upda_buff, __ppsp_impl_upda_offs);
                __ppsp_impl_upda_wrsz += __ppsp_impl_upda_offs;
                // LOG("__ppsp_impl_upda_wrsz = %x\r\n",__ppsp_impl_upda_wrsz);
                __ppsp_impl_upda_offs  = 0;
            }
        }

        __ppsp_impl_upda_dnsz += frsz;
        LOG("%s, crci:%04x, dnsz:%08x \r\n", __func__, __ppsp_impl_upda_crcd, __ppsp_impl_upda_dnsz);
    }

    if ( alln != seqn )
    {
        goto RSLT_SKIP_RESP;
    }
    else
    {
        __ppsp_impl_upda_seqn =  0;
    }

RSLT_RESP_MESG:
    {
        // DEPRESSION OF COMPILER WARNING
        /* complete of a package, should resp rcvd seqn, rcvd size */
        uint8   cipr_text[16], padd_offs;
        cipr_text[0] = ((alln&0x0F)<<4)|((seqn&0x0F)<<0); //
        osal_memcpy(cipr_text+1, &__ppsp_impl_upda_dnsz, sizeof(__ppsp_impl_upda_dnsz));
        // text[1] = 0x00; // last upda posi
        // text[2] = 0x20; // last upda posi
        // text[3] = 0x00; // last upda posi
        // text[4] = 0x00; // last upda posi
        padd_offs    = 5;
        uint8   auth;
        ppsp_impl_get_auth_rslt(auth);
        //LOG("auth = %x\r\n",auth);
        #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
        uint8   cipr_data[16];
        uint8   text_size, padd_valu; // plain text, text size, offset, padding byte;

        if ( auth )
        {
            text_size = sizeof(cipr_text);
            ppsp_impl_get_pkcs_7pad(text_size, padd_offs, padd_valu);
            osal_memset(cipr_text+padd_offs, padd_valu, text_size-padd_offs);
            ppsp_impl_enc_text(cipr_text, cipr_data);
        }

        #endif
        uint8* xfer = 0;
        #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

        if ( auth )
        {
            xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_PACK_RESP, cipr_data, sizeof(cipr_data));
        }
        else
        #endif
        {
            xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_PACK_RESP, cipr_text, padd_offs);
        }

        if ( 0 != xfer )
        {
            ppsp_impl_set_msgs_encr(xfer, auth);
            ppsp_impl_psh_msgs_xfer(xfer, 20);
        }
    }
RSLT_SKIP_RESP:
// RSLT_FAIL_MESG:
    return;
}

static void
ppsp_impl_prc_msgs_comp(uint8* msgs)
{
    uint8   msgn;
    uint8   encr;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs, msgn);
    ppsp_impl_get_msgs_encr(msgs, encr);
    ppsp_impl_get_msgs_frsz(msgs, frsz);
    ppsp_impl_get_msgs_plds(msgs, plds);

    if ( (0x01 == encr && 0x10 != frsz) || (0x00 == encr && 0x01 != frsz) )
    {
        LOG("[PPSP][ERRN] %s, !! INVALID MESG CONTENT !! \r\n", __func__);
        goto RSLT_FAIL_MESG;
    }

    uint8   cipr_text[16], padd_offs; // plain text, text size, offset, padding byte;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        ppsp_impl_dec_cipr(cipr_text, plds);
    }
    else
    #endif
    {
        osal_memcpy(cipr_text, plds, frsz);
    }

    if ( 0x01 != cipr_text[0] )
    {
        LOG("%s, !! INVALID MESG CONTENT !! \r\n", __func__);
        goto RSLT_FAIL_MESG;
    }

//      LOG("__ppsp_impl_upda_offs = %x\r\n",__ppsp_impl_upda_offs);
    // flsh remains in buffer
    if ( 0 <  __ppsp_impl_upda_offs )
    {
        // LOG("push tail pack\r\n");
        #if 0
        //LOG("__ppsp_impl_upda_fwsz_1= %x\r\n",__ppsp_impl_upda_fwsz);
        // flsh pack
        uint32  dwrd = (__ppsp_impl_upda_offs/sizeof(uint32)) + (__ppsp_impl_upda_offs%sizeof(uint32)?1:0);

        for ( int itr0 = 0; itr0 < dwrd; itr0 += 1 )
        {
            uint32 valw, valr;
            // push for write
            ppsp_impl_psh_prog_data(
                PPSP_IMPL_CFGS_PROG_ADDR_BGNS + __ppsp_impl_upda_wrsz + (itr0 * sizeof(uint32)),
                valw = *((uint32*)__ppsp_impl_upda_buff + itr0));
        }

        #else
        osal_memset(__ppsp_impl_upda_buff + __ppsp_impl_upda_offs, 0x00, sizeof(__ppsp_impl_upda_buff) - __ppsp_impl_upda_offs);
        hal_flash_write(PPSP_IMPL_CFGS_PROG_ADDR_BGNS + __ppsp_impl_upda_wrsz, __ppsp_impl_upda_buff, __ppsp_impl_upda_offs);
        #endif
        // __ppsp_impl_upda_crcd  = ppsp_impl_cal_crc16_CCITT_FALSE(__ppsp_impl_upda_crcd, __ppsp_impl_upda_buff, __ppsp_impl_upda_offs);
        __ppsp_impl_upda_wrsz += __ppsp_impl_upda_offs;
        __ppsp_impl_upda_offs  = 0;
    }

//      LOG("__ppsp_impl_upda_fwsz = %x\r\n",__ppsp_impl_upda_fwsz);
    /* complete of bins, should calc crc16 */
    __ppsp_impl_upda_crcd = 0xffff;

    for ( int itr0 = 0; itr0 < __ppsp_impl_upda_fwsz; itr0++ )
    {
        uint8 valu;
        osal_memcpy(&valu, (uint8_t*)(PPSP_IMPL_CFGS_PROG_ADDR_BASE+PPSP_IMPL_CFGS_PROG_ADDR_BGNS+itr0), 1);
        __ppsp_impl_upda_crcd = ppsp_impl_cal_crc16_CCITT_FALSE(__ppsp_impl_upda_crcd, &valu, 1);
    }

    LOG("%s, crci:%04x, dnsz:%08x, fwsz:%08x \r\n", __func__, __ppsp_impl_upda_crcd, __ppsp_impl_upda_dnsz, __ppsp_impl_upda_fwsz);
    //printf("%s, crci:%04x, dnsz:%08x, fwsz:%08x \n", __func__, __ppsp_impl_upda_crcd, __ppsp_impl_upda_dnsz, __ppsp_impl_upda_fwsz);
    /* complete of bins, should write tags for bldr */
    // uint32 tags;
    // osal_memcpy(&tags, "OTAF", 4);
    ppsp_impl_psh_prog_data(PPSP_IMPL_CFGS_PROG_ADDR_BGNS, 0x4641544f); // tag:"OTAF"
    // printf("%s, tags:%x \r\n", __func__, tags);
//      uint32 val;
//      ppsp_impl_pul_prog_data(PPSP_IMPL_CFGS_PROG_ADDR_BGNS,(uint32 *) &val);
//      LOG("val=%x\n",val);
    // osal_memset(&tags, 0xFF, 4);
    // ppsp_impl_pul_prog_data(PPSP_IMPL_CFGS_PROG_ADDR_BGNS,&tags);
    // printf("%s, tags:%x \r\n", __func__, tags);
    /* complete of bins, should resp crcs chck */
    cipr_text[0] = ((__ppsp_impl_upda_crcd == __ppsp_impl_upda_crcs) ? 0x01 : 0x00); //1:succ, 0:fail
    LOG("%s, complete of bins: crcs comp:%s \r\n", __func__, cipr_text[0]?"SUCC": "FAIL");
//      for(int i=0;i<16;i++){
//            uint32 value;
//              ppsp_impl_pul_prog_data(PPSP_IMPL_CFGS_PROG_ADDR_BGNS+0x10+i*4,(uint32 *) &value);
//              LOG("value_comp=%x\n",value);
//      }
    //printf("%s, complete of bins: crcs comp:%s \n", __func__, cipr_text[0]?"SUCC": "FAIL");
    padd_offs    = 1;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
    uint8   text_size, padd_valu;
    uint8   cipr_data[16];

    if ( 0x01 == encr )
    {
        text_size    = sizeof(cipr_text);
        ppsp_impl_get_pkcs_7pad(text_size, padd_offs, padd_valu);
        osal_memset(cipr_text+padd_offs, padd_valu, text_size-padd_offs);
        ppsp_impl_enc_text(cipr_text, cipr_data);
    }

    #endif
    uint8* xfer;
    #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

    if ( 0x01 == encr )
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_COMP_RESP, cipr_data, sizeof(cipr_data));
    }
    else
    #endif
    {
        xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_COMP_RESP, cipr_text, padd_offs);
    }

    ppsp_impl_set_msgs_encr(xfer, encr);

    if ( 0 != xfer ) ppsp_impl_psh_msgs_xfer(xfer, 20);

    __ppsp_impl_stat_bits_flag |= PPSP_IMPL_CFGS_OTAS_STAT_BITS;
//      LOG("__ppsp_impl_stat_bits_flag=%x\n",__ppsp_impl_stat_bits_flag);
    __ppsp_impl_proc_tout_coun  = 10;   // 1secs
RSLT_FAIL_MESG:
    return;
}

static void
ppsp_impl_prc_msgs_user(uint8* msgs_rcvd)
{
    uint8   msgn;
    uint8   frsz;
    uint8*  plds;
    ppsp_impl_get_msgs_numb(msgs_rcvd, msgn);
    ppsp_impl_get_msgs_frsz(msgs_rcvd, frsz);
    ppsp_impl_get_msgs_plds(msgs_rcvd, plds);

    if ( NULL == plds || 0 >= frsz )
    {
        return;
    }

    /* do whatever user wants */
    //tbrsh_leds_gpio_set_mode(TBRSH_LEDS_ENUM_LED1, plds[0]);
    //tbrsh_leds_gpio_set_mode(TBRSH_LEDS_ENUM_LED2, plds[0]);
    //tbrsh_leds_gpio_set_mode(TBRSH_LEDS_ENUM_LED3, plds[0]);
    //tbrsh_motr_pwms_set_valu(30);
    //tbrsh_motr_pwms_set_mode(plds[0]);
    /* do whatever user wants */
    plds[0] = 0x01; // set resp of true
    uint8* xfer = 0;
    xfer = ppsp_impl_new_msgs_resp(msgn, PPSP_IMPL_CFGS_OPCO_BKEY_RESP, plds, frsz);

    if ( 0 != xfer )
    {
        ppsp_impl_psh_msgs_xfer(xfer, 20);
    }
}

static uint8
ppsp_impl_queu_msgs_hdlr(void)
{
    uint8  rslt = 0;
    uint8* mesg = NULL;
    rslt = core_sque_pop(__ppsp_impl_msgs_queu_rcvd, &mesg);

    /* */
    if ( 1 == rslt )
    {
        uint8 opco;
        ppsp_impl_get_msgs_opco(mesg, opco);
        // LOG("[INF] ppsp_impl_get_msgs_opco: %02x\n\r", opco);

        if ( PPSP_IMPL_CFGS_OPCO_RAND_ISSU == opco )
        {
            #if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_rand(mesg);
            #endif
        }
        else if ( PPSP_IMPL_CFGS_OPCO_VERF_ISSU == opco )
        {
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_verf(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_NWRK_ISSU == opco )
        {
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_nwrk(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_VERS_ISSU == opco )
        {
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_vers(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_UPDA_ISSU == opco )
        {
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_upda(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_PACK_ISSU == opco )
        {
            ppsp_impl_prc_msgs_pack(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_COMP_ISSU == opco )
        {
            //printf("[PANDA][ERR] ppsp_impl_get_msgs_opco: %02x\n\r", opco);
            ppsp_impl_prc_msgs_comp(mesg);
        }
        else if ( PPSP_IMPL_CFGS_OPCO_USER_ISSU == opco )
        {
            ppsp_impl_prc_msgs_user(mesg);
        }

        osal_mem_free(mesg);
    }

    return ( rslt );
}

static void
ppsp_impl_serv_rcvd_hdlr(uint8 para, uint16 coun)
{
    // LOG("[ENT] %s para:%d, coun:%d \r\n", __func__, para, coun);
    if ( PPSP_SERV_CFGS_CHAR_FFD5_INDX == para ||
            PPSP_SERV_CFGS_CHAR_FFD7_INDX == para )
    {
        void* data = NULL;

        if ( NULL == (data = osal_mem_alloc(coun)) ) return;

        ppsp_serv_get_para(para, data, coun);
        ppsp_impl_psh_msgs_rcvd(data, coun);
        ppsp_impl_queu_msgs_hdlr();
        /* if ( NULL != data )  */
        osal_mem_free(data);
    }

    // LOG("[EXI] %s \r\n", __func__);
}

/* alia: ppsp_impl_pop_msgs_xfer */
static void
ppsp_impl_serv_xfer_hdlr(void)
{
    // LOG("[PANDA][ENT] %s \r\n", __func__);
    uint8  rslt = 1;
    uint8* mesg = NULL;
    uint8  opco = 0;
    uint8  frsz = 0;
    rslt = core_sque_pop(__ppsp_impl_msgs_queu_xfer, &mesg);

    if ( 1 == rslt )
    {
        ppsp_impl_get_msgs_opco(mesg, opco);
        ppsp_impl_get_msgs_frsz(mesg, frsz);

        if ( PPSP_IMPL_CFGS_OPCO_VERS_ISSU <= opco && PPSP_IMPL_CFGS_OPCO_COMP_RESP >= opco )
            ppsp_serv_set_para(PPSP_SERV_CFGS_CHAR_FFD8_INDX, frsz + PPSP_IMPL_CFGS_MSGS_HDER_SIZE, (void*)mesg);
        else
            ppsp_serv_set_para(PPSP_SERV_CFGS_CHAR_FFD6_INDX, frsz + PPSP_IMPL_CFGS_MSGS_HDER_SIZE, (void*)mesg);

        osal_mem_free(mesg);
    }
}

void
ppsp_impl_appl_timr_hdlr(void)
{
    // ppsp_impl_queu_msgs_hdlr();
    ppsp_impl_serv_xfer_hdlr();

    //LOG("timer_hdlr\n");
    if ( __ppsp_impl_stat_bits_flag & PPSP_IMPL_CFGS_OTAS_STAT_BITS )
    {
        //LOG("__ppsp_impl_stat_bits_flag\n");
        if ( 0 <  __ppsp_impl_proc_tout_coun )
        {
            __ppsp_impl_proc_tout_coun --;

            if ( 0 == __ppsp_impl_proc_tout_coun )
            {
                LOG("hal_system_soft_reset for bgns of otas \r\n");
                write_reg(0x4000f034, 'O'); // flag as an OTAs auto reset
                hal_system_soft_reset();
            }
        }
    }/*  else

    if ( __ppsp_impl_stat_bits_flag & PPSP_IMPL_CFGS_OTAE_STAT_BITS ) {
        if ( 0 <  __ppsp_impl_proc_tout_coun ) {
            __ppsp_impl_proc_tout_coun --;
            if ( 0 == __ppsp_impl_proc_tout_coun ) {
                printf("%s, hal_system_soft_reset for ends of otas \r\n");
                // write_reg(0x4000f034, 'O'); // flag as an OTAs auto reset
                hal_system_soft_reset();
            }
        }
    } */
}

uint8
ppsp_impl_ini(void)
{
    __ppsp_impl_stat_bits_flag = 0x00;
    uint32  regs = read_reg(0x4000f034);

    if ( 'O' == regs )
    {
        write_reg(0x4000f034, 0); // flag as an OTAs auto reset
        __ppsp_impl_stat_bits_flag |= PPSP_IMPL_CFGS_OTAE_STAT_BITS;
        // __ppsp_impl_proc_tout_coun  = 300;  // 30secs
    }

    __ppsp_impl_msgs_queu_rcvd = core_sque_new(sizeof(uint8*), 32);
    __ppsp_impl_msgs_queu_xfer = core_sque_new(sizeof(uint8*), 32);
    ppsp_serv_add_serv(PPSP_SERV_CFGS_SERV_FEB3_MASK);
    ppsp_serv_reg_appl(&__ppsp_impl_hdlr_serv);
    LOG("\r\n ################################## \r\n");
    LOG("%s, ALIS VERS NUMB: %02d.%02d.%02d \r\n",
        __func__, PPSP_IMPL_CFGS_PROG_VERS_MAJR, PPSP_IMPL_CFGS_PROG_VERS_MINR, PPSP_IMPL_CFGS_PROG_VERS_REVI);
    return ( PPlus_SUCCESS );
}


uint32
ppsp_impl_get_stat(void)
{
    return ( __ppsp_impl_stat_bits_flag );
}

uint8
ppsp_impl_get_pids(uint8* pids)
{
    uint8 rslt = 1;

    if ( NULL != pids )
    {
        // load PID
        for ( uint8 itr0 = 0; itr0 < PPSP_IMPL_CFGS_ALIS_PIDS_COUN; itr0 += 1 )
        {
            hal_flash_read(0x4030+itr0,&pids[PPSP_IMPL_CFGS_ALIS_PIDS_COUN-itr0-1],1);
        }
    }

    return ( rslt );
}

uint8
ppsp_impl_get_macs(uint8* macs)
{
    uint8  rslt = 1;
    uint32 addr = 0x4000;

    if ( NULL != macs )
    {
        // load MAC
        hal_flash_read(addr ++,&macs [3],1);
        hal_flash_read(addr ++,&macs [2],1);
        hal_flash_read(addr ++,&macs [1],1);
        hal_flash_read(addr ++,&macs [0],1);
        hal_flash_read(addr ++,&macs [5],1);
        hal_flash_read(addr ++,&macs [4],1);
//        macs [3] = (uint8)ReadFlash(addr ++);
//        macs [2] = (uint8)ReadFlash(addr ++);
//        macs [1] = (uint8)ReadFlash(addr ++);
//        macs [0] = (uint8)ReadFlash(addr ++);
//        macs [5] = (uint8)ReadFlash(addr ++);
//        macs [4] = (uint8)ReadFlash(addr);
    }

    return ( rslt );
}

uint8
ppsp_impl_get_scrt(uint8* scrt)
{
    uint8 rslt = 1;

    if ( NULL != scrt )
    {
        for ( uint8 itr0 = 0; itr0 < PPSP_IMPL_CFGS_ALIS_SCRT_COUN; itr0 ++ )
        {
            hal_flash_read(0x4010+itr0,&scrt[itr0],1);
            //scrt[itr0] = (uint8_t)ReadFlash(0x4010+itr0);
        }
    }

    return ( rslt );
}

/* calc auth keys */
#if (1 == PPSP_IMPL_CFGS_MSGS_CRYP_ENAB)

uint8
ppsp_impl_cal_keys(const uint8* rand, uint8 rsiz, const uint8* pids, uint8 psiz, const uint8* macs, uint8 msiz, const uint8* scrt, uint8 ssiz)
{
    LOG("[ENT]: %s(rand:#X08%x,rsiz:#D%d,pids#X08%x,psiz:#D%d,macs#X08%x,msiz:#D%d,scrt#X08%x,ssiz:#D%d) \r\n",
        __func__,  rand,       rsiz,     pids,      psiz,     macs,      msiz,     scrt,      ssiz);
    uint8 rslt = 1;
    uint8 temp[128];
    uint8 posi = 0;
    /* rand numb + ',' */
    LOG("\r\n[INF]: RAND DUMP:>>> ");
    my_dump_byte((uint8_t*)rand, rsiz);
    osal_memcpy(temp+posi, rand, rsiz);
    posi += rsiz;
    temp[posi] = ',';
    posi += 1;
    /* pids in hex str + ',' */
    LOG("\r\n[INF]: PIDS DUMP:>>> ");
    my_dump_byte((uint8_t*)pids, rsiz);
    hex2Str(pids, temp+posi, psiz, 1);
    posi += psiz * 2;
    temp[posi] = ',';
    posi += 1;
    /* mac in hex str + ',' */
    LOG("\r\n[INF]: MACS DUMP:>>> ");
    my_dump_byte((uint8_t*)macs, msiz);
    hex2Str(macs, temp+posi, msiz, 1);
    posi += msiz * 2;
    temp[posi] = ',';
    posi += 1;
    /* secret */
    LOG("\r\n[INF]: SCRT DUMP:>>> ");
    my_dump_byte((uint8_t*)scrt, ssiz);
    hex2Str(scrt, temp+posi, ssiz, 0);
    posi += ssiz * 2;
    mbedtls_sha256_context ctxt;
    uint8 sha256sum[32];
    mbedtls_sha256_init(&ctxt);
    rslt = mbedtls_sha256_starts_ret(&ctxt, 0);

    if ( rslt == 0 )
    {
        rslt = mbedtls_sha256_update_ret(&ctxt, temp, posi);
    }

    if ( rslt == 0 )
    {
        rslt = mbedtls_sha256_finish_ret(&ctxt, sha256sum);
    }

    if ( rslt == 0 )    // sucess
    {
        osal_memcpy(__ppsp_impl_auth_keys_data, sha256sum, sizeof(__ppsp_impl_auth_keys_data));
        rslt = 1;
    }

    LOG("\r\n[INF]: KEYS DUMP:>>> ");
    my_dump_byte((uint8_t*)__ppsp_impl_auth_keys_data, sizeof(__ppsp_impl_auth_keys_data));
    #if 0
    gen_aligenie_auth_key((uint8*)rand, rsiz, (uint8*)pids, psiz, (uint8*)macs, msiz, (uint8*)scrt, ssiz);
    cpy_aligenie_auth_key(__ppsp_impl_auth_keys_data);
    #endif
    return ( rslt );
}

uint8*
ppsp_impl_get_keys(void)
{
    return ( __ppsp_impl_auth_keys_data );
}

uint8
ppsp_impl_enc_text(uint8* text, uint8* cipr)
{
    uint8 rslt   = 1;
    uint8 iv[16] = { 0x31, 0x32, 0x33, 0x61, 0x71, 0x77, 0x65, 0x64,
                     0x23, 0x2a, 0x24, 0x21, 0x28, 0x34, 0x6a, 0x75,
                   };
    uint8 data[16];
    uint8 itr0 = 0;
    osal_memcpy(data, text, 16);

    while ( itr0 <  16 )
    {
        data[itr0] ^= iv[itr0];
        itr0       += 1;
    }

    struct tc_aes_key_sched_struct s;

    if ( 0 == tc_aes128_set_encrypt_key(&s, __ppsp_impl_auth_keys_data) )
    {
        LOG("AES128 %s (NIST encr test) failed.\n", __func__);
        rslt = 0;
        goto RSLT_FAIL_ENCR;
    }

    if (tc_aes_encrypt(cipr, data, &s) == 0)
    {
        LOG("AES128 %s (NIST encr test) failed.\n", __func__);
        rslt = 0;
        goto RSLT_FAIL_ENCR;
    }

    rslt = 1;
RSLT_FAIL_ENCR:
    return ( rslt );
}

uint8
ppsp_impl_dec_cipr(uint8* text, uint8* cipr)
{
    uint8 rslt = 1;
    struct tc_aes_key_sched_struct s;

    if (0 == tc_aes128_set_decrypt_key(&s, __ppsp_impl_auth_keys_data) )
    {
        LOG("AES128 %s (NIST decr test) failed.\n", __func__);
        rslt = 0;
        goto RSLT_FAIL_ENCR;
    }

    if (tc_aes_decrypt(text, cipr, &s) == 0)
    {
        LOG("AES128 %s (NIST decr test) failed.\n", __func__);
        rslt = 0;
        goto RSLT_FAIL_ENCR;
    }

    uint8 aesiv[16] = { 0x31, 0x32, 0x33, 0x61, 0x71, 0x77, 0x65, 0x64,
                        0x23, 0x2a, 0x24, 0x21, 0x28, 0x34, 0x6a, 0x75,
                      };
    uint8 itr0 = 0;

    while ( itr0 <  16 )
    {
        text[itr0] ^= aesiv[itr0];
        itr0       += 1;
    }

    rslt = 1;
RSLT_FAIL_ENCR:
    return ( rslt );
}
#endif


/*
    callback of connection stat changes
*/
void
ppsp_impl_ack_conn(uint8 flag)
{
//     /* this likely a disconnection after OTAs, reset is required for complete */
//     if ( __ppsp_impl_stat_bits_flag & PPSP_IMPL_CFGS_OTAS_STAT_BITS ) {
//         write_reg(0x4000f034, 'O'); // flag as an OTAs auto reset
//         hal_system_soft_reset();
//     }
//     // connection loss, shold reset all status
//     __ppsp_impl_stat_bits_flag = 0x00;
}

