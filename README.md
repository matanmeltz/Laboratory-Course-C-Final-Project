# Two-Pass Assembler in C

This project implements a two-pass assembler for a custom assembly language, developed as part of the "Systems Programming Laboratory in C" course at the Open University of Israel.

## 📌 Project Overview

The assembler processes `.as` source files written in a simplified assembly language. It performs two main passes:
1. **First Pass** – Parses the source code, builds a symbol table, and checks for syntax errors.
2. **Second Pass** – Generates machine code and outputs it into `.ob`, `.ent`, and `.ext` files.

The program also includes a pre-processor stage for handling macros.

## 🛠️ Technologies Used

- **Language:** C
- **Operating Systems:** Windows & Linux compatible
- **Build System:** `make`

## 📂 Project Structure

```
Program files/
│
├── assembler.c           # Main entry point
├── first_pass.c          # First-pass logic
├── second_pass.c         # Second-pass logic
├── pre_processor.c       # Macro processing
├── utilities.c           # Utility functions
├── errors.c              # Error handling
├── headers.h             # Shared definitions and structs
├── makefile              # Build script
```


## 🔧 How to Build and Run

1. **Navigate to the source folder:**
   ```bash
   cd "The final project/Program files"
   ```

2. **Compile the project:**
   ```bash
   make
   ```

3. **Run the assembler:**
   ```bash
   ./assembler path/to/filename  # Do NOT include the .as extension
   ```

   > ⚠️ **Note:** In the directory, the input file must have the `.as` extension!
   
   Running this will generate the `.ob`, `.ent`, and `.ext` output files in the same directory.
   

## 📂 Example Test Files

Located in `Example tests/`, each test includes:
- Assembly source code
- Output files: `.ob`, `.ent`, `.ext`
- Screenshot for reference (in some cases)

## 🧠 Instruction Set Overview

### 📘 Supported Instructions

The assembler supports 16 machine instructions, each with an opcode and addressing modes:

| Instruction | Opcode | Description            |
|-------------|--------|------------------------|
| mov         | 0      | Copy source to target  |
| cmp         | 1      | Compare source & target|
| add         | 2      | Add source to target   |
| sub         | 3      | Subtract from target   |
| not         | 4      | Bitwise NOT            |
| clr         | 5      | Clear target           |
| lea         | 6      | Load address           |
| inc         | 7      | Increment target       |
| dec         | 8      | Decrement target       |
| jmp         | 9      | Jump to label          |
| bne         | 10     | Branch if not equal    |
| red         | 11     | Read input to target   |
| prn         | 12     | Print operand          |
| jsr         | 13     | Jump to subroutine     |
| rts         | 14     | Return from subroutine |
| stop        | 15     | Stop program           |

### 🧭 Addressing Modes

| Mode Name      | Code | Description                                   |
|----------------|------|-----------------------------------------------|
| Immediate      | 0    | Constant value (e.g., `#5`)                   |
| Direct         | 1    | Symbolic label (e.g., `LABEL`)                |
| Register Direct| 2    | Register name (e.g., `r3`)                    |
| Register Indirect | 3 | Pointer to memory via register (e.g., `*r3`)  |

### 🧾 Assembler Directives

| Directive   | Description                                                |
|-------------|------------------------------------------------------------|
| `.data`     | Defines numeric data in memory. Example: `.data 3, -2, 5`  |
| `.string`   | Defines a null-terminated string. Example: `.string "abc"` |
| `.entry`    | Declares a symbol for the entry table (`.ent` file).       |
| `.extern`   | Declares an external symbol (from another file).           |


## ✍️ Author

Matan Meltz  
📧 matanmeltz@gmail.com  
📍 Kiryat Motzkin  
[LinkedIn](<https://www.linkedin.com/in/matan-meltz-03134521a?lipi=urn%3Ali%3Apage%3Ad_flagship3_profile_view_base_contact_details%3BElsf7pjSQyysrYCL6drx3A%3D%3D>) | [GitHub](<https://github.com/matanmeltz>)

---

