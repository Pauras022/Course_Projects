#include "assembler.h"
// Pass 1
// Takes program file as input
// and generates intermediate file
// and symbol table
int pass_one(FILE *inputFile, FILE *outputFile)
{

    fseek(inputFile, 0, SEEK_SET);

    //clear symbol table
    FILE *st = fopen(SYMTAB, "w");
    fclose(st);

    char *line = NULL, temp[MAXS];
    size_t len = 0;
    char *args[MAXW];
    int words;
    char *OPCODE, *LABEL, *OPERAND;

    // store addresses
    int LOCCTR;

    // read first line
    getline(&line, &len, inputFile);
    strcpy(temp, line);
    if (line[0] == ' ')
    {
        words = break_line(temp, 1, args);
        LABEL = NULL;
        OPCODE = args[1];
    }
    else
    {
        words = break_line(temp, 0, args);
        LABEL = args[0];
        OPCODE = args[1];
    }

    if (strcmp(args[1], "START") == 0)
    {
        OPERAND = args[2];
        STARTADDR = strtol(OPERAND, NULL, 16);
        strcpy(PROGNAME, LABEL);
        LOCCTR = STARTADDR;

        // write line to intermediate file
        fprintf(outputFile, "%0X\t%s", LOCCTR, line);

        // read next input line
        getline(&line, &len, inputFile);
        strcpy(temp, line);
        if (line[0] == ' ')
        {
            words = break_line(temp, 1, args);
            LABEL = NULL;
            OPCODE = args[1];
        }
        else
        {
            words = break_line(temp, 0, args);
            LABEL = args[0];
            OPCODE = args[1];
        }
    }
    else
    {
        LOCCTR = 0;
    }

    while (strcmp(OPCODE, "END") != 0)
    {
        if (line[0] != '.') // check if not a comment
        {
            if (LABEL) // check if symbol in label
            {
                if (search_symtab(LABEL))
                {
                    printf("Error: Duplicate Symbol. \n");
                    ERROR = 1;
                    return -1;
                }
                else
                {
                    insert_symtab(LABEL, LOCCTR);
                }
            }

            // write line to intermediate file
            fprintf(outputFile, "%0X\t%s", LOCCTR, line);

            if (search_optab(OPCODE))
            {
                LOCCTR = LOCCTR + 3 * ILEN;
            }
            else if (strcmp(OPCODE, "WORD") == 0)
            {
                LOCCTR = LOCCTR + 3;
            }
            else if (strcmp(OPCODE, "RESW") == 0)
            {
                OPERAND = args[2];
                LOCCTR = LOCCTR + 3 * strtol(OPERAND, NULL, 10); 
            }
            else if (strcmp(OPCODE, "RESB") == 0)
            {
                OPERAND = args[2];
                LOCCTR = LOCCTR + strtol(OPERAND, NULL, 10); 
            }
            else if (strcmp(OPCODE, "BYTE") == 0)
            {
                OPERAND = args[2];
                if (OPERAND[0] == 'C')
                    LOCCTR = LOCCTR + strlen(OPERAND) - 3;
                else if (OPERAND[0] == 'X')
                    LOCCTR = LOCCTR + (strlen(OPERAND) - 3 + 1) / 2; 
            }
            else
            {
                printf("Error: Invalid operation code. \n");
                ERROR = 1;
                return -1;
            }
        }
        else
        {
            fprintf(outputFile, "    \t%s", line);
        }

        // read next input line
        getline(&line, &len, inputFile);
        strcpy(temp, line);
        if (line[0] == ' ')
        {
            words = break_line(temp, 1, args);
            LABEL = NULL;
            OPCODE = args[1];
        }
        else
        {
            words = break_line(temp, 0, args);
            LABEL = args[0];
            OPCODE = args[1];
        }
    }

    //fprintf(outputFile, "    \t%s\n", line);
    fprintf(outputFile, "%0X\t%s", LOCCTR, line);
    
    // store program length
    PROGLEN = LOCCTR - STARTADDR;

    printf("PASS ONE COMPLETED.\n");

    return 1;
}


// Pass 2
// Takes intermediate file as input
// and generates object code
int pass_two(FILE *inputFile, FILE *outputFile, FILE *listingFile)
{
    // set read pointer to beginning of file
    fseek(inputFile, 0, SEEK_SET);

    char *line = NULL, temp[MAXS], addr[MAXS];
    size_t len = 0;
    char *args[MAXW];
    int words;
    char *OPCODE, *LABEL, *OPERAND;

    // read first line
    fscanf(inputFile, "%[^\t]s", addr);
    getline(&line, &len, inputFile);
    strcpy(temp, line);
    if (line[0] == ' ')
    {
        words = break_line(temp, 1, args);
        LABEL = NULL;
        OPCODE = args[1];
    }
    else
    {
        words = break_line(temp, 0, args);
        LABEL = args[0];
        OPCODE = args[1];
    }

    if (strcmp(OPCODE, "START") == 0)
    {
        // write listing line
        fprintf(listingFile, "%s%s", addr, line);

        // read next input line
        fscanf(inputFile, "%[^\t]s", addr);
        getline(&line, &len, inputFile);
        strcpy(temp, line);
        if (line[0] == ' ')
        {
            words = break_line(temp, 1, args);
            LABEL = NULL;
            OPCODE = args[1];
        }
        else
        {
            words = break_line(temp, 0, args);
            LABEL = args[0];
            OPCODE = args[1];
        }
    }

    // write header record
    fprintf(outputFile, "H%-6s%06X%06X\n", PROGNAME, STARTADDR, PROGLEN);

    // initialise first text record
    char record[MAXS] = "";
    char firstaddr[MAXS];
    sprintf(firstaddr, "%0X", STARTADDR);

    while (strcmp(OPCODE, "END") != 0)
    {
        char objcode[MAXS];
        strcpy(objcode, "");
        if (line[1] != '.') // check line not a comment
        {
            const char *opcode = search_optab(OPCODE);
            // check if opcode found
            if (opcode) 
            {
                strcat(objcode, opcode);
                // check if operand is symbol
                if (words > 2) 
                {
                    OPERAND = args[2];
                    int len = strlen(OPERAND);
                    int val = 0;

                    // check for adressing mode
                    if (len >= 2 && OPERAND[len - 2] == ',' && OPERAND[len - 1] == 'X')
                    {
                        OPERAND[len - 2] = '\0';
                        val = 0x8000;
                    }

                    char symval[MAXS] = "";
                    const char *symcode = search_symtab(OPERAND);
                    // check if symbol found
                    if (symcode)
                    {
                        strcpy(symval, symcode);
                    }
                    else
                    {
                        strcpy(symval, "0000");
                        printf("Error: Undefined Symbol.\n");
                        ERROR = 1;
                    }
                    strcpy(symval, symcode);
                    val = val | strtol(symval, NULL, 16);
                    sprintf(symval, "%04X", val);
                    strcat(objcode, symval);
                }
                else
                {
                    strcat(objcode, "0000");
                }
            }
            else if (strcmp(OPCODE, "BYTE") == 0)
            {
                OPERAND = args[2];
                if (OPERAND[0] == 'C')
                {
                    int c;
                    for (int i = 2; i < strlen(OPERAND) - 1; i++)
                    {
                        int c = OPERAND[i];
                        char temp[2];
                        sprintf(temp, "%0X", c);
                        strcat(objcode, temp);
                    }
                }
                else if (OPERAND[0] == 'X')
                {
                    strcat(objcode, OPERAND + 2);
                    objcode[strlen(objcode) - 1] = '\0';
                }
            }
            else if (strcmp(OPCODE, "WORD") == 0)
            {
                OPERAND = args[2];
                sprintf(objcode, "%06X", (int)strtol(OPERAND, NULL, 10));
            }

            // check if new record fits
            // or for discontinuity in adress
            if (strlen(record) + strlen(objcode) > 60 || strcmp(OPCODE, "RESW") == 0 || strcmp(OPCODE, "RESB") == 0)
            {
                if (strlen(record) > 1)
                {
                    fprintf(outputFile, "T%06X%02X%s\n", (int)strtol(firstaddr, NULL, 16),
                            (int)strlen(record) / 2, record);
                }
                strcpy(record, "");
            }

            // set start address of record
            if (strlen(record) == 0)
            {
                strcpy(firstaddr, addr);
            }
            strcat(record, objcode);

            // write listing line
            line[strlen(line) - 1] = '\0';
            fprintf(listingFile, "%s%-26s\t%s\n", addr, line, objcode);
        }
        else
        {
            fprintf(listingFile, "\t%s", line);
        }

        // read next input line
        fscanf(inputFile, "%[^\t]s", addr);
        getline(&line, &len, inputFile);
        strcpy(temp, line);
        words = break_line(temp, 0, args);
        LABEL = args[0];
        OPCODE = args[1];
    }

    // write last text record
    if (strlen(record) > 1)
    {
        fprintf(outputFile, "T%06X%02X%s\n", (int)strtol(firstaddr, NULL, 16),
                (int)strlen(record) / 2, record);
        strcpy(record, "");
    }

    // write end record
    fprintf(outputFile, "E%06X\n", (int)strtol(search_symtab(args[2]), NULL, 16));

    sprintf(record, "%06X", (int)strtol(search_symtab(args[2]), NULL, 16));
    // write last listing line
    fprintf(listingFile, "%s%-26s\t%s", addr, line, record);

    printf("PASS TWO COMPLETED.\n");

    return 1;
}