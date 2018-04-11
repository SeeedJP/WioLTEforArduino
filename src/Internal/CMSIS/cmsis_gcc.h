#pragma once

////////////////////////////////////////////////////////////////////////////////
// Copy from CMSIS in STM32Cube_FW_F4_V1.19.0

#define __ASM            __asm                                      /*!< asm keyword for GNU Compiler */
#define __INLINE         inline                                     /*!< inline keyword for GNU Compiler */
#define __STATIC_INLINE  static inline

////////////////////////////////////////////////////////////////////////////////
// Copy from CMSIS in STM32Cube_FW_F4_V1.19.0

/**
  \brief   No Operation
  \details No Operation does nothing. This instruction can be used for code alignment purposes.
 */
__attribute__((always_inline)) __STATIC_INLINE void __NOP(void)
{
  __ASM volatile ("nop");
}

/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
__attribute__((always_inline)) __STATIC_INLINE void __DSB(void)
{
  __ASM volatile ("dsb 0xF":::"memory");
}
