#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "c_extern.h"

#ifdef ONE_GAME_DIRECTORY
#define CSV_FILE_PATH ""
#else
#define CSV_FILE_PATH "Assets\\Text\\"
#endif

#define CSV_FILE_NAME "ctxt.csv"
#define CSV_DELIMITATOR ','
#define CSV_QUOTATION '"'
#define CSV_BUFFER_SIZE 1024
#define CSV_INVALID_INT_VALUE -1
#define CSV_INVALID_TEXT_VALUE ""

#define LBA_ENGINE_LINE_BREAK " @ " // used to replace a '\n' in a multi-line column with the string value that the engine interprets as a line break (found in dialogs).

#define ASCII_MIN_INT 48
#define ASCII_MAX_INT 57

#define OUT

FILE* _customMultiTextFile; 

char* fgets_dynamic(char **line, size_t *bufferSize, FILE *file);

// struct returned upon calling readLong function
struct csvLongValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where an integer column ended
    LONG value;
};

// struct returned upon calling readText function
struct csvTextValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where a text column ended
    char* value;
};

// struct returned upon calling treatSingleColumn function
struct treatSingleColumnResult
{
    int lastLineIndex; // helper variable to indicate the index of the line where a text column ended (taking into consideration a multi-line column case)
    int treatedColLength;
    char* value;
};

// struct returned upon calling CheckIfLineHasEndingQuote function
struct endingQuoteResult
{
    int lastLineIndex; // helper variable to indicate the index of the line where a multi-line quote ended
    bool result;
};

/* 
* In CSV, if a column starts with a quotation mark, it must end with a quotation mark. 
* Everything will be assimilated as part of the text until a final quotation mark is found. 
* If this final quotation mark is not found, there must be an error in the CSV formatting.
*/
bool hasQuotationMarks(char *line, int lineLength, int index)
{
    return index < lineLength ? line[index] == CSV_QUOTATION : false;
}

bool isIndexDelimitator(char *line, int lineLength, int index)
{
    return index < lineLength ? line[index] == CSV_DELIMITATOR : false;
}

struct treatSingleColumnResult* createSingleColumnResult(int lineLength)
{
    struct treatSingleColumnResult* result = calloc(1, sizeof(struct treatSingleColumnResult));

    if (!result)
        return NULL;

    result->lastLineIndex = CSV_INVALID_INT_VALUE;
    result->treatedColLength = CSV_INVALID_INT_VALUE;
    result->value = calloc(lineLength + 1, sizeof(char));

    if (!result->value)
        return NULL;

    return result;
}

/*
* Returns the result of the parsed column after treatment of CSV formatting. result.value for the treated string, result.lastLineIndex for the last index of the line read
* result.treatedColLength for the length of the treated string.
* 
* Parameters:
*   line: the current line being read from the file
*   lineLength: the length of the line (typically equivalent to strlen)
*   index: the index value to start reading in the line
*/
struct treatSingleColumnResult* treatSingleColumnWithoutQuotes(char* line, int lineLength, int index)
{
    struct treatSingleColumnResult* result = createSingleColumnResult(lineLength);
    int i = index;
    int treatedColIndex = 0;

    if (!result)
        return NULL;

    if (!line)
        return result;

    while (i < lineLength && treatedColIndex < lineLength)
    {
        //Ignore NUL characters
        if (line[i] == '\0')
        {
            ++i;
            continue;
        }

        // If current character is a delimitator or a line break, then this text has reached it's end. 
        if (line[i] == '\n' || isIndexDelimitator(line, lineLength, i))
            break;
        // Else append character to treatedColumn
        else result->value[treatedColIndex++] = line[i];

        ++i;
    }

    result->lastLineIndex = i;
    result->treatedColLength = treatedColIndex;

    return result;
}

/*
* Returns the result of the parsed column after treatment of CSV formatting. result.value for the treated string, result.lastLineIndex for the last index of the line read
* result.treatedColLength for the length of the treated string.
* 
* Parameters:
*   line: the current line being read from the file
*   lineLength: the length of the line (typically equivalent of strlen)
*   index: the index value to start reading in the line
*/
struct treatSingleColumnResult* treatSingleColumnWithQuotes(char* line, int lineLength, int index)
{
    struct treatSingleColumnResult* result = createSingleColumnResult(lineLength);
    struct treatSingleColumnResult* nextLineResult = NULL;
    int i = index;
    int treatedColIndex = 0;

    if (!result)
        return NULL;

    if (!line)
        return result;

    //Iterate through each character of the line, starting from the index where the text column starts
    while (i < lineLength && treatedColIndex < lineLength)
    {
        //Ignore NUL characters
        if (line[i] == '\0')
        {
            ++i;
            continue;
        }

        //If current character is a quotation mark, evaluate what to do next
        if (line[i] == CSV_QUOTATION)
        {
            int nextI = i + 1;

            //If the next character is a quotation mark (double quotations), treat it as a quote mark part of the text to be printed
            if (nextI < lineLength && line[nextI] == CSV_QUOTATION)
            {
                result->value[treatedColIndex++] = line[nextI];
                i += 2;
            }
            //If the next character is not a quotation mark, we have reached the end of the column
            else 
            {  
                ++i;
                break;
            }
        }
        //Otherwise, if the current character is still under quotations, and a line break is found, it means the text has a new paragraph
        else if (line[i] == '\n')
        {
            size_t bufferSize = CSV_BUFFER_SIZE;
            char* tempValue;

            if (!_customMultiTextFile)
                break;

            if (feof(_customMultiTextFile))
                break;

            // Get the next line from the file
            fgets_dynamic(OUT &line, OUT &bufferSize, _customMultiTextFile);

            if (!line)
                break;

            // Recursively call this function to treat the next line
            nextLineResult = treatSingleColumnWithQuotes(line, strlen(line), 0);

            if (!nextLineResult || 
                nextLineResult->treatedColLength == CSV_INVALID_INT_VALUE || 
                !nextLineResult->value ||
                nextLineResult->value == CSV_INVALID_TEXT_VALUE)
                break;

            if (nextLineResult->value)
            {
                int tempIndex = treatedColIndex + nextLineResult->treatedColLength + strlen(LBA_ENGINE_LINE_BREAK);
                tempValue = realloc(result->value, (tempIndex + 1) * sizeof(char));
                
                if (tempValue)
                {
                    result->value = tempValue;
                    //concatenate a new line indication to the treated string, using LBA's engine specific line break (used in game dialogs)
                    strcat(result->value, LBA_ENGINE_LINE_BREAK);
                    //concatenate the next line to result->value
                    strcat(result->value, nextLineResult->value);

                    // Add the length of the new treated line to treatedColIndex
                    treatedColIndex = tempIndex;
                }
            }

            break;
        }
        //If the character doesn't match the previous conditions, simply append it to treatedColumn (including delimitators)
        else
        {
            result->value[treatedColIndex++] = line[i];
            ++i;
        }
    }

    result->lastLineIndex = nextLineResult && nextLineResult->lastLineIndex != CSV_INVALID_INT_VALUE ? nextLineResult->lastLineIndex : i;
    result->treatedColLength = treatedColIndex;
    //result->value = realloc(result->value, treatedColIndex);

    if(nextLineResult)
    {
        free(nextLineResult->value);
        free(nextLineResult);
    }

    return result;
}

// Function that parses a text column
struct csvTextValue readText(char* line, int lineLength, int index)
{
    // Set initial values to invalid incase the parsing fails
    struct csvTextValue returnValue = { CSV_INVALID_INT_VALUE, CSV_INVALID_TEXT_VALUE };

    bool lineWithQuotes = hasQuotationMarks(line, lineLength, index);
    int i = index + lineWithQuotes;

    struct treatSingleColumnResult* treatedTextResult;

    if (lineWithQuotes)
        treatedTextResult = treatSingleColumnWithQuotes(line, lineLength, i);
    else treatedTextResult = treatSingleColumnWithoutQuotes(line, lineLength, i);

    if (treatedTextResult)
    {
        returnValue.lastLineIndex = treatedTextResult->lastLineIndex;
        
        if(treatedTextResult->value)
        {
            returnValue.value = calloc(strlen(treatedTextResult->value) + 1, sizeof(char));
            
            strcpy(returnValue.value, treatedTextResult->value);
        }

        free(treatedTextResult->value);
        free(treatedTextResult);
    }

    return returnValue;
}

// Function that parses an integer column
struct csvLongValue readLong(char* line, int lineLength, int index)
{
    // Set initial values to invalid incase the parsing fails
    struct csvLongValue returnValue = { CSV_INVALID_INT_VALUE, CSV_INVALID_INT_VALUE };
    
    char* column;
    int columnLen = 0;
    
    int i = index;

    // Iterate through each character
    while(i < lineLength)
    {
        // Check if the character is in the ASCII range for integers
        bool isIndexValidNumber = line[i] >= ASCII_MIN_INT && line[i] <= ASCII_MAX_INT;

        // If the current character is a delimitator, we have reached the end of this column
        if (isIndexDelimitator(line, lineLength, i))
            break;
        // If a non integer character is found, it means we cannot use this column. Break the loop.
        else if (!isIndexValidNumber)
        {
            columnLen = 0;
            break;
        }
        
        ++columnLen;
        ++i;
    }

    //If we have a valid number with a length
    if (columnLen > 0)
    {
        column = calloc(columnLen + 1, sizeof(char));

        if (column)
        {
            // copy the part of the line that corresponds to the integer
            strncpy(column, line, columnLen);

            returnValue.lastLineIndex = i;
            //Convert ascii to long
            returnValue.value = atol(column);
        }
        else
        {
            returnValue.lastLineIndex = i;
            returnValue.value = CSV_INVALID_INT_VALUE;
        }

        free(column);
    }

    return returnValue;
}

// Function that checks if a line has a csv quotation mark that is unended (indicating that next lines in the file are to be treated as part of the column)
// Returns 1 if there is an unended quote, 0 if not.
bool CheckIfLineHasUnendedQuote(char* line, int lineLength, int index)
{
    bool hasUnendedQuote = false;
    int i = index;
    int nextI, prevI;

    nextI = i + 1;

    if (i == 0 && nextI < lineLength && line[i] == CSV_QUOTATION && line[nextI] != CSV_QUOTATION)
    {
        hasUnendedQuote = true;
        ++index;
    }

    while(i < lineLength)
    {
        if (line[i] == '\n')
        {
            break;
        }

        if (line[i] != CSV_QUOTATION)
        {
            ++i;
            continue;
        }

        nextI = i + 1;
        prevI = i - 1;

        if(hasUnendedQuote)
        {
            if (line[nextI] != CSV_QUOTATION)
                hasUnendedQuote = false;
            else 
            {
                i += 2;
                continue;
            }
        }
        else if (prevI >= 0 && nextI < lineLength)
        {
            if(line[prevI] == CSV_DELIMITATOR && line[nextI] != CSV_QUOTATION)
            {
                hasUnendedQuote = true;
            }
        }

        ++i;
    }

    return hasUnendedQuote;
}

// Function to be called under an unended quote state (multi-line text column detected)
// Returns a struct with a 'result' boolean indicating if the ending quote was found, and 'lastLineIndex' indicating the index on the line where it was found.
struct endingQuoteResult CheckIfLineHasEndingQuote(char* line, int lineLength)
{
    struct endingQuoteResult hasEndingQuote;
    int i = 0;

    hasEndingQuote.lastLineIndex = 0;
    hasEndingQuote.result = false;

    while(i < lineLength)
    {
        int nextI;

        if (line[i] == '\n')
            break;

        if (line[i] != CSV_QUOTATION)
        {
            ++i;
            continue;
        }

        nextI = i + 1;

        if (nextI >= lineLength)
        {
            hasEndingQuote.lastLineIndex = i;
            hasEndingQuote.result = true;
            break;
        }

        if (line[nextI] == CSV_QUOTATION)
        {
            i += 2;
            continue;
        }

        if (line[nextI] != CSV_QUOTATION)
        {
            hasEndingQuote.lastLineIndex = i;
            hasEndingQuote.result = true;
            break;
        }

        ++i;
    }

    return hasEndingQuote;
}

// Function that calls fopen in the desired filepath and initializes the FILE* in context
void InitializeCustomMessageFile()
{
    char* systemLang = GetCurrentListLanguage();
    int pathLength = strlen(CSV_FILE_PATH) + strlen(systemLang) + strlen(CSV_FILE_NAME);
    char* filePath = calloc(pathLength + 1, sizeof(char));

    if (!filePath)
        return;

    strcpy(filePath, CSV_FILE_PATH);
    strcat(filePath, systemLang);
    strcat(filePath, CSV_FILE_NAME);

    _customMultiTextFile = fopen(filePath, "r");

    free(filePath);
}

//Function that calls fclose on the opened FILE* in context
void CloseCustomMessageFile()
{
    if (_customMultiTextFile)
    {
        fclose(_customMultiTextFile);
        _customMultiTextFile = NULL;
    }
}


char* fgets_dynamic(char **line, size_t *bufferSize, FILE *file)
{
    size_t length;
    int initialBufferSize = *bufferSize;
    char* c;

    if (!fgets(*line, *bufferSize, file))
        return NULL;

    length = strlen(*line);

    while(strchr(*line, '\n') == NULL) 
    {
        *bufferSize += initialBufferSize;
        *line = realloc(*line, *bufferSize * sizeof(char));

        if (!line)
            break;

        if(!fgets(*line + length, *bufferSize - length, file))
            break;

        length += strlen(*line + length);
    }

    return *line;
}

/* 
* Function to get text not defined in original assets
* Return Value: A string with the result for the given numParam. This string should be freed from memory by the caller after being used, by calling the <stdlib.h> function 'void free(void *ptr)'.
*
* Parameters:
*    numParam: the int key code to identify the string that needs to be fetched
*/
char* GetCustomizedMultiText(LONG numParam)
{
    char* returnValue = "";
    bool inUnendedQuoteState = false;
    size_t bufferSize = CSV_BUFFER_SIZE;
    char *line = calloc(bufferSize + 1, sizeof(char));

    InitializeCustomMessageFile();

    if (!line)
        return "";

    if (!_customMultiTextFile)
        return "";

    // Iterate every single line of the file
    while (!feof(_customMultiTextFile) && fgets_dynamic(OUT &line, OUT &bufferSize, _customMultiTextFile))
    {
        int index = 0;
        int lineLength = strlen(line);
        struct csvLongValue num;
        struct csvTextValue text;
        
        // On the off-chance a line is part of a multi-lined text column, and this line starts with the expected format (e.g. 10,Text), this line should be skipped in this flow. 
        // Without this check, text under multi-lined columns could be confused as values to be fetched and displayed by the program.
        // Multi-line text columns are already parsed in the treatSingleColumn function, in case a valid num is found.
        if (inUnendedQuoteState)
        {
            struct endingQuoteResult hasEndingQuote;

            hasEndingQuote = CheckIfLineHasEndingQuote(line, lineLength);

            if (hasEndingQuote.result)
                inUnendedQuoteState = CheckIfLineHasUnendedQuote(line, lineLength, hasEndingQuote.lastLineIndex + 1);

            continue;
        }

        inUnendedQuoteState = CheckIfLineHasUnendedQuote(line, lineLength, 0);
        
        num = readLong(line, lineLength, index);
        // skip line until a valid num is found
        if (num.value == CSV_INVALID_INT_VALUE || num.value != numParam)
            continue;

        index = num.lastLineIndex;

        // if for a given num there is no column next to it with text, don't continue and stop the loop.
        if (!isIndexDelimitator(line, lineLength, index))
            break;

        ++index;

        // get the text for a valid num and language
        text = readText(line, lineLength, index);
        if (text.value)
            returnValue = text.value;

        // at this point we've already found a num for a given language: break the loop
        break;
    }

    CloseCustomMessageFile();

    free(line);

    return returnValue;
}
