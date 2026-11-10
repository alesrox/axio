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