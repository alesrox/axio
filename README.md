# Axio

![Language](https://img.shields.io/badge/Language-C11-blue.svg) ![Build](https://img.shields.io/badge/Build-Makefile-brightgreen.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg) ![Status](https://img.shields.io/badge/Status-In%20Development-orange.svg)

> **Axio** is an imperative, high-level, and strongly typed programming language featuring a register-based VM that combines **Native Finite State Machines (FSM)** with **Deterministic Replay / Rollback execution**, providing first-class primitives for building reliable, event-driven, and reversible stateful applications.

> This project is a personal study and learning exercise, exploring the design and implementation of a custom programming language.

---

## Table of Contents

- [System Architecture](#system-architecture)
  - [1. Compiler](#1-compiler)
  - [2. Virtual Machine](#2-virtual-machine)
- [Code Example](#code-example)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building from Source](#building-from-source)
- [Usage](#usage)
  - [1. Compile Source Code](#1-compile-source-code)
  - [2. Run on the Virtual Machine](#2-run-on-the-virtual-machine)
- [Axio: Instruction Set Architecture (ISA)](#axio-instruction-set-architecture-isa)
- [Development Roadmap](#development-roadmap)
- [License](#license)

---
## System Architecture

The project ecosystem is divided into two independent components:

```
+------------------+
|   Source Code    |
+------------------+
         |
         v
+------------------+
|     Compiler     |  ---> (Lexer, Parser/AST, Bytecode Emitter)
+------------------+
         |
         v
+------------------+
|  Bytecode (.bin) |
+------------------+
         |
         v
+------------------+
| Virtual Machine  |  ---> (Register Execution Engine & GC)
+------------------+
```
### 1. Compiler
* **Lexer & Parser:** Converts the source code into an Abstract Syntax Tree (AST).
* **Bytecode Generator:** Emits the optimized instruction set for the Virtual Machine.

### 2. Virtual Machine
* **Bytecode Interpreter:** Executes instructions by emulating a register-based architecture with sliding register frames.
* **Memory Management:** Garbage Collector.

---

## Code Example

```c
// Native Finite State Machine definition
state_machine OrderProcessor {
    int order_id;
    float total;

    initial state Received {
        on event_payment_ok -> transition Processing;
        on event_cancel -> transition Cancelled;
    }

    state Processing {
        on_enter {
            // Create a restoration point for current frame registers
            checkpoint "pre_inventory_check";
        }

        on event_out_of_stock {
            // Revert register mutations and restore previous state
            rollback "pre_inventory_check";
        }

        on event_shipped -> transition Shipped;
    }

    state Shipped {}
    state Cancelled {}
}

func main() -> int {
    OrderProcessor order = OrderProcessor(101, 49.99);
    order.emit(event_payment_ok);
    return 0;
}
```

You can see the grammar.ebnf on `docs/grammar.ebnf` or below on this README.

---

## Getting Started
### Prerequisites
* Compiler: `gcc` (supporting C11 or higher)
* Build System: `make`

### Building from Source
```bash
# Clone the repository
git clone https://github.com/alesrox/axio.git
cd axio

# Build both compiler and VM
make
```

---

## Usage
Axio follows a two-step execution model: compile source code into bytecode, then execute it on the VM.

### 1. Compile Source Code
```bash
./bin/compiler main.bs main.bin
```

### 2. Run on the Virtual Machine
```bash
./bin/vm main.bin
```

---

## Testing and Memory Diagnostics

To run the test suite and verify that there are no memory leaks in the VM or Compiler using **Valgrind**:

```bash
# Run tests
make tests

# Check for memory leaks in the Virtual Machine
valgrind --leak-check=full ./bin/vm sample.bin
```
---

## Grammar EBNF

```EBNF
Program             ::= { Statement } ;

Statement           ::= VarDecl | Assignment | FunctionDecl | 
                        StateMachineDecl | CheckpointStmt | RollbackStmt | EmitStmt | 
                        ReturnStmt | IfStmt | ForStmt | WhileStmt | ExprStmt ;

Type                ::= PrimitiveType | CustomType ;
PrimitiveType       ::= "int" | "float" | "bool" | "string" ;
CustomType          ::= IDENTIFIER ;

VarDecl             ::= Type IDENTIFIER [ "=" Expression ] ";" ;

StateMachineDecl    ::= "state_machine" IDENTIFIER "{" { StateFieldDecl } { StateDecl } "}" ;
StateFieldDecl      ::= VarDecl ;
StateDecl           ::= [ "initial" ] "state" IDENTIFIER "{" { StateBody } "}" ;
StateBody           ::= OnEnterBlock | OnUpdateBlock | OnEventBlock ;

OnEnterBlock        ::= "on_enter" Block ;
OnUpdateBlock       ::= "on_update" "(" Parameter ")" Block ;
OnEventBlock        ::= "on" IDENTIFIER [ "->" "transition" IDENTIFIER ] [ Block ] ;

CheckpointStmt      ::= "checkpoint" STRING_LITERAL ";" ;
RollbackStmt        ::= "rollback" STRING_LITERAL ";" ;

FunctionDecl        ::= "func" IDENTIFIER "(" [ ParameterList ] ")" [ "->" Type ] Block ;
ParameterList       ::= Parameter { "," Parameter } ;
Parameter           ::= Type IDENTIFIER ;

Block               ::= "{" { Statement } "}" ;

IfStmt              ::= "if" Expression Block { "else" "if" Expression Block } [ "else" Block ] ;
ForStmt             ::= "for" IDENTIFIER "in" Expression Block ;
WhileStmt           ::= "while" Expression Block ;
ReturnStmt          ::= "return" [ Expression ] ";" ;

Expression          ::= AssignmentExpr ;
AssignmentExpr      ::= [ PrimaryExpr "." ] IDENTIFIER "=" Expression | LogicalOrExpr ;

LogicalOrExpr       ::= LogicalAndExpr { "or" LogicalAndExpr } ;
LogicalAndExpr      ::= BitwiseExpr { "and" BitwiseExpr } ;

BitwiseExpr         ::= EqualityExpr { ( "&" | "|" | "^" | "<<" | ">>" ) EqualityExpr } ;

EqualityExpr        ::= RelationalExpr { ( "==" | "!=" ) RelationalExpr } ;
RelationalExpr      ::= AdditiveExpr { ( "<" | "<=" | ">" | ">=" ) AdditiveExpr } ;

AdditiveExpr        ::= MultiplicativeExpr { ( "+" | "-" ) MultiplicativeExpr } ;
MultiplicativeExpr  ::= UnaryExpr { ( "*" | "/" | "%" ) UnaryExpr } ;

UnaryExpr           ::= ( "-" | "not" | "~" ) UnaryExpr | PrimaryExpr ;

PrimaryExpr         ::= Literal | MethodCallExpr| CallExpr | AccessExpr | IDENTIFIER | "(" Expression ")" ;

MethodCallExpr      ::= PrimaryExpr "." IDENTIFIER "(" [ ArgumentList ] ")" ;
CallExpr            ::= IDENTIFIER "(" [ ArgumentList ] ")" ;
ArgumentList        ::= Expression { "," Expression } ;

AccessExpr          ::= PrimaryExpr "[" Expression "]" ;

Literal             ::= INT_LITERAL | FLOAT_LITERAL | STRING_LITERAL | BOOLEAN_LITERAL | "null" ;

BOOLEAN_LITERAL     ::= "True" | "False" ;
INT_LITERAL         ::= DIGIT { DIGIT } ;
FLOAT_LITERAL       ::= DIGIT { DIGIT } "." DIGIT { DIGIT } ;
STRING_LITERAL      ::= '"' { ANY_CHARACTER_EXCEPT_QUOTE } '"' ;

IDENTIFIER          ::= ( LETTER | "_" ) { LETTER | DIGIT | "_" } ;
LETTER              ::= "a".."z" | "A".."Z" ;
DIGIT               ::= "0".."9" ;
```

---

## Axio: Instruction Set Architecture (ISA)

This document proposes a new ISA for Axio. Each instruction is **32 bits (4 bytes)**. Hexadecimal values show these bytes in the order in which they are read and written in the file; C structures must not be serialized directly because the host's byte order can vary.

### Instruction Formats

| Format | Bit Layout | Use |
|---|---|---|
| `ABC` | `opcode:8 · A:8 · B:8 · C:8` | Registers and small operands |
| `ABx` | `opcode:8 · A:8 · Bx:16` | Register plus immediate or index |
| `AsBx` | `opcode:8 · A:8 · sBx:16` | Register plus signed offset |
| `sAx` | `opcode:8 · sAx:24` | Signed relative jump |

Signed immediates use two's complement. In `AsBx` and `sAx`, the offset is measured from the next instruction. In the tables, `HH LL` represents the two bytes of a 16-bit field; for example, `1000 = 0x03E8`, so its bytes are `03 E8`.

- `R0`–`R255`: registers in the active window.
- `K[n]`: constant number `n` in the constant pool.
- `F[n]`: function number `n` in the function table.
- Binary operations read `B` and `C`, and write to `A`; unary operations read `B` and write to `A`.
- The hexadecimal examples are illustrative: each line is one 4-byte word.

### Load and Move

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `MOVE` | `01 A B 00` (`ABC`) | Copies a register. | `b = a` → `MOVE R1,R0` → `01 01 00 00` |
| `LOAD_I16` | `02 A HH LL` (`ABx`, `int16` immediate) | Loads a small integer. | `x = 1000` → `LOAD_I16 R0,1000` → `02 00 03 E8` |
| `LOAD_CONST` | `03 A HH LL` (`ABx`, `K` index) | Loads a constant from the pool. | `s = "hello"` → `LOAD_CONST R0,K[2]` → `03 00 00 02` |
| `LOAD_BOOL` | `04 A 00 00` (`ABC`, `B=0/1`) | Loads `false` or `true`. | `ok = True` → `LOAD_BOOL R0,1` → `04 00 01 00` |
| `LOAD_NULL` | `05 A 00 00` (`ABC`) | Loads the `null` value. | `x = null` → `LOAD_NULL R0` → `05 00 00 00` |

Integers outside the `int16` range are also loaded with `LOAD_CONST`. The pool can contain 64-bit integers, floating-point values, strings, and other immutable values.

### Integer Arithmetic

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `ADD_I` | `10 A B C` (`ABC`) | Adds integers. | `c = a + b` → `ADD_I R2,R0,R1` → `10 02 00 01` |
| `SUB_I` | `11 A B C` (`ABC`) | Subtracts integers. | `c = a - b` → `SUB_I R2,R0,R1` → `11 02 00 01` |
| `MUL_I` | `12 A B C` (`ABC`) | Multiplies integers. | `c = a * b` → `MUL_I R2,R0,R1` → `12 02 00 01` |
| `DIV_I` | `13 A B C` (`ABC`) | Divides integers. | `c = a / b` → `DIV_I R2,R0,R1` → `13 02 00 01` |
| `MOD_I` | `14 A B C` (`ABC`) | Integer remainder. | `c = a % b` → `MOD_I R2,R0,R1` → `14 02 00 01` |
| `NEG_I` | `15 A B 00` (`ABC`) | Negates an integer. | `b = -a` → `NEG_I R1,R0` → `15 01 00 00` |

### Floating-Point Arithmetic (`float`)

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `ADD_F` | `20 A B C` (`ABC`) | Adds floating-point values. | `c = a + b` → `ADD_F R2,R0,R1` → `20 02 00 01` |
| `SUB_F` | `21 A B C` (`ABC`) | Subtracts floating-point values. | `c = a - b` → `SUB_F R2,R0,R1` → `21 02 00 01` |
| `MUL_F` | `22 A B C` (`ABC`) | Multiplies floating-point values. | `c = a * b` → `MUL_F R2,R0,R1` → `22 02 00 01` |
| `DIV_F` | `23 A B C` (`ABC`) | Divides floating-point values. | `c = a / b` → `DIV_F R2,R0,R1` → `23 02 00 01` |
| `MOD_F` | `24 A B C` (`ABC`) | Floating-point remainder. | `c = a % b` → `MOD_F R2,R0,R1` → `24 02 00 01` |
| `NEG_F` | `25 A B 00` (`ABC`) | Negates a floating-point value. | `b = -a` → `NEG_F R1,R0` → `25 01 00 00` |

`DIV_I`, `MOD_I`, `DIV_F`, and `MOD_F` raise a runtime error if the divisor is zero. Integer division truncates toward zero; the integer remainder has the sign of the dividend. `MOD_F` uses the IEEE 754 remainder operation (quotient rounded to the nearest integer, ties to even). Integer overflow wraps at 64 bits.

### Comparisons

All comparisons write `0` or `1` (a boolean) to `A`. String comparisons are limited to content equality; lexicographic ordering is not part of this base ISA.

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `EQ_I` | `40 A B C` (`ABC`) | Integers are equal. | `ok = a == b` → `EQ_I R2,R0,R1` → `40 02 00 01` |
| `NE_I` | `41 A B C` (`ABC`) | Integers are not equal. | `ok = a != b` → `NE_I R2,R0,R1` → `41 02 00 01` |
| `LT_I` | `42 A B C` (`ABC`) | Integer less than. | `ok = a < b` → `LT_I R2,R0,R1` → `42 02 00 01` |
| `LE_I` | `43 A B C` (`ABC`) | Integer less than or equal to. | `ok = a <= b` → `LE_I R2,R0,R1` → `43 02 00 01` |
| `GT_I` | `44 A B C` (`ABC`) | Integer greater than. | `ok = a > b` → `GT_I R2,R0,R1` → `44 02 00 01` |
| `GE_I` | `45 A B C` (`ABC`) | Integer greater than or equal to. | `ok = a >= b` → `GE_I R2,R0,R1` → `45 02 00 01` |
| `EQ_F` | `46 A B C` (`ABC`) | Floating-point values are equal. | `ok = a == b` → `EQ_F R2,R0,R1` → `46 02 00 01` |
| `NE_F` | `47 A B C` (`ABC`) | Floating-point values are not equal. | `ok = a != b` → `NE_F R2,R0,R1` → `47 02 00 01` |
| `LT_F` | `48 A B C` (`ABC`) | Floating-point less than. | `ok = a < b` → `LT_F R2,R0,R1` → `48 02 00 01` |
| `LE_F` | `49 A B C` (`ABC`) | Floating-point less than or equal to. | `ok = a <= b` → `LE_F R2,R0,R1` → `49 02 00 01` |
| `GT_F` | `4A A B C` (`ABC`) | Floating-point greater than. | `ok = a > b` → `GT_F R2,R0,R1` → `4A 02 00 01` |
| `GE_F` | `4B A B C` (`ABC`) | Floating-point greater than or equal to. | `ok = a >= b` → `GE_F R2,R0,R1` → `4B 02 00 01` |
| `EQ_B` | `4C A B C` (`ABC`) | Booleans are equal. | `same = a == b` → `EQ_B R2,R0,R1` → `4C 02 00 01` |
| `NE_B` | `4D A B C` (`ABC`) | Booleans are not equal. | `diff = a != b` → `NE_B R2,R0,R1` → `4D 02 00 01` |
| `EQ_NULL` | `50 A B C` (`ABC`) | Checks equality with `null`. | `empty = a == null` → `EQ_NULL R1,R0` → `50 01 00 00` |
| `NE_NULL` | `51 A B C` (`ABC`) | Checks inequality with `null`. | `present = a != null` → `NE_NULL R1,R0` → `51 01 00 00` |

### Booleans and Bitwise Operations

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `NOT_B` | `60 A B 00` (`ABC`) | Negates a boolean. | `b = not a` → `NOT_B R1,R0` → `60 01 00 00` |
| `AND_B` | `61 A B C` (`ABC`) | Boolean AND. | `c = a and b` → `AND_B R2,R0,R1` → `61 02 00 01` |
| `OR_B` | `62 A B C` (`ABC`) | Boolean OR. | `c = a or b` → `OR_B R2,R0,R1` → `62 02 00 01` |
| `AND_I` | `30 A B C` (`ABC`) | Bitwise AND. | `c = a & b` → `AND_I R2,R0,R1` → `30 02 00 01` |
| `OR_I` | `31 A B C` (`ABC`) | Bitwise OR. | `c = a \| b` → `OR_I R2,R0,R1` → `31 02 00 01` |
| `XOR_I` | `32 A B C` (`ABC`) | Bitwise XOR. | `c = a ^ b` → `XOR_I R2,R0,R1` → `32 02 00 01` |
| `NOT_I` | `33 A B 00` (`ABC`) | Inverts the bits. | `b = ~a` → `NOT_I R1,R0` → `33 01 00 00` |
| `SHL` | `34 A B C` (`ABC`) | Shifts left. | `c = a << b` → `SHL R2,R0,R1` → `34 02 00 01` |
| `SHR` | `35 A B C` (`ABC`) | Shifts right. | `c = a >> b` → `SHR R2,R0,R1` → `35 02 00 01` |

`SHR` is defined as an arithmetic right shift. The shift count is reduced to its 6 least significant bits (`count & 63`).

### Jumps and Termination

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `JUMP` | `70 ss ss ss` (`sAx`) | Unconditional relative jump. | `goto L` → `JUMP +2` → `70 00 00 02` |
| `JUMP_IF_TRUE` | `71 A ss ss` (`AsBx`) | Jumps if the register is true. | `if (ok) goto L` → `JUMP_IF_TRUE R0,+3` → `71 00 00 03` |
| `JUMP_IF_FALSE` | `72 A ss ss` (`AsBx`) | Jumps if the register is false. | `if (not ok) goto L` → `JUMP_IF_FALSE R0,-2` → `72 00 FF FE` |
| `HALT` | `73 00 00 00` (`ABC`) | Terminates the program. | end of program → `HALT` → `73 00 00 00` |

Jumps use an offset from the `PC` following the jump instruction. The compiler can fill in the offset later using backpatching.

### Functions and Register Windows

Each function-table entry contains at least the start address, number of local registers, parameter count, and return type. Each call creates a window whose `R0` corresponds to the caller's first argument register. On return, the VM restores the previous window.

| Instruction | Hex Encoding | Description | Axio Example → Bytecode → Bytes |
|---|---|---|---|
| `CALL` | `80 A B C` (`ABC`) | Calls function `F[B]`; `A` is the argument/result base and `C` is the argument count. | `r = sum(x,y)` → `CALL R4,F[1],2` → `80 04 01 02` |
| `RETURN` | `81 A 00 00` (`ABC`) | Returns `R[A]` to the caller. | `return result` → `RETURN R2` → `81 02 00 00` |
| `RETURN_VOID` | `82 00 00 00` (`ABC`) | Returns without a value. | `return;` → `RETURN_VOID` → `82 00 00 00` |

`CALL` convention: before the call, arguments are contiguous from `R[A]` through `R[A+C-1]`. The called window starts at the caller's absolute register `A`, so its parameters are `R0..R(C-1)`. If the function returns a value, it is written to the caller's `R[A]`; the first argument is overwritten after returning. `B` is an 8-bit function-table index, so v1 supports up to 256 functions. `C` supports up to 255 arguments.

Complete example:

```text
func sum(int x, int y) -> int { return x + y; }
int r = sum(10, 20);
```

```text
Caller:                         Callee (its new window):
02 04 00 0A  LOAD_I16 R4,10     10 02 00 01  ADD_I R2,R0,R1
02 05 00 14  LOAD_I16 R5,20     81 02 00 00  RETURN R2
80 04 01 02  CALL R4,F[1],2
                                On return, the result is in caller R4.
```

There is no need for `PUSH` and `POP` register opcodes: `CALL` saves the caller's minimal state (window, return `PC`, and result destination) on the VM call stack; `RETURN` restores it. An explicit stack instruction would only be needed if Axio exposed a data stack to programs.

### File Format

There is no universal file format that a register-based VM must inherit. Axio defines its own versioned container. Multi-byte container integers are serialized byte by byte in **little-endian** order; a C structure is never written directly. The code stream stores each instruction as four bytes in opcode, A, B, C order (or immediate bytes).

#### Fixed Header (16 Bytes)

| Offset | Size | Field | Rule |
|---:|---:|---|---|
| 0 | 4 | `magic` | ASCII `AXBC`: `41 58 42 43` |
| 4 | 2 | `version_major` | `u16`, little-endian |
| 6 | 2 | `version_minor` | `u16`, little-endian |
| 8 | 4 | `flags` | `u32`; unknown bits must be zero in v1 |
| 12 | 2 | `section_count` | `u16` |
| 14 | 2 | reserved | Zero in v1 |

Each section follows the header as a block: an 8-byte header says what the section is and how many bytes it occupies, followed by its data. It is like a labeled box with a size, allowing the VM to skip an optional section it does not recognize and find the next one.

| Type | Contents |
|---:|---|
| `1` | Constant pool |
| `2` | Function table |
| `3` | Native function imports |
| `4` | Executable code |

Each section header contains `type:u16, flags:u16, payload_length:u32` (8 bytes, little-endian). `payload_length` is the exact number of data bytes following that header. In `flags`, bit 0 indicates `OPTIONAL`; all other bits must be zero in v1. The four sections in the table are required and appear exactly once, in that order. The file ends after the last section's data.

#### Section Contents

- **Constants (`1`):** `count:u32`, followed by entries `tag:u8, length:u32, data[length]`. Tags: `1=int64` (8-byte two's complement, LE), `2=float64` (8-byte IEEE 754 binary64, LE), `3=string` (UTF-8 without a terminator), `4=bool` (one byte `00`/`01`), `5=null` (zero length).
- **Functions (`2`):** `count:u16`, followed by fixed 16-byte entries: `name_constant:u16`, `start_pc:u32`, `code_length:u32` (instructions), `register_count:u16`, `parameter_count:u8`, `return_type:u8`, `flags:u16`. `start_pc` is counted from the first instruction in the code payload (after its `instruction_count`). Functions are indexed from zero; `CALL.B` limits v1 to 256 functions.
- **Code (`4`):** `instruction_count:u32` and exactly `instruction_count × 4` bytes. `start_pc` and jump offsets are measured in instructions from the beginning of the code.

---

## Development Roadmap

* [ ] **Phase 1: Compiler Core**
    * [x] **Lexer:** Base tokenization for primitives, flow control, and functions.
    * [x] **Parser:** EBNF grammar parsing for core structures and explicit typing.
    * [x] **Semantic Analysis:** Symbol table, scope resolution, and basic type checking.
    * [ ] **Bytecode Generator:** Core 32-bit opcode generation (`MOVE`, `ADD`, `JUMP`, `CALL`).

* [ ] **Phase 2: Virtual Machine Core**
    * [ ] **Value Representation:** 64-bit NaN-boxing memory layout.
    * [ ] **32-bit Instruction Decoder:** Bitwise extraction macros ($ABC$, $ABx$, $sAx$).
    * [ ] **Register Engine & Sliding Frames:** Main dispatch loop and zero-copy call frames.
    * [ ] **Garbage Collector:** Mark-and-Sweep collector for heap objects.

* [ ] **Phase 3: Native State Machines (FSM)**
    * [ ] **Compiler Extensions:** AST nodes, parser rules, and semantic validation for `state_machine` and `on` events.
    * [ ] **Bytecode Integration:** Generator support for `OP_TRANSITION` and `OP_EMIT`.
    * [ ] **VM FSM Engine:** State register management and event dispatch system.

* [ ] **Phase 4: Time-Travel & Reversible Execution**
    * [ ] **Compiler Extensions:** Syntax and AST support for `checkpoint` and `rollback` statements.
    * [ ] **VM Mutation Buffer:** Circular Ring Buffer to capture active register snapshots.
    * [ ] **Rollback Execution:** Low-overhead state restoration without heap reallocations.

* [ ] **Phase 5: Standard Library & Interop**
    * [ ] **Core I/O & Strings:** Basic print, file handling, and string interning.
    * [ ] **Foreign Function Interface (FFI):** C-API bindings to embed the VM.

## License
This project is open-source and available under the [MIT License](LICENSE).