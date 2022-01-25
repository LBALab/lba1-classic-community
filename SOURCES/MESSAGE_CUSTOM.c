#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_extern.h"

#ifdef ONE_GAME_DIRECTORY
#define CSV_FILE_PATH ""
#else
#define CSV_FILE_PATH "CommunityAssets\\Text\\"
#endif

#define CSV_FILE_NAME "cust_txt.csv"
#define CSV_LANG_TOKEN "lang"
#define CSV_LANG_DELIMITATOR ":"
#define CSV_DELIMITATOR ','
#define CSV_QUOTATION '"'
#define CSV_BUFFER_SIZE 1024
#define CSV_TEXT_BUFFER_MULTIPLER 10
#define CSV_INVALID_INT_VALUE -1
#define CSV_INVALID_TEXT_VALUE ""

#define ASCII_MIN_INT 48
#define ASCII_MAX_INT 57

FILE* _customMultiTextFile; 

struct csvIntValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where an integer column ended
    int value;
};

struct csvTextValue
{
    int lastLineIndex; // helper variable to indicate the index of the line where a text column ended
    char* value;
};

struct treatSingleLineResult
{
    int lastLineIndex;
    int treatedColLength;
};

/* 
* In CSV, if a column starts with a quotation mark, it must end with a quotation mark. 
* Everything will be assimilated as part of the text until a final quotation mark is found. 
* If this final quotation mark is not found, there must be an error in the CSV formatting.
*/
short hasQuotationMarks(char *line, int index)
{
    return line[index] == CSV_QUOTATION;
}

short isIndexDelimitator(char *line, int index)
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
struct treatSingleLineResult treatSingleLine(char* line, int index, char* treatedColumn, int treatedColIndex, short lineWithQuotes)
{
    struct treatSingleLineResult nextLineResult;
    int i = index;
    int initialTreatedColIndex = treatedColIndex;
    short saveNextLineLastIndex = 0;

    //Iterate through each character of the line, starting from the index where the text column starts
    while (i < CSV_BUFFER_SIZE)
    {
        //Treat text in case line does not start with quotes
        if (!lineWithQuotes)
        {
            short isColumnEnd = line[i] == '\n' || isIndexDelimitator(line, i);
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
                if (!_customMultiTextFile)
                    break;

                // Get the next line from the file
                fgets(line, CSV_BUFFER_SIZE, _customMultiTextFile);

                if (!line)
                    break;

                // Add a new line '\n' to treatedColumn
                treatedColumn[treatedColIndex++] = '\n';

                // Recursively call this function to treat the next line, and keep appending characters to treatedColumn on current treatedColIndex and onwards
                nextLineResult = treatSingleLine(line, 0, treatedColumn, treatedColIndex, lineWithQuotes);

                // Add the length of the new treated line to treatedColIndex
                treatedColIndex += nextLineResult.treatedColLength;

                saveNextLineLastIndex = 1;
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

struct csvTextValue readText(char* line, int index)
{
    // Set initial values to invalid incase the parsing fails
    struct csvTextValue returnValue = { CSV_INVALID_INT_VALUE, CSV_INVALID_TEXT_VALUE };

    short lineWithQuotes = hasQuotationMarks(line, index);
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
        short isIndexValidNumber = line[i] >= ASCII_MIN_INT && line[i] <= ASCII_MAX_INT;

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

void InitializeCustomMessageFile()
{
    int pathLen = strlen(CSV_FILE_PATH) + strlen(CSV_FILE_NAME);
    char *filePath = malloc(pathLen * sizeof(char));

    strcpy(filePath, CSV_FILE_PATH);
    strcat(filePath, CSV_FILE_NAME);

    _customMultiTextFile = fopen(filePath, "r");
}

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
    char* systemLang;
    char line[CSV_BUFFER_SIZE];
    short hasFoundLangInFile = 0;

    InitializeCustomMessageFile();

    if (!_customMultiTextFile)
        return "";

    systemLang = GetCurrentListLanguage();

    // Iterate every single line of the file
    while (fgets(line, CSV_BUFFER_SIZE, _customMultiTextFile))
    {
        int index;
        char* langTokenInFile;
        struct csvIntValue num;
        struct csvIntValue size;
        struct csvTextValue text;
        struct csvTextValue langColumnInFile;
        short isCurrentLineLangToken;

        langColumnInFile = readText(line, 0);
        langTokenInFile = strtok(langColumnInFile.value, CSV_LANG_DELIMITATOR);

        isCurrentLineLangToken = langTokenInFile && strcmp(langTokenInFile, CSV_LANG_TOKEN) == 0;

        // If the current line represents a language token
        if (isCurrentLineLangToken)
        {
            //If we haven't found the system language in the file yet, check if current token represents it
            if (!hasFoundLangInFile)
            {   
                langTokenInFile = strtok(NULL, CSV_LANG_DELIMITATOR);

                if (langTokenInFile)
                {
                    if (strcmp(langTokenInFile, systemLang) != 0)
                    {
                        hasFoundLangInFile = 0;
                    }
                    else 
                    {
                        hasFoundLangInFile = 1;
                    }
                }
                
                //Continue the loop until the language of the system is found
                continue;
            }
            //If we have found the system language previously, but reach another language token, break the loop (the num we were looking for was not found for the system language)
            else
            {
                break;
            }
        }

        // Do not look for a num until the language has been found
        if (!hasFoundLangInFile)
            continue;

        index = 0;
        
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
