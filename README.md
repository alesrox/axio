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
- [Instruction Set Architecture (ISA)](#instruction-set-architecture-isa)
  - [1. Load and Move](#1-load-and-move)
  - [2. Integer Arithmetic (`int32_t`)](#2-integer-arithmetic-int32_t)
  - [3. Floating-Point Arithmetic (`double`)](#3-floating-point-arithmetic-double)
  - [4. Bitwise Operations](#4-bitwise-operations)
  - [5. Type Casting](#5-type-casting)
  - [6. Comparisons and Logic](#6-comparisons-and-logic)
  - [7. Control Flow (Jumps)](#7-control-flow-jumps)
  - [8. Functions and Sliding Frame](#8-functions-and-sliding-frame)
  - [9. Complex Structures and Heap](#9-complex-structures-and-heap)
- [Tag and Payload Decoding](#tag-and-payload-decoding)
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

## Instruction Set Architecture (ISA)
* **Instruction Format:** Fixed width of 32 bits (4 bytes).

### 1. Load and Move
Operations for transferring data between registers and the **Constant Pool**.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `MOVE` | `ABC` | `R(A) = R(B)` | Copies the value from R(B) to R(A). |
| `LOADK` | `ABx` | `R(A) = Constants[Bx]` | Loads the constant at index Bx into R(A). |
| `LOADBOOL` | `ABC` | `R(A) = (bool)B` | Loads an explicit boolean value into R(A). |
| `LOADNIL` | `ABx` | `R(A)..R(A+Bx) = NIL` | Fills a range of contiguous registers with NIL. |

### 2. Integer Arithmetic (`int32_t`)
Native arithmetic operations without runtime type checking.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `ADD_INT` | `ABC` | `R(A) = R(B) + R(C)` | 32-bit integer addition. |
| `SUB_INT` | `ABC` | `R(A) = R(B) - R(C)` | Integer subtraction. |
| `MUL_INT` | `ABC` | `R(A) = R(B) * R(C)` | Integer multiplication. |
| `DIV_INT` | `ABC` | `R(A) = R(B) / R(C)` | Integer division. |
| `MOD_INT` | `ABC` | `R(A) = R(B) % R(C)` | Integer modulo / remainder. |
| `NEG_INT` | `ABC` | `R(A) = -R(B)` | Unary integer negation. |

### 3. Floating-Point Arithmetic (`double`)
IEEE 754 double-precision operations.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `ADD_FLOAT` | `ABC` | `R(A) = R(B) + R(C)` | 64-bit floating-point addition. |
| `SUB_FLOAT` | `ABC` | `R(A) = R(B) - R(C)` | Floating-point subtraction. |
| `MUL_FLOAT` | `ABC` | `R(A) = R(B) * R(C)` | Floating-point multiplication. |
| `DIV_FLOAT` | `ABC` | `R(A) = R(B) / R(C)` | Floating-point division. |
| `NEG_FLOAT` | `ABC` | `R(A) = -R(B)` | Unary floating-point negation. |

### 4. Bitwise Operations
Exclusive to integer operands.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `BIT_AND` | `ABC` | `R(A) = R(B) & R(C)` | Bitwise AND. |
| `BIT_OR` | `ABC` | `R(A) = R(B) \| R(C)` | Bitwise OR. |
| `BIT_XOR` | `ABC` | `R(A) = R(B) ^ R(C)` | Bitwise XOR. |
| `BIT_NOT` | `ABC` | `R(A) = ~R(B)` | Unary bitwise NOT. |
| `SHL` | `ABC` | `R(A) = R(B) << R(C)` | Bitwise left shift. |
| `SHR` | `ABC` | `R(A) = R(B) >> R(C)` | Bitwise right shift. |

### 5. Type Casting
Required by the compiler to enforce strong typing safety.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `I2F` | `ABC` | `R(A) = (double)R(B)` | Converts an integer to double. |
| `F2I` | `ABC` | `R(A) = (int32_t)R(B)` | Converts a double to integer (truncated). |
| `OBJ2STR` | `ABC` | `R(A) = String(R(B))` | Converts an object/primitive to a string object. |

### 6. Comparisons and Logic
Generates a NaN-boxed boolean value in R(A).

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `EQ_INT` | `ABC` | `R(A) = (R(B) == R(C))` | Integer equality. |
| `LT_INT` | `ABC` | `R(A) = (R(B) < R(C))` | Less than (integers). |
| `LTE_INT` | `ABC` | `R(A) = (R(B) <= R(C))` | Less than or equal to (integers). |
| `EQ_FLOAT` | `ABC` | `R(A) = (R(B) == R(C))` | Floating-point equality. |
| `LT_FLOAT` | `ABC` | `R(A) = (R(B) < R(C))` | Less than (floats). |
| `LTE_FLOAT` | `ABC` | `R(A) = (R(B) <= R(C))` | Less than or equal to (floats). |
| `EQ_OBJ` | `ABC` | `R(A) = (R(B) == R(C))` | Heap pointer reference comparison. |
| `NOT` | `ABC` | `R(A) = !R(B)` | Unary logical NOT. |

### 7. Control Flow (Jumps)
Support for internal control flow and far absolute targets.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `JUMP` | `AsAx` | `PC = PC + sAx` | Unconditional relative jump (±8.3M range). |
| `JUMP_IF_TRUE` | `AsBx` | `if R(A) then PC = PC + sBx` | Conditional relative jump if R(A) is true. |
| `JUMP_IF_FALSE` | `AsBx` | `if !R(A) then PC = PC + sBx` | Conditional relative jump if R(A) is false. |
| `JUMP_ABS` | `ABC` | `PC = R(A)` | Absolute jump to memory address stored in R(A). |

### 8. Functions and Sliding Frame
Zero-copy argument passing by re-orienting current frame base pointer.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `CALL` | `ABC` | `R(A).. = Call(R(B), C)` | Calls function in R(A). B is new frame base, C is argument count. |
| `RET` | `ABC` | `Return R(A)..R(A+B)` | Returns from function, passing back B values starting at R(A). |
| `TAILCALL` | `ABC` | `TailCall(R(A), B, C)` | Tail-call optimization reusing current frame stack. |

### 9. Complex Structures and Heap
Handling dynamic collections and heap-referenced objects.

| Opcode | Format | Operation | Description |
| :--- | :---: | :--- | :--- |
| `NEW_ARRAY` | `ABx` | `R(A) = new Array(Bx)` | Instantiates an array pre-allocated for Bx elements. |
| `NEW_OBJECT` | `ABx` | `R(A) = new Object()` | Instantiates an empty object/hashmap. |
| `GET_FIELD` | `ABC` | `R(A) = R(B)[R(C)]` | Reads property/index R(C) from object R(B). |
| `SET_FIELD` | `ABC` | `R(A)[R(B)] = R(C)` | Writes value R(C) into property/index R(B) of object R(A). |
| `CONCAT` | `ABC` | `R(A) = R(B) \|\| R(C)` | String concatenation in Heap. |

> [!NOTE] TO DO:
> **FSM and Rollback Instructions** 

---

## Tag and Payload Decoding
* **Double**: Any 64-bit value whose exponent is not all 1s (non-NaN).
* **Integer (int32_t)**: Dedicated tag (001) + 32-bit signed payload.
* **Boolean**: Dedicated tag (010) + 1-bit payload (0 for false, 1 for true).
* **Nil**: Dedicated tag (011) + Null payload (0x0).
* **Pointer**: Dedicated tag (100) + 48-bit payload containing the direct heap object memory address.


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

Literal             ::= INT_LITERAL | FLOAT_LITERAL | STRING_LITERAL | BOOLEAN_LITERAL | "nil" ;

BOOLEAN_LITERAL     ::= "True" | "False" ;
INT_LITERAL         ::= DIGIT { DIGIT } ;
FLOAT_LITERAL       ::= DIGIT { DIGIT } "." DIGIT { DIGIT } ;
STRING_LITERAL      ::= '"' { ANY_CHARACTER_EXCEPT_QUOTE } '"' ;

IDENTIFIER          ::= ( LETTER | "_" ) { LETTER | DIGIT | "_" } ;
LETTER              ::= "a".."z" | "A".."Z" ;
DIGIT               ::= "0".."9" ;
```

---

## Development Roadmap

* [ ] **Phase 1: Compiler Core**
    * [x] **Lexer:** Base tokenization for primitives, flow control, and functions.
    * [x] **Parser:** EBNF grammar parsing for core structures and explicit typing.
    * [ ] **Semantic Analysis:** Symbol table, scope resolution, and basic type checking.
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