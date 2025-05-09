# Two-Pass Assembler in C

This project implements a two-pass assembler for a custom assembly language, developed as part of the "Systems Programming Laboratory in C" course at the Open University.

## 📌 Project Overview

The assembler processes `.as` source files written in a simplified assembly language. It performs two main passes:
1. **First Pass** – Parses the source code, builds a symbol table, and checks for syntax errors.
2. **Second Pass** – Generates machine code and outputs it into `.ob`, `.ent`, and `.ext` files.

The program also includes a pre-processor stage for handling macros.

## 🛠️ Technologies Used

- **Language:** C
- **Operating Systems:** Windows & Linux compatible
- **Build System:** `make`

## 📁 Project Structure

```
Program files/
│
├── assembler.c           # Main entry point
├── first_pass.c          # First-pass logic
├── second_pass.c         # Second-pass logic
├── pre_processor.c       # Macro processing
├── utilitys.c            # Utility functions
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
   ./assembler path_to_file/filename(without_extension)
   ```

   Running this will generate the `.ob`, `.ent`, and `.ext` output files in the same directory.
   **In the directory, the input filename must have the .as extension.**

## 📂 Example Test Files

Located in `Example tests/`, each test includes:
- Assembly source code
- Output files: `.ob`, `.ent`, `.ext`
- Screenshot for reference (in some cases)

## ✍️ Author

Matan Meltz  
📧 matanmeltz@gmail.com  
📍 Kiryat Motzkin  
[LinkedIn](<https://www.linkedin.com/in/matan-meltz-03134521a?lipi=urn%3Ali%3Apage%3Ad_flagship3_profile_view_base_contact_details%3BElsf7pjSQyysrYCL6drx3A%3D%3D>) | [GitHub](<https://github.com/matanmeltz>)

---

