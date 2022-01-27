#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "c_extern.h"

#ifdef ONE_GAME_DIRECTORY
#define CSV_FILE_PATH ""
#else
#define CSV_FILE_PATH "CommunityAssets\\Text\\"
#endif

#define CSV_FILE_NAME "ctxt.csv"
#define CSV_DELIMITATOR ','
#define CSV_QUOTATION '"'
#define CSV_BUFFER_SIZE 1024
#define CSV_TEXT_BUFFER_MULTIPLER 10
#define CSV_INVALID_INT_VALUE -1
#define CSV_INVALID_TEXT_VALUE ""

#define ASCII_MIN_INT 48
#define ASCII_MAX_INT 57

FILE* _customMultiTextFile; 

// struct returned upon calling readInt function
struct csvIntValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where an integer column ended
    int value;
};

// struct returned upon calling readText function
struct csvTextValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where a text column ended
    char* value;
};

// struct returned upon calling treatSingleLine function
struct treatSingleLineResult
{
    int lastLineIndex; // helper variable to indicate the index of the line where a text column ended (taking into consideration a multi-line column case)
    int treatedColLength;
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
bool hasQuotationMarks(char *line, int index)
{
    return line[index] == CSV_QUOTATION;
}

bool isIndexDelimitator(char *line, int index)
{
    return line[index] == CSV_DELIMITATOR;
}

/*
* Returns the length of the parsed line after treatment of CSV formatting. The parameter char* treatedColumn is populated with the characters to be later printed.
* 
* Parameters:
*   line: the current line being read from the file
*   index: the index value to start reading in the line
*   treatedColumn: the OUT parameter in which the necessary text characters are added to
*   treatedColIndex: the last index that was added to treatedColumn (technically it represents it's length). Used as an auxiliary for recursive calls.
*   lineWithQuotes: boolean indicating if this line has quotes (1 or 0)
*/
struct treatSingleLineResult treatSingleLine(char* line, int index, char* treatedColumn, int treatedColIndex, bool lineWithQuotes)
{
    struct treatSingleLineResult nextLineResult;
    int i = index;
    int initialTreatedColIndex = treatedColIndex;
    bool saveNextLineLastIndex = false;

    //Iterate through each character of the line, starting from the index where the text column starts
    while (line && i < CSV_BUFFER_SIZE)
    {
        //Treat text in case line does not start with quotes
        if (!lineWithQuotes)
        {
            bool isColumnEnd = line[i] == '\n' || isIndexDelimitator(line, i);
            // If current character is a delimitator, then this text has reached it's end. 
            if (isColumnEnd)
                break;
            // Else append character to treatedColumn
            else
            {
                treatedColumn[treatedColIndex++] = line[i];
                ++i;
            }
        }
        //Else treat text as being wrapped in quotes
        else
        {   
            //If current character is a quotation mark, evaluate what to do next
            if (line[i] == CSV_QUOTATION)
            {
                int nextI = i + 1;

                //If the next character is a quotation mark (double quotations), treat it as a quote mark part of the text to be printed
                if (nextI < CSV_BUFFER_SIZE && line[nextI] == CSV_QUOTATION)
                {
                    treatedColumn[treatedColIndex++] = line[nextI];
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
                // Add a new line '\n' to treatedColumn
                treatedColumn[treatedColIndex++] = '\n';

                if (!_customMultiTextFile)
                    break;

                 if (feof(_customMultiTextFile))
                    break;

                // Get the next line from the file
                fgets(line, CSV_BUFFER_SIZE, _customMultiTextFile);

                if (!line)
                    break;

                // Recursively call this function to treat the next line, and keep appending characters to treatedColumn on current treatedColIndex and onwards
                nextLineResult = treatSingleLine(line, 0, treatedColumn, treatedColIndex, lineWithQuotes);

                // Add the length of the new treated line to treatedColIndex
                treatedColIndex += nextLineResult.treatedColLength;

                saveNextLineLastIndex = true;
                break;
            }
            //If the character doesn't match the previous conditions, simply append it to treatedColumn (including delimitators)
            else
            {
                treatedColumn[treatedColIndex++] = line[i];
                ++i;
            }
        }
    }

    // finish string with '\0' to indicate the end of the text and avoid printing garbage characters
    treatedColumn[treatedColIndex++] = '\0';

    nextLineResult.lastLineIndex = saveNextLineLastIndex ? nextLineResult.lastLineIndex : i;
    nextLineResult.treatedColLength = treatedColIndex - initialTreatedColIndex;

    // return length of char* treated and parsed in a single line
    return nextLineResult;
}

// Function that parses a text column
struct csvTextValue readText(char* line, int index)
{
    // Set initial values to invalid incase the parsing fails
    struct csvTextValue returnValue = { CSV_INVALID_INT_VALUE, CSV_INVALID_TEXT_VALUE };

    bool lineWithQuotes = hasQuotationMarks(line, index);
    int i = index + lineWithQuotes;

    // Buffer size increased by multiplier to allow text in multiple lines
    char treatedColumn[CSV_BUFFER_SIZE * CSV_TEXT_BUFFER_MULTIPLER];
    
    struct treatSingleLineResult treatedTextResult = treatSingleLine(line, i, treatedColumn, 0, lineWithQuotes);

    // Allocate the exact length in memory for the treated text
    returnValue.lastLineIndex = treatedTextResult.lastLineIndex;
    returnValue.value = malloc(treatedTextResult.treatedColLength * sizeof(char));

    strncpy(returnValue.value, treatedColumn, treatedTextResult.treatedColLength);

    return returnValue;
}

// Function that parses an integer column
struct csvIntValue readInt(char* line, int index)
{
    // Set initial values to invalid incase the parsing fails
    struct csvIntValue returnValue = { CSV_INVALID_INT_VALUE, CSV_INVALID_INT_VALUE };
    
    char* column;
    int columnLen = 0;
    
    int i = index;

    // Iterate through each character
    while(i < CSV_BUFFER_SIZE)
    {
        // Check if the character is in the ASCII range for integers
        bool isIndexValidNumber = line[i] >= ASCII_MIN_INT && line[i] <= ASCII_MAX_INT;

        // If the current character is a delimitator, we have reached the end of this column
        if (isIndexDelimitator(line, i))
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
        // copy the part of the line that corresponds to the integer
        column = malloc(columnLen * sizeof(char) + 1);
        strncpy(column, line, columnLen);
        column[columnLen] = '\0';

        returnValue.lastLineIndex = i;
        //Convert ascii to integer
        returnValue.value = atoi(column);
    }

    return returnValue;
}

// Function that checks if a line has a csv quotation mark that is unended (indicating that next lines in the file are to be treated as part of the column)
// Returns 1 if there is an unended quote, 0 if not.
bool CheckIfLineHasUnendedQuote(char* line, int index)
{
    bool hasUnendedQuote = false;
    int i = index;
    int nextI, prevI;

    nextI = i + 1;

    if (i == 0 && nextI < CSV_BUFFER_SIZE && line[i] == CSV_QUOTATION && line[nextI] != CSV_QUOTATION)
    {
        hasUnendedQuote = true;
        ++index;
    }

    while(i < CSV_BUFFER_SIZE)
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
        else if (prevI >= 0 && nextI < CSV_BUFFER_SIZE)
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
struct endingQuoteResult CheckIfLineHasEndingQuote(char* line)
{
    struct endingQuoteResult hasEndingQuote;
    int i = 0;

    hasEndingQuote.lastLineIndex = 0;
    hasEndingQuote.result = false;

    while(i < CSV_BUFFER_SIZE)
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

        if (nextI >= CSV_BUFFER_SIZE)
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
    char filePath[256];
    char* systemLang = GetCurrentListLanguage();

    strcpy(filePath, CSV_FILE_PATH);
    strcat(filePath, systemLang);
    strcat(filePath, CSV_FILE_NAME);

    _customMultiTextFile = fopen(filePath, "r");
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

// Function to get text not defined in TEXT.HQR
char* GetCustomizedMultiText(int numParam)
{
    char* returnValue = "";
    char line[CSV_BUFFER_SIZE];
    bool inUnendedQuoteState = false;

    InitializeCustomMessageFile();

    if (!_customMultiTextFile)
        return "";

    // Iterate every single line of the file
    while (!feof(_customMultiTextFile) && fgets(line, CSV_BUFFER_SIZE, _customMultiTextFile))
    {
        int index = 0;
        struct csvIntValue num;
        struct csvTextValue text;
        
        // On the off-chance a line is part of a multi-line text column, and this line starts with the expected format (e.g. 10,Text), this line should be skipped in this flow. 
        // Without this check, text under multi-line columns could be confused as values to be fetched and displayed by the program.
        // Multi-line text columns are already parsed in the treatSingleLine function, in case a valid num is found.
        if (inUnendedQuoteState)
        {
            struct endingQuoteResult hasEndingQuote;

            hasEndingQuote = CheckIfLineHasEndingQuote(line);

            if (hasEndingQuote.result)
                inUnendedQuoteState = CheckIfLineHasUnendedQuote(line, hasEndingQuote.lastLineIndex + 1);

            continue;
        }

        inUnendedQuoteState = CheckIfLineHasUnendedQuote(line, 0);
        
        num = readInt(line, index);
        // skip line until a valid num is found
        if (num.value == CSV_INVALID_INT_VALUE || num.value != numParam)
            continue;

        index = num.lastLineIndex;

        // if for a given num there is no column next to it with text, don't continue and stop the loop.
        if (!isIndexDelimitator(line, index))
            break;

        ++index;

        // get the text for a valid num and language
        text = readText(line, index);
        if (text.value)
            returnValue = text.value;

        // at this point we've already found a num for a given language: break the loop
        break;
    }

    CloseCustomMessageFile();

    return returnValue;
}
