#include "CortexM_StackCheck.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#if ((defined CHECK_RAM_SPACE) && (CHECK_RAM_SPACE == 1))

extern void Stack_Size(void);
extern void Stack_Mem(void);

#define CheckKey (0x89abcdef)

uint32_t TempStartAddr = 0;
uint32_t TempStackSize = 0;
float    TempStackUsed = 0;

//*****************************************************************************
//
//! @brief  return the status of stack
//! @return 1:error. 0:normal.
//
//*****************************************************************************
float CortexM_StackCheck(void)
{
    static int IsFirstCheck = 1;
    uint32_t i;

    TempStartAddr = ((uint32_t)Stack_Mem);
    TempStackSize = ((uint32_t)Stack_Size);

    //第一次执行此函数.
    //检查此时此刻是否栈溢出,并在栈底设定检查标志.
    if (0 != IsFirstCheck)
    {
        //有足够的空间用于填入检查用的数据.
        for (i = 0; i < TempStackSize / 4; i += 4)
        {
            //填入数据
            *((uint32_t*)Stack_Mem + i) = CheckKey;
        }

        //标记已经填入了数据,以后就不来这里玩了.
        IsFirstCheck = 0;

        //OK
        return 0;
    }
    else //前面已经埋下了好东西,看看还在不在...
    {
        for (i = 0; i < TempStackSize / 4; i += 4)
        {
            if (CheckKey != *((uint32_t*)Stack_Mem + i))
                break;
        }
    }

    TempStackUsed = (TempStackSize - (i * 4)) / 1024.0f;

    return TempStackUsed;
}

#endif
