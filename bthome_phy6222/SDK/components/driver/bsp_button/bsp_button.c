/*************
 bsp_button.c
 SDK_LICENSE
***************/

#include "error.h"
#include "bsp_button.h"
#include "log.h"

#define KEY_BUF_INDEX(index)    (index / 32)
#define KEY_BUF_MOD(index)      (index % 32)
#define KEY_COMBINE_LEN         (sizeof(BTN_COMBINE_T) << 3)

static BTN_T*                   g_btn_ptr = NULL;
static uint8_t                  g_btn_num;

static uint32_t*                g_combing_ptr = NULL;
static uint8_t                  g_combing_start;

static KEY_FIFO_T               s_Key;
static void                     bsp_DetectBtn(uint8_t index);
static uint32_t                 g_btn_value[(BTN_NUMBER-1)/32+1];

static uint8_t bsp_btn_map(uint32_t index)
{
    switch(index)
    {
    case BIT(0):
        return 0;

    case BIT(1):
        return 1;

    case BIT(2):
        return 2;

    case BIT(3):
        return 3;

    case BIT(4):
        return 4;

    case BIT(5):
        return 5;

    case BIT(6):
        return 6;

    case BIT(7):
        return 7;

    case BIT(8):
        return 8;

    case BIT(9):
        return 9;

    case BIT(10):
        return 10;

    case BIT(11):
        return 11;

    case BIT(12):
        return 12;

    case BIT(13):
        return 13;

    case BIT(14):
        return 14;

    case BIT(15):
        return 15;

    case BIT(16):
        return 16;

    case BIT(17):
        return 17;

    case BIT(18):
        return 18;

    case BIT(19):
        return 19;

    case BIT(20):
        return 20;

    case BIT(21):
        return 21;

    case BIT(22):
        return 22;

    case BIT(23):
        return 23;

    case BIT(24):
        return 24;

    case BIT(25):
        return 25;

    case BIT(26):
        return 26;

    case BIT(27):
        return 27;

    case BIT(28):
        return 28;

    case BIT(29):
        return 29;

    case BIT(30):
        return 30;

    case BIT(31):
        return 31;
    }

    return 0xFF;
}

static bool bsp_is_key_press(uint8_t index)
{
    uint32_t ComKeyFlag = (g_btn_ptr + index)->KeyConfig & BSP_BTN_CM_CFG;
    uint32_t combine_temp;

    if(ComKeyFlag == 0)
    {
        if(g_btn_value[KEY_BUF_INDEX(index)] & BIT(KEY_BUF_MOD(index)))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        combine_temp = g_combing_ptr[(index - g_combing_start)];

        for(int i = 0; i < KEY_COMBINE_LEN; i++)
        {
            if(combine_temp & BIT(i))
            {
                index = bsp_btn_map(BIT(i));

                if((g_btn_value[KEY_BUF_INDEX(index)] & BIT(KEY_BUF_MOD(index))) == 0x00)
                {
                    return FALSE;
                }
            }
        }

        return TRUE;
    }
}

bool bsp_set_key_value_by_row_col(uint8_t cols_num,uint8_t row,uint8_t col,bool value)
{
    uint8_t m0;
    uint32_t temp,combine_temp;
    uint32_t index = row * cols_num + col;

    //LOG("----------->(%d) %d %d\n",__LINE__,index,value);
    if(value)
    {
        g_btn_value[KEY_BUF_INDEX(index)] |= BIT(KEY_BUF_MOD(index));
    }
    else
    {
        g_btn_value[KEY_BUF_INDEX(index)] &= ~BIT(KEY_BUF_MOD(index));
    }

    if((g_btn_ptr + index)->KeyConfig & BSP_BTN_CM_CFG)
    {
        combine_temp = g_combing_ptr[(index - g_combing_start)];

        for(int i=0; i<KEY_COMBINE_LEN; i++)
        {
            temp = (combine_temp & BIT(i));

            if(temp)
            {
                m0 = bsp_btn_map(temp);

                if((g_btn_value[KEY_BUF_INDEX(m0)] & BIT(KEY_BUF_MOD(m0))) == 0)
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

void bsp_set_key_value_by_index(uint8_t index,bool value)
{
    //LOG("----------->(%d) %d %d\n",__LINE__,index,value);
    if(value)
    {
        g_btn_value[KEY_BUF_INDEX(index)] |= BIT(KEY_BUF_MOD(index));
    }
    else
    {
        g_btn_value[KEY_BUF_INDEX(index)] &= ~BIT(KEY_BUF_MOD(index));
    }
}

bool bsp_InitBtn(BTN_T* sum_btn_array,uint8_t sum_btn_num,uint8_t combine_btn_start,BTN_COMBINE_T* combine_btn_array)
{
    int i;
    s_Key.Read = 0;
    s_Key.Write = 0;

    if( (sum_btn_array == NULL) || (sum_btn_num == 0) || (sum_btn_num > BTN_NUMBER))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    g_btn_ptr = sum_btn_array;
    g_btn_num = sum_btn_num;

    for(i=0; i<sum_btn_num; i++)
    {
        g_btn_ptr[i].State = 0;
        g_btn_ptr[i].Count = 0;
        g_btn_ptr[i].FilterTime = BTN_FILTER_TICK_COUNT;
        #ifdef BSP_BTN_LONG_PRESS_ENABLE
        g_btn_ptr[i].LongCount = 0;

        if((g_btn_ptr[i].KeyConfig & BSP_BTN_LPS_CFG) || (g_btn_ptr[i].KeyConfig & BSP_BTN_LPK_CFG))
        {
            g_btn_ptr[i].LongTime = BTN_LONG_PRESS_START_TICK_COUNT;
            g_btn_ptr[i].RepeatSpeed = BTN_LONG_PRESS_KEEP_TICK_COUNT;
            g_btn_ptr[i].RepeatCount = 0;
        }
        else
        {
            g_btn_ptr[i].LongTime = 0;
            g_btn_ptr[i].RepeatSpeed = 0;
            g_btn_ptr[i].RepeatCount = 0;
        }

        #endif
    }

    for(i=0; i<sizeof(g_btn_value)/sizeof(g_btn_value[0]); i++)
    {
        g_btn_value[i] = 0;
    }

    if((combine_btn_array != NULL) && combine_btn_start <= (sum_btn_num - 1))
    {
        g_combing_start = combine_btn_start;
        g_combing_ptr = combine_btn_array;

        for(int i = combine_btn_start; i < sum_btn_num; i++)
        {
            g_btn_ptr[i].KeyConfig |= BSP_BTN_CM_CFG;
        }
    }

    return PPlus_SUCCESS;
}

static void bsp_PutKey(uint8_t _KeyCode)
{
    s_Key.Buf[s_Key.Write] = _KeyCode;

    if (++s_Key.Write  >= KEY_FIFO_SIZE)
    {
        s_Key.Write = 0;
    }
}

uint8_t bsp_GetKey(void)
{
    uint8_t ret;

    if (s_Key.Read == s_Key.Write)
    {
        return BTN_NONE;
    }
    else
    {
        ret = s_Key.Buf[s_Key.Read];

        if (++s_Key.Read >= KEY_FIFO_SIZE)
        {
            s_Key.Read = 0;
        }

        return ret;
    }
}

static void bsp_DetectBtn(uint8_t index)
{
    BTN_T* _pBtn = (g_btn_ptr + index);

    if (bsp_is_key_press(index))
    {
        if (_pBtn->Count < _pBtn->FilterTime)
        {
            _pBtn->Count = _pBtn->FilterTime;
        }
        else if(_pBtn->Count < 2 * _pBtn->FilterTime)
        {
            _pBtn->Count++;
        }
        else
        {
            if (_pBtn->State == 0)
            {
                _pBtn->State = 1;

                if (_pBtn->KeyConfig & BSP_BTN_PD_CFG)
                {
                    bsp_PutKey(BSP_BTN_PD_BASE + index);
                }
            }

            #ifdef BSP_BTN_LONG_PRESS_ENABLE

            if (_pBtn->LongTime > 0)
            {
                if (_pBtn->LongCount < _pBtn->LongTime)
                {
                    if (++_pBtn->LongCount == _pBtn->LongTime)
                    {
                        if (_pBtn->KeyConfig & BSP_BTN_LPS_CFG)
                        {
                            bsp_PutKey(BSP_BTN_LPS_BASE + index);
                        }
                    }
                }
                else
                {
                    if (_pBtn->RepeatSpeed > 0)
                    {
                        if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
                        {
                            _pBtn->RepeatCount = 0;

                            if (_pBtn->KeyConfig & BSP_BTN_LPK_CFG)
                            {
                                bsp_PutKey(BSP_BTN_LPK_BASE + index);
                            }
                        }
                    }
                }
            }

            #endif
        }
    }
    else
    {
        if(_pBtn->Count > _pBtn->FilterTime)
        {
            _pBtn->Count = _pBtn->FilterTime;
        }
        else if(_pBtn->Count != 0)
        {
            _pBtn->Count--;
        }
        else
        {
            if (_pBtn->State == 1)
            {
                _pBtn->State = 0;

                if (_pBtn->KeyConfig & BSP_BTN_UP_CFG )
                {
                    bsp_PutKey(BSP_BTN_UP_BASE + index);
                }
            }
        }

        #ifdef BSP_BTN_LONG_PRESS_ENABLE
        _pBtn->LongCount = 0;
        _pBtn->RepeatCount = 0;
        #endif
    }
}

uint8_t bsp_KeyPro(void)
{
    uint8_t ucKeyCode;

    for(int i = 0; i < g_btn_num; i++)
    {
        bsp_DetectBtn(i);
    }

    ucKeyCode = bsp_GetKey();
    return ucKeyCode;
}

bool bsp_KeyEmpty(void)
{
    for(int i = 0; i < g_btn_num; i++)
    {
        BTN_T* _pBtn = (g_btn_ptr + i);
        #ifdef BSP_BTN_LONG_PRESS_ENABLE

        if ((_pBtn->State == 1) || (_pBtn->Count != 0) || (_pBtn->LongCount != 0) ||(_pBtn->RepeatCount != 0))
        #else
        if ((_pBtn->State == 1) || (_pBtn->Count != 0) )
        #endif
        {
            return FALSE;
        }
    }

    if (s_Key.Read == s_Key.Write)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

