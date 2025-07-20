# ArrowLanguage
## Syntax
It is very similar to Python, no need to declare variables before use it, but ArrowLanguage only accept only float variables.
### Python code
1. x = 1
2. x = x + 1
3. z = x + y
4. x = x * x
5. print(x+1)
6. function defination
```Python
def add(a, b):
    return a + b

```
### Arrow language code
1. x <- 1
2. x <- x + 1
3. z <- x + y
4. x <- x * x
5. print(x+1)
6. function defination
```Arrow
function add(a, b)
    return a + b
```
## Others
### Compiler
runml.c is the source code of compiler, it can translate Arrow language into C language and compile it automatically.
### Command Line augments
The executable file which is compiled from arrow language can accept up to 9 extra commandline augments.
To use them, use arg0, arg1, ..., arg9. Only Integer and Float variable are acceptable, no others is available.