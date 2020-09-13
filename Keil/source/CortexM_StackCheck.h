#ifndef _CortexM_StackCheck_h_
#define _CortexM_StackCheck_h_
//#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
在arm_startup_nrf51.s里增加下面的语句,把栈起始地址和大小搞出来

Stack_Mem       SPACE   Stack_Size
                EXPORT  Stack_Size
                EXPORT  Stack_Mem
#endif


#define CHECK_RAM_SPACE  0


//初始化栈底,栈底空间作为测试空间.
//需要导出Stack_Mem和Stack_Size.
//EXPORT  Stack_Mem
//EXPORT  Stack_Size
//Stack_Mem是栈底.
//需要吃 8 bytes的空间,分配栈时要注意这个值.

//栈太小时,调用这个函数的时候已经过了爆栈,并且不回到原位置,
//检查是无效的,因为栈指针已经在后面捣乱了.

//检查栈底是否被改写,改写了就1.
//检查正确返回0.

#if ((defined CHECK_RAM_SPACE) && (CHECK_RAM_SPACE == 1))

float CortexM_StackCheck(void);

#endif

#ifdef __cplusplus
}
#endif
#endif

