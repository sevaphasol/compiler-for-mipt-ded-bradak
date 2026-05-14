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
    IR_OPC_LOCAL_LABEL  = 15,
    IR_OPC_GLOBAL_LABEL = 16,
    IR_OPC_FILDL        = 17,
    IR_OPC_FSQRT        = 18,
    IR_OPC_FISTPL       = 19,
    IR_OPC_CQO          = 20,
};

//——————————————————————————————————————————————————————————————————————————————

#endif // _IR_OPCODES_H__
