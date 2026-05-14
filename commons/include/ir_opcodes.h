#ifndef _IR_OPCODES_H__
#define _IR_OPCODES_H__

//——————————————————————————————————————————————————————————————————————————————

/* IR operation codes. */
enum ir_opc_t {
    IR_OPC_NOP          = 0,
    IR_OPC_ADD          = 1,
    IR_OPC_SUB          = 2,
    IR_OPC_IMUL         = 3,
    IR_OPC_IDIV         = 4,
    IR_OPC_MOV          = 5,
    IR_OPC_PUSH         = 6,
    IR_OPC_POP          = 7,
    IR_OPC_CALL         = 8,
    IR_OPC_RET          = 9,
    IR_OPC_SYSCALL      = 10,
    IR_OPC_JMP          = 11,
    IR_OPC_JE           = 12,
    IR_OPC_JNE          = 13,
    IR_OPC_TEST         = 14,

    IR_OPC_IN           = 15,
    IR_OPC_OUT          = 16,
    
    IR_OPC_LOCAL_LABEL  = 17,
    IR_OPC_GLOBAL_LABEL = 18,
    IR_OPC_FILDL        = 19,
    IR_OPC_FSQRT        = 20,
    IR_OPC_FISTPL       = 21,
    IR_OPC_CQO          = 22,
};

//——————————————————————————————————————————————————————————————————————————————

#endif // _IR_OPCODES_H__
