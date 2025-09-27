#ifndef LIBASM_H
#define LIBASM_H

#include <stdint.h>

/**
 * @file libasm.h
 * @brief Declarations of low-level assembly routines for userland.
 *
 * These externs provide the syscall interface and basic CPU instructions
 * implemented in assembly, used by the UserCodeModule.
 */

 
/**
 * @brief Issue a software interrupt 0x80 system call.
 *
 * Packs the given registers and triggers int 0x80 to enter kernel mode.
 *
 * @param rax System call number.
 * @param rdi First argument (e.g. file descriptor).
 * @param rsi Second argument (e.g. buffer pointer).
 * @param r10 Third argument.
 * @param r8  Fourth argument.
 * @param r9  Fifth argument 
 * @return Value returned in RAX by the kernel syscall handler.
 */
extern uint64_t sys_call(uint64_t rax,
                         uint64_t rdi,
                         uint64_t rsi,
                         uint64_t r10,
                         uint64_t r8,
                         uint64_t r9);

/**
 * @brief Execute the HLT instruction.
 *
 * Stops the CPU until the next interrupt arrives.
 */
extern void _hlt(void);

/**
 * @brief Clear the interrupt flag.
 *
 * Disables maskable hardware interrupts by clearing the IF flag.
 */
extern void _cli(void);

/**
 * @brief Set the interrupt flag.
 *
 * Enables maskable hardware interrupts by setting the IF flag.
 */
extern void _sti(void);

/**
 * @brief Force a divide-by-zero exception.
 *
 * Clears RAX/RDX then does DIV RAX.
 */
extern void _div(void);

/**
 * @brief Force an invalid-opcode exception.
 *
 * Executes UD2.
 */
extern void _ioe(void);

#endif // LIBASM_H
