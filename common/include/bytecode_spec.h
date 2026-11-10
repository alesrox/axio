#ifndef BYTECODE_SPEC_H
#define BYTECODE_SPEC_H

#include <stdint.h>

/* ============================================================================
 * 1. BINARY FORMAT METADATA
 * ============================================================================ */

#define VM_MAGIC_NUMBER  0x564D4243  /* ASCII: "VMBC" */
#define VM_VERSION_MAJOR 1
#define VM_VERSION_MINOR 0

/* ============================================================================
 * 2. BINARY HEADER (16 Bytes, unpacked)
 * ============================================================================ */

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;           /* Must match VM_MAGIC_NUMBER */
    uint8_t  version_major;   /* Specification major version */
    uint8_t  version_minor;   /* Specification minor version */
    uint16_t reserved;        /* Reserved for alignment/future flags */
    uint32_t constant_count;  /* Number of entries in Constant Pool */
    uint32_t code_count;      /* Number of 32-bit instructions */
} BinaryHeader;
#pragma pack(pop)

/* ============================================================================
 * 3. INSTRUCTION SET ARCHITECTURE (ISA)
 * ============================================================================ */

typedef enum {
    /* --- 1. Load and Move --- */
    OP_MOVE        = 0x01,  /* R(A) = R(B) */
    OP_LOADK       = 0x02,  /* R(A) = Constants[Bx] */
    OP_LOADBOOL    = 0x03,  /* R(A) = (bool)B */
    OP_LOADNULL     = 0x04,  /* R(A)..R(A+Bx) = NULL */

    /* --- 2. Integer Arithmetic (int32_t) --- */
    OP_ADD_INT     = 0x10,  /* R(A) = R(B) + R(C) */
    OP_SUB_INT     = 0x11,  /* R(A) = R(B) - R(C) */
    OP_MUL_INT     = 0x12,  /* R(A) = R(B) * R(C) */
    OP_DIV_INT     = 0x13,  /* R(A) = R(B) / R(C) */
    OP_MOD_INT     = 0x14,  /* R(A) = R(B) % R(C) */
    OP_NEG_INT     = 0x15,  /* R(A) = -R(B) */

    /* --- 3. Floating-Point Arithmetic (double) --- */
    OP_ADD_FLOAT   = 0x20,  /* R(A) = R(B) + R(C) */
    OP_SUB_FLOAT   = 0x21,  /* R(A) = R(B) - R(C) */
    OP_MUL_FLOAT   = 0x22,  /* R(A) = R(B) * R(C) */
    OP_DIV_FLOAT   = 0x23,  /* R(A) = R(B) / R(C) */
    OP_NEG_FLOAT   = 0x24,  /* R(A) = -R(B) */

    /* --- 4. Bitwise Operations --- */
    OP_BIT_AND     = 0x30,  /* R(A) = R(B) & R(C) */
    OP_BIT_OR      = 0x31,  /* R(A) = R(B) | R(C) */
    OP_BIT_XOR     = 0x32,  /* R(A) = R(B) ^ R(C) */
    OP_BIT_NOT     = 0x33,  /* R(A) = ~R(B) */
    OP_SHL         = 0x34,  /* R(A) = R(B) << R(C) */
    OP_SHR         = 0x35,  /* R(A) = R(B) >> R(C) */

    /* --- 5. Type Casting --- */
    OP_I2F         = 0x40,  /* R(A) = (double)R(B) */
    OP_F2I         = 0x41,  /* R(A) = (int32_t)R(B) */
    OP_OBJ2STR     = 0x42,  /* R(A) = String(R(B)) */

    /* --- 6. Comparisons and Logic --- */
    OP_EQ_INT      = 0x50,  /* R(A) = (R(B) == R(C)) */
    OP_LT_INT      = 0x51,  /* R(A) = (R(B) < R(C)) */
    OP_LTE_INT     = 0x52,  /* R(A) = (R(B) <= R(C)) */
    OP_EQ_FLOAT    = 0x53,  /* R(A) = (R(B) == R(C)) */
    OP_LT_FLOAT    = 0x54,  /* R(A) = (R(B) < R(C)) */
    OP_LTE_FLOAT   = 0x55,  /* R(A) = (R(B) <= R(C)) */
    OP_EQ_OBJ      = 0x56,  /* R(A) = (R(B) == R(C)) */
    OP_NOT         = 0x57,  /* R(A) = !R(B) */

    /* --- 7. Control Flow (Jumps) --- */
    OP_JUMP        = 0x60,  /* PC = PC + sAx */
    OP_JUMP_IF_TRUE= 0x61,  /* if R(A) then PC = PC + sBx */
    OP_JUMP_IF_FALSE=0x62,  /* if !R(A) then PC = PC + sBx */
    OP_JUMP_ABS    = 0x63,  /* PC = R(A) */

    /* --- 8. Functions and Sliding Frame --- */
    OP_CALL        = 0x70,  /* R(A).. = Call(R(B), C) */
    OP_RET         = 0x71,  /* Return R(A)..R(A+B) */
    OP_TAILCALL    = 0x72,  /* TailCall(R(A), B, C) */

    /* --- 9. Complex Structures and Heap --- */
    OP_NEW_ARRAY   = 0x80,  /* R(A) = new Array(Bx) */
    OP_NEW_OBJECT  = 0x81,  /* R(A) = new Object() */
    OP_GET_FIELD   = 0x82,  /* R(A) = R(B)[R(C)] */
    OP_SET_FIELD   = 0x83,  /* R(A)[R(B)] = R(C) */
    OP_CONCAT      = 0x84,  /* R(A) = R(B) || R(C) */

    /* --- 10. Input / Output (I/O) --- */
    OP_PRINT       = 0x90,  /* Print R(A) to stdout based on NaN-boxed type */
    OP_READ_LINE   = 0x91   /* Read line from stdin and store String Object in R(A) */
} OpCode;

/* ============================================================================
 * 4. ENCODING MACROS (Compiler side)
 * ============================================================================ */

/* Format ABC: [ Opcode: 8b | Reg A: 8b | Reg B: 8b | Reg C: 8b ] */
#define PACK_ABC(op, a, b, c) \
    (((uint32_t)(op) << 24) | ((uint32_t)(a) << 16) | ((uint32_t)(b) << 8) | ((uint32_t)(c) & 0xFF))

/* Format ABx: [ Opcode: 8b | Reg A: 8b | Unsigned Immediate Bx: 16b ] */
#define PACK_ABX(op, a, bx) \
    (((uint32_t)(op) << 24) | ((uint32_t)(a) << 16) | ((uint32_t)(bx) & 0xFFFF))

/* Format AsBx: [ Opcode: 8b | Reg A: 8b | Signed Immediate sBx: 16b (biased) ] */
#define PACK_ASBX(op, a, sbx) \
    PACK_ABX(op, a, (uint16_t)((sbx) + 0x7FFF))

/* Format AsAx: [ Opcode: 8b | Signed Immediate sAx: 24b (biased) ] */
#define PACK_ASAX(op, sax) \
    (((uint32_t)(op) << 24) | ((uint32_t)((sax) + 0x7FFFFF) & 0xFFFFFF))

/* ============================================================================
 * 5. DECODING MACROS (VM side)
 * ============================================================================ */

#define GET_OPCODE(inst) ((uint8_t)((inst) >> 24))
#define GET_ARG_A(inst)  ((uint8_t)((inst) >> 16))
#define GET_ARG_B(inst)  ((uint8_t)((inst) >> 8))
#define GET_ARG_C(inst)  ((uint8_t)(inst))

#define GET_ARG_BX(inst) ((uint16_t)((inst) & 0xFFFF))
#define GET_ARG_SBX(inst)((int16_t)GET_ARG_BX(inst) - 0x7FFF)

#define GET_ARG_SAX(inst)((int32_t)((inst) & 0xFFFFFF) - 0x7FFFFF)

#endif /* BYTECODE_SPEC_H */