#ifndef MINIX_INSTRUCTIONS_H
#define MINIX_INSTRUCTIONS_H

// Minix VM Instructions
//

enum {
  MinixIPut,          // Put the following <number> into RArg
  MinixILoadByte,     // Replace a memory address currently in
                      // RArg with a byte value found at this
                      // address
  MinixILoadInt,      // Ditto, but for integer
  MinixIStoreByte,    // Store a byte currently residing in
                      // RArg into memory whose address is on
                      // the top of the stack
  MinixIStoreInt,     // Ditto, but for integer
  MinixIPush,         // Push from RArg to the stack
  MinixIJump,         // Jump to the <address> following this instruction
  MinixIJumpIfZero,   // Jump to <address> if RArg is 0
  MinixIJumpIfNotZero // Jump to <address> if RArg is not
                      // 0
};

#endif
