# ByteStack

![Language](https://img.shields.io/badge/Language-C11-blue.svg) ![Build](https://img.shields.io/badge/Build-Makefile-brightgreen.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg) ![Status](https://img.shields.io/badge/Status-In%20Development-orange.svg)

> An imperative, high-level and strongly typed programming language featuring a bytecode compiler and a virtual machine (VM), similar to Java. This project is a personal study and learning exercise, exploring the design and implementation of a custom programming language.

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
// Variable declarations with explicit typing
int x = 10;
int y = 20;

// Conditional structure
if (x < y) {
    print("ByteStack VM is running!");
}

// Function with parameters and typed return
func add(int a, int b) -> int {
    return a + b;
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
git clone https://github.com/alesrox/ByteStack
cd ByteStack

# Build both compiler and VM
make
```

---

## Usage
ByteStack follows a two-step execution model: compile source code into bytecode, then execute it on the VM.

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

---

## Tag and Payload Decoding
* **Double**: Any 64-bit value whose exponent is not all 1s (non-NaN).
* **Integer (int32_t)**: Dedicated tag (001) + 32-bit signed payload.
* **Boolean**: Dedicated tag (010) + 1-bit payload (0 for false, 1 for true).
* **Nil**: Dedicated tag (011) + Null payload (0x0).
* **Pointer**: Dedicated tag (100) + 48-bit payload containing the direct heap object memory address.

---

## Development Roadmap
* [ ] Compiler: 
    * [x] **Lexer**
    * [x] **Parser**
    * [ ] **Semantic Analysis**
    * [ ] **Bytecode generator**
* [ ] Virtual Machine (VM);
    * [ ] **Value Representation:** Implement data representation.
    * [ ] **32-bit Instruction Decoder:** Bitwise packing/unpacking macros for 32-bit instruction formats.
    * [ ] **Register Execution Engine:** Main dispatch loop
    * [ ] **Sliding Call Frames:** Pointers to contiguous register windows within a global array for function calls without argument copying.
    * [ ] **Foreign Function Interface (FFI / Native C-API):** A mechanism for invoking functions written in C from the VM environment.
    * [ ] **Core I/O:** Printing (print, println), input reading, and basic file handling. 
    * [ ] **String Operations:** String manipulation, concatenation, and optimization via string interning.

## License
This project is open-source and available under the [MIT License](LICENSE).