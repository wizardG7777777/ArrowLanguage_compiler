/* Caution: the following content is the warning from Microsoft:
 However, portable code shouldn't depend on the size of int because the language standard allows this to be implementation-specific.
 */
/*
 Thus, int32_t is necessary, don't define any integer variables with int rather int32_t
 For every mini language variables, define them with float type and assign them with an initial number: 0.0
 */
/*
 In this program, there are three main module: statement translation, print translation, function translation
 statement translation can transalte normal expression and return a string to main logic
 print can translate the printing call and return a string to main logic
 function translation can translate mini language as main logic does, it won't return anything
 Also, I have defined a lot function used for validation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// All name of variable or functions should be less than 13, means from 1 to 12, but program need to reserve a place for '\0'
#define name_length 13
#define buffer 1024 // Big enough for handle every line in mini language
//Delete these define when completed final test
#define output_file "./translation.c"
//The followings are global variables
//Both functions and variables which defined in main scope are allowed to access from anywhere, so in logic, they are global variables
int32_t number_of_global_variable;
int32_t number_of_function;
char *variables_name_global[60];
char *function_name[60];
// The following two strings are extremely big because we need to ensure they can handle all the content. Although they may consume a lot of memory, but it is necessary for handling extreme edge case
char scope_mainfunction[buffer*buffer];
char scope_localfunction[buffer*buffer];
char *file_content[100];
int32_t current_line_number=0;
//The following functions are prototype functions
int ReadFromFile(FILE *ml);
void WriteInto(char line[buffer], int32_t flag); // This function is responsible for write any string in the scope it should stay
void TranslateComment(char *line,int32_t length);
int KeyWordDetect(char *line, int32_t length);
// The following function are used for check and validation
// Check the character is alphabetic
int IsAlphabetic(char c);
// Check whether the character is a space
int IsSpace(char c);
// Check whether the character is a digital number
int IsDigital(char c);
// Check whether the character is one of +,-,*,/
int IsArithmeticOperators(char c);
// Check whether the current line is a total empty line
int IsEmptyLine(char *line, int32_t length);
// Check whether the current number is a float number
int IsFloatNumber(char *name);
// Check whether the current variable is a valid name
int NameValidityCheck(char *name);
// Check bracket Integrity for current line
int Bracket_Integrity_Check(char *line, int32_t length);
int Illegal_Symbol_Check(char *line, int32_t length);
int IsVariableExistInList(char *name);
int IsFunctionExistInList(char *name);
// The following two will return a string which contain the translated content
char *StatementTranslatation(char *line, int32_t length, int32_t flag, char *variable_list[], int32_t *number_of_variable);
char *PrintTranslation(char *line, int32_t length, int32_t flag, char *variable_list[], int32_t *number_of_variable);
char *Num2String(int32_t x);
/* Function translation need to translate the line which define the function, also it need to control the following translate process and make sure statements are written into the right scope */
int FunctionTranslation(char *line, int32_t length, int32_t linenumber_total);


int main(int argc, const char * argv[]) {
//The following three line should be deleted in the final test process
/*
    These are arguments for debugging
    argc = 2;
    argv[0] = "runml";
    argv[1] = source_file;
 */
    if (argc < 2){
        // If user input is not precisely 1 file, program will exit failure and report this error
        // Users should input at least two argument
        fprintf(stderr, "!This translation program consider only one input source file without any flag! \n");
        fprintf(stderr, "!Usage: runml <source.ml> \n");
        exit(EXIT_FAILURE);
    }
    else{
        FILE *source = fopen(argv[1], "r");
        // Caution, translation is a global file pointer, it should only be closed by main function
        // Global variables initialization
        if (source == NULL){
        // Test whether the user input exist or not, if there is no such file, warn user and exit failure
            fprintf(stderr, "!No such file or directory \n");
            fclose(source);
            exit(EXIT_FAILURE);
        }
        // Initialize main function scope and local function scope
        strcpy(scope_mainfunction, "\0");
        strcpy(scope_localfunction, "\0");
        number_of_function = 0;
        number_of_global_variable = 0;
        // Add head of main function to main function scope
        strcpy(scope_mainfunction, "int main(){\n");
        if (ReadFromFile(source)) exit(EXIT_FAILURE);
    }
    FILE *translation = NULL;
    translation = fopen(output_file, "w");
    char Assemble[buffer*buffer*2]="\0";
    if (translation == NULL) {
    // For any reason, if program can not write into filesystem, it will exit
        fprintf(stderr, "!Fail to write translation into file system \n");
        fclose(translation);
        exit(EXIT_FAILURE);
    }
    strcat(scope_mainfunction, "return 0;}\n");
// Finally, it is time to assemble customized function and main function and write into LocalFile system
//
    char global_defination[buffer*10] = "\0";
    strcpy(global_defination, "#include <stdio.h>\n"); //Anyway, include must be placed at the top of file
    if (argc>2) {
        // Which means command line argument exist, program need to deal with this situation
        int32_t number_argument = argc-2;
        char *command_line_arguments=malloc(buffer*10*sizeof(char));
        strcpy(command_line_arguments, "\0");
        for (int32_t i=0; i<number_argument; i++) {
            strcat(command_line_arguments, "float arg");
            strcat(command_line_arguments, Num2String(i));
            strcat(command_line_arguments, "=");
            strcat(command_line_arguments, argv[i+2]);
            strcat(command_line_arguments, ";\n");
        }
        strcat(Assemble, command_line_arguments);
        free(command_line_arguments);
    }
    for (int32_t i=0; i<number_of_global_variable; i++) {
        strcat(global_defination, "float ");
        strcat(global_defination, variables_name_global[i]);
        strcat(global_defination, "=0.0;\n");
    }
    strcat(Assemble, global_defination);
    strcat(Assemble, scope_localfunction);
    strcat(Assemble, scope_mainfunction);
    fprintf(translation, "%s", Assemble);
    fclose(translation); // In final step, close the file
    // Don't forget to free the memory that program used
    for (int32_t i=0; i<number_of_global_variable; i++)
        free(variables_name_global[i]);
    for (int32_t i=0; i<number_of_function; i++)
        free(function_name[i]);
    // Said by project description page, our C program need to compile and run mini language automatically
    char command[buffer]="\0";
    strcat(command, "cc -std=c11 -Wall -Werror -o ./ml ");
    strcat(command, output_file);
    strcat(command, "\n");
    if (system(command)) {
        fprintf(stderr, "!Some goes wrong with my compiler\n");
        exit(EXIT_FAILURE);
    }
    else{
        system("./ml");
    }
    remove("./ml");
    remove("./translation.c");

    return 0;
}
int ReadFromFile(FILE *ml){
//This function can read from source file line by line, also it is responsible for main program logic scope
//This function is not responsible for reporting error message to stderr, it only need to report to main function and make it exit with failure code
    int32_t linenumber_total = 0;
    char line[buffer];
    const char *line_translated = "\0";
    while (fgets(line, buffer, ml) != NULL) {
        file_content[linenumber_total]=malloc(buffer*sizeof(char));
        strcpy(file_content[linenumber_total], line);
        linenumber_total++;
    }
    fclose(ml);
    while (current_line_number<linenumber_total) {
        char *line = file_content[current_line_number];
        int32_t line_length = (int32_t)strlen(line);
        //Read and translate one line until the end of file, variable line will hold all the content
        TranslateComment(line, line_length);
        line_length = (int32_t)strlen(line); //ReCalculate, because current line may be modified
        if (Bracket_Integrity_Check(line, line_length)) {
            return 1;
        }
        if (Illegal_Symbol_Check(line, line_length)) {
            return 1;
        }
        if (IsEmptyLine(line, line_length)) {
            current_line_number++;
            continue;
        }
        switch (KeyWordDetect(line, line_length)) {
            case 0:
                break;
            case 1:
                // If keyword print is detected, then call print translation function
                line_translated = PrintTranslation(line, line_length, 1, variables_name_global, &number_of_global_variable);
                if (line_translated == NULL) return 1;
                else {
                    strcat(scope_mainfunction, line_translated);
//                    free(line_translated);
                }
                break;
            case 2:
                // If keyword function is detected, the call function translation function
                
                if (FunctionTranslation(line, line_length, linenumber_total)) return 1;
                break;
            case 3:
                // If no keyword is founded, then threat the word which deteced as a variable
                line_translated = StatementTranslatation(line, line_length, 1, variables_name_global, &number_of_global_variable);
                if (line_translated == NULL) return 1;
                else {
                    strcat(scope_mainfunction, line_translated);
                }
                break;
            case -2:
                // For this flag, there is an unexpected tab or space exist in current line, program should output error
                fprintf(stderr, "!Unexpected tab or space in %s \n",line);
                return 2;
                break;
            default:
                fprintf(stderr, "!Unknown error appear, program is exiting \n");
                return 2;
                break;
        }
        current_line_number++;
    }
    for (int32_t i=0; i<linenumber_total; i++)
        free(file_content[i]);
    return 0; // 0 means whole function works well, ready to exit
}
void TranslateComment(char *line, int32_t length){ //This function works well
    //Comment for above, since the maxium size of line is 1024, it is unnecessary for us to store them with unsighed long.
    for (int32_t i=0; i<length; i++) {
        if (*(line+i) == '#') {
            *(line+i) = '\0' ;
            return;
        }
    }
}
int KeyWordDetect(char *line, int32_t length){
    if (length == 0) return 0; // No need to translate an empty line, return 0 to warn reading function
    char keyword[buffer]="\0";
    for (int32_t i=0; i<length; i++) {
        if (*(line+i)==' ' || *(line+i)=='\t') {
            strncpy(keyword, line, i);
            break;
        }
    }
    if (strlen(keyword)==0) return -2; // This error may caused by unexpected space at the head of current line
    if (!strcmp(keyword, "print")) return 1; // keyword print is detected
    else if (!strcmp(keyword, "function")) return 2; // keyword function is detected
    else if (!strcmp(keyword, "return")) return 4; //keyword return is detected
    else return 3; // No keyword is detected, may be program should threat it as a normal statement
    
    return -1; //For any unknown error exit with -1
}
char *StatementTranslatation(char *line, int32_t length, int32_t flag, char *variable_list[], int32_t *number_of_variable){
    /*
     This function is responsible for translate all statements, both main function scope and other function scope
     In order to define and initial a variable when we need, it require a list of function name and the number of variables in the list.
     For variables in main scope, add it in global list and do not define them in here, for variables in other function, define them here. In both situation, add them in variable list.
     Use flag to identify which situation is the function facing
     */
    // Variables intialization
    int32_t assign_operator_position = -2;
    char *translated_line = malloc(buffer*sizeof(char));
    char current_variable_name[buffer]="\0";
    //point out the start and end of variable name and function name
    int32_t start_pointer = 0;
    // find where is assign operator
    for (int32_t i=0; i<length; i++) {
        if (*(line+i)=='<'&&*(line+i+1)=='-') {
            assign_operator_position=i;
            break;
        }
    }
    /*
    Just treat the block in left side as a variable， because in legal expression, only variable is allowed to appear in the left side of expression
     */
    if (assign_operator_position>0) {
        // program need to check what is the actual length of variable
        // Suppose there is no space at the head of expression
        int32_t actual_variable_length = assign_operator_position-1;
        while (actual_variable_length>0) {
            if (IsSpace(*(line+actual_variable_length))) {
                actual_variable_length--;
            }
            else break;
        }
        strncpy(current_variable_name, line, actual_variable_length+1);
        if (NameValidityCheck(current_variable_name)) {
            free(translated_line);
            return NULL;
        }
        int32_t found = -1;
        for (int32_t i=0; i<*number_of_variable; i++) {
            if (!strcmp(current_variable_name, variable_list[i])) {
                found=1;
                break;
            }
        }
        if (found<0) {
            variable_list[*number_of_variable]=malloc(name_length*sizeof(char));
            strcpy(variable_list[*number_of_variable], current_variable_name);
            *number_of_variable = *number_of_variable + 1;
            if (flag==2) {
            // 2 means translate program is current in other function scope
                strcat(translated_line, "float ");
                strcat(translated_line, current_variable_name);
                strcat(translated_line, "=0.0;\n");
            }
        }
    }
    else if (assign_operator_position==0){
    // This branch is used for deal with some edge cases
        fprintf(stderr, "!Missing variable");
        return NULL;
    }
    // Translate the right side of expression
    for (int32_t i=assign_operator_position+2; i<length; i++) {
    // Find any variables which is not defined before
        if (IsAlphabetic(*(line+i)) && !IsAlphabetic(*(line+i-1))) {
            start_pointer = i;
        }
        else if(!IsAlphabetic(*(line+i)) && IsAlphabetic(*(line+i-1))){
            // Identify the current name is a function name or a variable name
            int32_t function_name_symbol = -1;
            for (int32_t j=i; j<length; j++) {
                if (*(line+j)=='(') {
                    function_name_symbol=1;
                    break;
                }
                // Only space character is allowed to appear between alphabetic and left bracket
                else if (IsSpace(*(line+j))) continue;
                // Other wise, just exits
                else break;
            }
            // copy the string from start_point to i
            strncpy(current_variable_name, line+start_pointer, i-start_pointer);
            if (NameValidityCheck(current_variable_name)) return NULL;
            if (function_name_symbol==1){
            // In this case, program need to check whether this is a invalid function call
                int32_t function_found = -1;
                for (int32_t k=0; k<number_of_function; k++) {
                    if (!strcmp(function_name[k], current_variable_name)) {
                        function_found = 1;
                        break;
                    }
                }
                if (function_found == -1) {
                    fprintf(stderr, "!Invalid function call");
                    return NULL;
                }
            }
            else{
            /*  For variable, use it before define it is totally legally
                Check whether it is defined, if not, define it.
             */
                int32_t found = -1;
                for (int32_t i=0; i<*number_of_variable; i++) {
                    if (!strcmp(current_variable_name, variable_list[i])) {
                        found=1;
                        break;
                    }
                }
                if (found<0) {
                    variable_list[*number_of_variable]=malloc(name_length*sizeof(char));
                    strcpy(variable_list[*number_of_variable], current_variable_name);
                    *number_of_variable = *number_of_variable + 1;
                    if (flag==2) {
                        // 2 means translate program is current in other function scope
                        strcat(translated_line, "float ");
                        strcat(translated_line, current_variable_name);
                        strcat(translated_line, "=0.0;\n");
                    }
                }
            }
        }
    }
    // convert <- into  = , also don't forget ; at the end of expression
    if (assign_operator_position>0) {
        strncat(translated_line, line, assign_operator_position-1);
        strcat(translated_line, "=");
        strncat(translated_line, line+assign_operator_position+2, length-assign_operator_position-2);
    }
    else{
        strncat(translated_line, line, length-1);
    }
    strcat(translated_line, ";\n");
    return translated_line;
}
char *PrintTranslation(char *line, int32_t length, int32_t flag, char *variable_list[], int32_t *number_of_variable){
    // In order to retuen a string, I have to define a char pointer, also pre-allocate memory for it
    char *translated_line = malloc(buffer*sizeof(char));
    char current_expression[buffer]="\0";
    // There are 5 characters in print and 1 space attached at the rear of keyword, so start from index 6
    strncpy(current_expression, line+6,length-7);
    // The following translation plan may not perfect, it just works
    /*
     Convert all number into integer manually, then minus itself.
     For example, there is a number x, if x-(int)x equals 0, then x is actually an integer, else, x is not an integer.
     The final translated statement should like this:
     if ((expression)-(int)(expression)==0) printf("%d\n",expression);
     else printf("%.6f\n",expression);
     */
    if (Bracket_Integrity_Check(line, length)) return NULL;
    if (Illegal_Symbol_Check(line, length)) return NULL;
    int32_t start_pointer=0;
//    char *current_variable_name=malloc(name_length*sizeof(char));
    for (int32_t i=6 ; i<length; i++) {
    // Find any variables which is not defined before
        if (IsAlphabetic(*(line+i)) && !IsAlphabetic(*(line+i-1))) {
            start_pointer = i;
        }
        else if(!IsAlphabetic(*(line+i)) && IsAlphabetic(*(line+i-1))){
            // Identify the current name is a function name or a variable name
            int32_t function_name_symbol = -1;
            for (int32_t j=i; j<length; j++) {
                if (*(line+j)=='(') {
                    function_name_symbol=1;
                    break;
                }
                // Only space character is allowed to appear between alphabetic and left bracket
                else if (IsSpace(*(line+j))) continue;
                // Other wise, just exits
                else break;
            }
// copy the string from start_point to i
//            memcpy(current_variable_name, line+start_pointer, i-start_pointer);
            char *temperoary = malloc(name_length*sizeof(char));
            strncpy(temperoary, line+start_pointer, i-start_pointer);
//            printf("%s\n",current_variable_name);
            if (NameValidityCheck(temperoary)) return NULL;
            if (function_name_symbol==1){
            // In this case, program need to check whether this is a invalid function call
                int32_t function_found = -1;
                for (int32_t k=0; k<number_of_function; k++) {
                    if (!strcmp(function_name[k], temperoary)) {
                        function_found = 1;
                        break;
                    }
                }
                if (function_found == -1) {
                    fprintf(stderr, "!Invalid function call");
                    return NULL;
                }
            }
            else{
            /*  For variable, use it before define it is totally legally
                Check whether it is defined, if not, define it.
             */
                int32_t found = -1;
                for (int32_t i=0; i<*number_of_variable; i++) {
                    if (!strcmp(temperoary, variable_list[i])) {
                        found=1;
                        break;
                    }
                }
                if (found<0) {
                    variable_list[*number_of_variable]=malloc(name_length*sizeof(char));
                    strcpy(variable_list[*number_of_variable], temperoary);
                    *number_of_variable = *number_of_variable + 1;
                    if (flag==2) {
                        // 2 means translate program is current in other function scope
                        strcat(translated_line, "float ");
                        strcat(translated_line, temperoary);
                        strcat(translated_line, "=0.0;\n");
                    }
                }
            }
            free(temperoary);
        }
    }
    strcat(translated_line, "if ((");
    strcat(translated_line, current_expression);
    strcat(translated_line, ")-(int)(");
    strcat(translated_line, current_expression);
    strcat(translated_line, ")==0) printf(\"%d\\n\",(int)(");
    strcat(translated_line, current_expression);
    strcat(translated_line, "));\nelse printf(\"%.6f\\n\",(");
    strcat(translated_line, current_expression);
    strcat(translated_line, "));\n");
    return translated_line;
}
int FunctionTranslation(char *line, int32_t length, int32_t linenumber_total){
    // There are 8 characters in words function, and a space attached by the rear, so the real function name is beginning from index 9
    char current_function_name[name_length] = "\0"; // Initialization
//    char current_variable_name[name_length] = "\0";
    int32_t number_of_localvariables = 0;
    char *local_variable_list[32]; 
    // Normally, there is not a function require 32 arguments, so 32 is big enough to handle those arguments
    int32_t flag_return=0;
    // mini-language does not require return mandatory, but C languague need one if the type of function is not void. So, if there is no keyword: return in the origional file, then we need to add one
    int32_t i = 9; // There are 8 words in word function, and a space attached by it, thus the real function name begins at index 9
    while (i<length) {
        if (*(line+i)==' ' || *(line+i)=='\t' || *(line+i)=='\n') {
            strncpy(current_function_name, line+9, i-9);
            if (NameValidityCheck(current_function_name)) return 1;
            // Once the function name passed the check, add it into list
            function_name[number_of_function] = malloc(name_length*sizeof(char));
            strcpy(function_name[number_of_function], current_function_name);
            number_of_function++;
            strcat(scope_localfunction, "float ");
            strcat(scope_localfunction, current_function_name);
            strcat(scope_localfunction, "( ");
            break;
        }
        i++;
    }
    // Read and add all the function arguments
    for (int32_t j=i+1; j<length; j++) {
        if (*(line+j)==' ' || *(line+j)=='\t' || *(line+j)=='\n') { // when meet a space, then there is an argument
            if (j==i+1){
            // Well, sometimes program may face with consecutive multiple spaces, and program must not include any space
                i=j;
                continue;
            }
            else{
                local_variable_list[number_of_localvariables] = malloc(name_length*sizeof(char));
                strncpy(local_variable_list[number_of_localvariables], line+i+1, j-i-1);
                if (NameValidityCheck(local_variable_list[number_of_localvariables])){
                // Free the memory before exit, even if program is exiting with some failure
                    for (int32_t j=0; j<=number_of_localvariables; j++)
                        free(local_variable_list[j]);
                    return 1;
                }
                strcat(scope_localfunction, "float ");
                strcat(scope_localfunction, local_variable_list[number_of_localvariables]);
                strcat(scope_localfunction, ",");
                i=j;
                number_of_localvariables++;
            }
        }
    }
    scope_localfunction[strlen(scope_localfunction)-1] = ')'; // Exchange the comma at the rear of string into bracket
    current_line_number++;
    strcat(scope_localfunction, "{\n");
    // function define line has been translated, next is translating every line in local function scope
    while (current_line_number<linenumber_total){
        if (IsEmptyLine(file_content[current_line_number], length)) {
            current_line_number++;
            continue;
        }
        if (Bracket_Integrity_Check(line, length)) {
            return 1;
        }
        if (Illegal_Symbol_Check(line, length)) {
            return 1;
        }
        char *line=file_content[current_line_number];
        int32_t length = 0; // This length is used to record the length of statement
        length = (int32_t)strlen(line);
        TranslateComment(line, length);// \t can be ignored because it could not affect translation for now
        if (line[0]!='\t'){
            // if the current line is not begin with tab, then we may consider the current function scope is done
            // program has already read the statement from main scope, back to last line
            current_line_number--;
            break;
        }
        // Also, TranslateComment may affect the length of current line, then it is necessary to recalculate the length of line
        length = (int32_t)strlen(line)-1;
        // Now \t will affect translation, it must be excluded from current line, use line+1 can exclude the \t at the head of line
        const char *line_translated = NULL;
        switch (KeyWordDetect(line+1, length)) {
            case 0:
                break;
            case 1:
                // If keyword print is detected, then call print translation function
                // Because there is always a \t at the head of line, so translate should begin from the second character
                line_translated = PrintTranslation(line+1, length, 1, local_variable_list, &number_of_localvariables);
                if (line_translated == NULL) return 1;
                else {
                    strcat(scope_localfunction, line_translated);
                }
                break;
            case 2:
                // If keyword function is detected, the call function translation function
                return 1; // define function inside function is not allowed in this mini-language
                break;
            case 3:
                // If no keyword is founded, then threat the word which deteced as a variable
                line_translated = StatementTranslatation(line+1, length-1, 2, local_variable_list, &number_of_localvariables);
                if (line_translated == NULL) return 1;
                else {
                    strcat(scope_localfunction, line_translated);
                }
                break;
            case 4:
                // In this case, keyword return is detected, set return flag as 1
                // Well, actually there is nothing need to be translated, but since keyword return is detected, keep this case for any additional code for new feathures
                flag_return=1;
                // Maybe add something to make sure program still work eve if function tend to return nothing
                strcat(scope_localfunction, line+1);
                strcat(scope_localfunction, ";");
                break;
            case -2:
                // For this flag, there is an unexpected tab or space exist in current line, program should output error
                fprintf(stderr, "!Unexpected tab or space in %s \n",line);
                return 2;
                break;
            default:
                fprintf(stderr, "!Unknown error appear, program is exiting \n");
                return 2;
                break;
        }
        current_line_number++;
    }
    if (flag_return==0) {
        strcat(scope_localfunction, "return 0.0;");
    }
    strcat(scope_localfunction, "}\n"); //Finally complete the local function scope
    // Don't forget free the memory we allocated, when the array is no longer be used by program
    for (int32_t j=0; j<number_of_localvariables; j++) {
        free(local_variable_list[j]);
    }
    return 0;
}

int NameValidityCheck(char *name){
    // Check whether the variable name is valid, if not, this function is responsible for reporting error
    int32_t length = (int32_t)strlen(name);
    if (length<1 || length>name_length) {
        fprintf(stderr, "!Invalid variable name: empty or too long \n");
        return 1; // flag 1 means the length of variable is invalid
    }
    for (int32_t i=0; i<length; i++) {
        if (*(name+i)<97 || *(name+i)>122) {
            fprintf(stderr, "Invalid variable name: it only condered lower case alphabetic characters \n");
            return  2; // flag 2 means the characters of variable name is invalid
        }
    }
    if (!strcmp(name, "print") || !strcmp(name, "function")) {
        fprintf(stderr, "!Invalid variable name: reserved keyword cannot be used as variable name or function name \n");
        return 3;
    }
    return 0;
}
int Bracket_Integrity_Check(char *line, int32_t length){
    int32_t unparied_bracket_number = 0;
    for (int32_t i=0; i<length; i++) {
        if (*(line+i)=='(') unparied_bracket_number++;
        else if (*(line+i)==')') unparied_bracket_number--;
        // if unparied_bracket_number is less than 0, means unparied ) appear in expression
        if (unparied_bracket_number<0) {
            fprintf(stderr, "!Encountered unparied brackets");
            return 1;
        }
    }
    if (unparied_bracket_number!=0) {
        // if unparied_bracket_number is not equal to 0, means unparied ( appear in expression
        fprintf(stderr, "!Encountered unparied brackets");
        return 1;
    }
    return 0;
}
int Illegal_Symbol_Check(char *line, int32_t length){
    // This function can not only check whether there is a invalid symbol, it also can check some syntax error
    for (int32_t i=0; i<length; i++) {
        if (IsAlphabetic(*(line+i))) continue;
        else if (IsSpace(*(line+i))) continue;
        else if (IsDigital(*(line+i))) continue;
        else if (*(line+i)=='(' || *(line+i)==')') continue;
        else if (*(line+i)=='.' && i>0 && i<length-1){
            if (IsDigital(*(line+i+1)) && IsDigital(*(line+i-1))) continue;
            else {
                fprintf(stderr, "!Illegal character found");
                return 1;
            }
        }
        else if (*(line+i)=='<'){
            if (*(line+i+1)=='-' && i<length-2) {
                i++;
                continue;
            }
            else {
                fprintf(stderr, "!Illegal character found");
                return 1;
            }
        }
        else if (IsArithmeticOperators(*(line+i))) continue;
        else if (*(line+i)==',') continue;
        else {
            fprintf(stderr, "!Illegal character found");
            return 1;
        }
    }
//    int32_t valid_symbol_flag=1;
    // The following block can ensure there is no such "a b" or "a++" exist in statement
/*
    for (int32_t i=0; i<length; i++) {
        // This time, we need to suppose those brackets are paired, also all float number are valid float number
        if (IsAlphabetic(*(line+i)) && !IsAlphabetic(*(line+i-1))) {
            if (valid_symbol_flag==1) {
                valid_symbol_flag=-1;
                continue;
            }
            else return 1;
        }
        else if (IsDigital(*(line+i)) && !IsDigital(*(line+i))){
            if (valid_symbol_flag==1) {
                valid_symbol_flag=-1;
                continue;
            }
            else return 1;
        }
        else if (*(line+i)=='(' || *(line+i)==')' || *(line+i)=='.' || IsArithmeticOperators(*(line+i))) {
            if (valid_symbol_flag==-1) {
                valid_symbol_flag=1;
                continue;
            }
            else return 1;
        }
        else if (*(line+i)=='<' && *(line+i+1)=='-'){
            if (valid_symbol_flag==-1) {
                valid_symbol_flag=1;
                i++;
                continue;
            }
            else return 1;
        }
    }
 */
    return 0;
}
int IsAlphabetic(char c){
    if (c>96 && c<123) return 1;
    else return 0;
}
int IsSpace(char c){
    // Check whether c is a sapce character, if it is, then return 1, else return 0
    switch (c) {
        case ' ':
            return 1;
            break;
        case '\t':
            return 1;
            break;
        case '\n':
            return 1;
    // '\r' is also valid in windows, although it will not appear in MacOS or Linux
        case '\r':
            return 1;
        default:
            return 0;
            break;
    }
}
int IsDigital(char c){
    if (c>47 && c<58) return 1;
    else return 0;
}
int IsArithmeticOperators(char c){
// In this mini language '=' is not allowed here
    switch (c) {
        case '+':
            return 1;
            break;
        case '-':
            return 1;
            break;
        case '*':
            return 1;
            break;
        case '/':
            return 1;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}
int IsEmptyLine(char *line, int32_t length){
    for (int32_t i=0; i<length; i++)
        if (!IsSpace(*(line+i)))
            return 0;
    return 1;
}
int IsVariableExistInList(char *name){
    // Check whether the variable in main control-flow is exist or not
    // If not, this function can add a new one
    for (int32_t i=0; i<number_of_global_variable; i++) {
        // The variable exist in variable list, no list to define it again
        if (!strcmp(variables_name_global[i], name)) return 1;
    }
    variables_name_global[number_of_function] = name;
    number_of_global_variable++;
    return 0;
}
int IsFunctionExistInList(char *name){
    for (int32_t i=0; i<number_of_function; i++) {
        if (!strcmp(name, function_name[i]))
            return 1;
    }
    function_name[number_of_function] = name;
    number_of_function++;
    return 0;
}
char *Num2String(int32_t x){
    // We only support 99 arguments at most
    char *result=malloc(3*sizeof(char));
    if (x<10) {
        strcpy(result, "0\0");
        *(result)=(char)(x+48);
    }else if (x<100){
        strcpy(result, "00");
        *(result+1)=(char)(x%10+48);
        x=x/10;
        *result=(char)(x+48);
    }
    return result;
}
//  CITS2002 Project 1 2024
//  Student1:   24256987   Yanchen Yu
//  Student2:   23954936   Jiabao Liu
//  Platform:   Apple
