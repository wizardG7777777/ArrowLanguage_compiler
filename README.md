# ArrowLanguage Compiler

## Overview

ArrowLanguage is a simple, Python-like programming language that supports only floating-point variables. The `runml.c` compiler translates Arrow language source code into C and automatically compiles and executes it.

## Language Syntax

ArrowLanguage syntax is very similar to Python, with a few key differences:

### Variable Assignment
- **Python**: `x = 1`
- **Arrow**: `x <- 1`

Variables do not need to be declared before use, and all variables are floating-point numbers.

### Basic Operations
```arrow
x <- 1          # Assignment
x <- x + 1      # Arithmetic operations
z <- x + y      # Using multiple variables
x <- x * x      # Self-assignment
print(x+1)      # Print expressions
```

### Function Definition
**Python:**
```python
def add(a, b):
    return a + b
```

**Arrow:**
```arrow
function add(a, b)
    return a + b
```

### Comments
Comments start with `#` and continue to the end of the line.

## How runml.c Works

The `runml.c` compiler is a sophisticated translator that converts Arrow language into C code through several key modules:

### 1. **Main Architecture**
The compiler consists of three main translation modules:
- **Statement Translation**: Handles variable assignments and expressions
- **Print Translation**: Converts print statements to appropriate C printf calls
- **Function Translation**: Processes function definitions and calls

### 2. **Translation Process**

#### **Input Processing**
1. Reads the Arrow source file line by line
2. Stores all lines in memory for processing
3. Handles comments by removing everything after `#`
4. Validates syntax including bracket integrity and illegal symbols

#### **Variable Management**
- **Global Variables**: Variables defined in main scope are treated as global
- **Local Variables**: Function parameters and local variables are scoped appropriately
- **Automatic Declaration**: Variables are automatically declared as `float` when first used
- **Initialization**: All variables are initialized to `0.0`

#### **Statement Translation**
- Converts `<-` assignment operator to `=`
- Identifies and validates variable names (lowercase alphabetic only)
- Handles arithmetic expressions with proper operator precedence
- Manages variable scope (global vs local)

#### **Print Translation**
The compiler implements intelligent printing that automatically detects integer vs float values:
```c
if ((expression)-(int)(expression)==0)
    printf("%d\n",(int)(expression));
else
    printf("%.6f\n",(expression));
```

#### **Function Translation**
- Parses function definitions with parameters
- Manages local variable scope within functions
- Handles return statements (adds default `return 0.0;` if missing)
- Validates function calls against defined functions

### 3. **Code Generation**
The compiler assembles the final C program in this order:
1. **Headers**: `#include <stdio.h>`
2. **Command Line Arguments**: Global variables for `arg0`, `arg1`, etc.
3. **Global Variable Declarations**: All main-scope variables as `float`
4. **Function Definitions**: All user-defined functions
5. **Main Function**: Translated main program logic

### 4. **Compilation and Execution**
After generating the C code:
1. Saves translated code to `translation.c`
2. Compiles using: `cc -std=c11 -Wall -Werror -o ./ml translation.c`
3. Executes the compiled program: `./ml`
4. Cleans up temporary files

## Usage

### Basic Usage
```bash
runml <source.ml>
```

### With Command Line Arguments
```bash
runml <source.ml> [arg0] [arg1] ... [arg9]
```

The compiler accepts up to 9 additional command line arguments that become available as `arg0`, `arg1`, ..., `arg9` in your Arrow program. Only integer and float values are supported.

## Examples

### Simple Assignment and Printing
```arrow
x <- 2.5
print x          # Prints: 2.500000
```

### Arithmetic Operations
```arrow
x <- 8
y <- 3
print x * y      # Prints: 24
```

### Function Definition and Call
```arrow
function multiply a b
    return a * b

print multiply(12, 6)    # Prints: 72
```

### Using Command Line Arguments
```arrow
# Run with: runml program.ml 5.5 10.2
print arg0 + arg1        # Prints: 15.700000
```

## Language Restrictions

- **Variables**: Only lowercase alphabetic characters allowed
- **Data Types**: Only floating-point numbers supported
- **Keywords**: `print`, `function`, and `return` are reserved
- **Scope**: Functions cannot be defined inside other functions
- **Names**: Variable and function names must be 1-12 characters long

## Error Handling

The compiler provides comprehensive error checking:
- Syntax validation (brackets, illegal symbols)
- Variable name validation
- Function call validation
- Scope management
- Memory management with proper cleanup

## Technical Details

- **Memory Management**: Dynamic allocation for variable names and translated code
- **Buffer Sizes**: 1024 characters per line, expandable buffers for large programs
- **Platform**: Cross-platform C code generation
- **Standards**: C11 compliant output with strict compiler flags
