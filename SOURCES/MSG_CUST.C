#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "C_EXTERN.H"

#ifdef ONE_GAME_DIRECTORY
#define CSV_FILE_PATH ""
#else
#define CSV_FILE_PATH "assets//text//"
#endif

#define CSV_FILE_NAME "ctxt.csv"
#define CSV_DELIMITATOR ','
#define CSV_QUOTATION '"'
#define CSV_BUFFER_SIZE 1024
#define CSV_INVALID_INT_VALUE -1
#define CSV_INVALID_TEXT_VALUE ""

#define LBA_ENGINE_LINE_BREAK " @ "

#define ASCII_MIN_INT 48
#define ASCII_MAX_INT 57

#define OUT

FILE *_customMultiTextFile;

struct customMessageEntry
{
    LONG id;
    char *text;
};

static struct customMessageEntry *_messageCache = NULL;
static int _messageCacheSize = 0;
static char *_cachedLanguage = NULL;
static bool _cacheLoaded = false;

char *fgets_dynamic(char **line, size_t *bufferSize, FILE *file);

struct csvLongValue
{
    int lastLineIndex;
    LONG value;
};

struct csvTextValue
{
    int lastLineIndex;
    char *value;
};

struct treatSingleColumnResult
{
    int lastLineIndex;
    int treatedColLength;
    char *value;
};

struct endingQuoteResult
{
    int lastLineIndex;
    bool result;
};

bool hasQuotationMarks(char *line, int lineLength, int index)
{
    return index < lineLength ? line[index] == CSV_QUOTATION : false;
}

bool isIndexDelimitator(char *line, int lineLength, int index)
{
    return index < lineLength ? line[index] == CSV_DELIMITATOR : false;
}

struct treatSingleColumnResult *createSingleColumnResult(int lineLength)
{
    struct treatSingleColumnResult *result = calloc(1, sizeof(struct treatSingleColumnResult));

    if (!result)
        return NULL;

    result->lastLineIndex = CSV_INVALID_INT_VALUE;
    result->treatedColLength = CSV_INVALID_INT_VALUE;
    result->value = calloc(lineLength + 1, sizeof(char));

    if (!result->value)
        return NULL;

    return result;
}

struct treatSingleColumnResult *treatSingleColumnWithoutQuotes(char *line, int lineLength, int index)
{
    struct treatSingleColumnResult *result = createSingleColumnResult(lineLength);
    int i = index;
    int treatedColIndex = 0;

    if (!result)
        return NULL;

    if (!line)
        return result;

    while (i < lineLength && treatedColIndex < lineLength)
    {
        if (line[i] == '\0')
        {
            ++i;
            continue;
        }

        if (line[i] == '\n' || isIndexDelimitator(line, lineLength, i))
            break;
        else
            result->value[treatedColIndex++] = line[i];

        ++i;
    }

    result->lastLineIndex = i;
    result->treatedColLength = treatedColIndex;

    return result;
}

struct treatSingleColumnResult *treatSingleColumnWithQuotes(char *line, int lineLength, int index)
{
    struct treatSingleColumnResult *result = createSingleColumnResult(lineLength);
    struct treatSingleColumnResult *nextLineResult = NULL;
    int i = index;
    int treatedColIndex = 0;

    if (!result)
        return NULL;

    if (!line)
        return result;

    while (i < lineLength && treatedColIndex < lineLength)
    {
        if (line[i] == '\0')
        {
            ++i;
            continue;
        }

        if (line[i] == CSV_QUOTATION)
        {
            int nextI = i + 1;

            if (nextI < lineLength && line[nextI] == CSV_QUOTATION)
            {
                result->value[treatedColIndex++] = line[nextI];
                i += 2;
            }
            else
            {
                ++i;
                break;
            }
        }
        else if (line[i] == '\n')
        {
            size_t bufferSize = CSV_BUFFER_SIZE;
            char *tempValue;

            if (!_customMultiTextFile)
                break;

            if (feof(_customMultiTextFile))
                break;

            fgets_dynamic(OUT & line, OUT & bufferSize, _customMultiTextFile);

            if (!line)
                break;

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
                    strcat(result->value, LBA_ENGINE_LINE_BREAK);
                    strcat(result->value, nextLineResult->value);
                    treatedColIndex = tempIndex;
                }
            }

            break;
        }
        else
        {
            result->value[treatedColIndex++] = line[i];
            ++i;
        }
    }

    result->lastLineIndex = nextLineResult && nextLineResult->lastLineIndex != CSV_INVALID_INT_VALUE ? nextLineResult->lastLineIndex : i;
    result->treatedColLength = treatedColIndex;

    if (nextLineResult)
    {
        free(nextLineResult->value);
        free(nextLineResult);
    }

    return result;
}

struct csvTextValue readText(char *line, int lineLength, int index)
{
    struct csvTextValue returnValue = {CSV_INVALID_INT_VALUE, CSV_INVALID_TEXT_VALUE};

    bool lineWithQuotes = hasQuotationMarks(line, lineLength, index);
    int i = index + lineWithQuotes;

    struct treatSingleColumnResult *treatedTextResult;

    if (lineWithQuotes)
        treatedTextResult = treatSingleColumnWithQuotes(line, lineLength, i);
    else
        treatedTextResult = treatSingleColumnWithoutQuotes(line, lineLength, i);

    if (treatedTextResult)
    {
        returnValue.lastLineIndex = treatedTextResult->lastLineIndex;

        if (treatedTextResult->value)
        {
            returnValue.value = calloc(strlen(treatedTextResult->value) + 1, sizeof(char));

            strcpy(returnValue.value, treatedTextResult->value);
        }

        free(treatedTextResult->value);
        free(treatedTextResult);
    }

    return returnValue;
}

struct csvLongValue readLong(char *line, int lineLength, int index)
{
    struct csvLongValue returnValue = {CSV_INVALID_INT_VALUE, CSV_INVALID_INT_VALUE};

    char *column;
    int columnLen = 0;

    int i = index;

    while (i < lineLength)
    {
        bool isIndexValidNumber = line[i] >= ASCII_MIN_INT && line[i] <= ASCII_MAX_INT;

        if (isIndexDelimitator(line, lineLength, i))
            break;
        else if (!isIndexValidNumber)
        {
            columnLen = 0;
            break;
        }

        ++columnLen;
        ++i;
    }

    if (columnLen > 0)
    {
        column = calloc(columnLen + 1, sizeof(char));

        if (column)
        {
            strncpy(column, line, columnLen);
            returnValue.lastLineIndex = i;
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

bool CheckIfLineHasUnendedQuote(char *line, int lineLength, int index)
{
    bool hasUnendedQuote = false;
    int i = index;
    int nextI, prevI;

    nextI = i + 1;

    if (i == 0 && nextI < lineLength && line[i] == CSV_QUOTATION && line[nextI] != CSV_QUOTATION)
    {
        hasUnendedQuote = true;
        ++i;
    }

    while (i < lineLength)
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

        if (hasUnendedQuote)
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
            if (line[prevI] == CSV_DELIMITATOR && line[nextI] != CSV_QUOTATION)
            {
                hasUnendedQuote = true;
            }
        }

        ++i;
    }

    return hasUnendedQuote;
}

struct endingQuoteResult CheckIfLineHasEndingQuote(char *line, int lineLength)
{
    struct endingQuoteResult hasEndingQuote;
    int i = 0;

    hasEndingQuote.lastLineIndex = 0;
    hasEndingQuote.result = false;

    while (i < lineLength)
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

void InitializeCustomMessageFile()
{
    char *systemLang = GetCurrentListLanguage();
    int pathLength = strlen(CSV_FILE_PATH) + strlen(systemLang) + strlen(CSV_FILE_NAME);
    char *filePath = calloc(pathLength + 1, sizeof(char));

    if (!filePath)
        return;

    strcpy(filePath, CSV_FILE_PATH);
    strcat(filePath, systemLang);
    strcat(filePath, CSV_FILE_NAME);

    _customMultiTextFile = fopen(filePath, "r");

    free(filePath);
}

void CloseCustomMessageFile()
{
    if (_customMultiTextFile)
    {
        fclose(_customMultiTextFile);
        _customMultiTextFile = NULL;
    }
}

char *fgets_dynamic(char **line, size_t *bufferSize, FILE *file)
{
    size_t length;
    int initialBufferSize = *bufferSize;
    char *c;

    if (!fgets(*line, *bufferSize, file))
        return NULL;

    length = strlen(*line);

    while (strchr(*line, '\n') == NULL)
    {
        *bufferSize += initialBufferSize;
        *line = realloc(*line, *bufferSize * sizeof(char));

        if (!line)
            break;

        if (!fgets(*line + length, *bufferSize - length, file))
            break;

        length += strlen(*line + length);
    }

    return *line;
}

void ClearCustomMessageCache()
{
    int i;

    if (_messageCache)
    {
        for (i = 0; i < _messageCacheSize; i++)
        {
            if (_messageCache[i].text)
            {
                free(_messageCache[i].text);
                _messageCache[i].text = NULL;
            }
        }
        free(_messageCache);
        _messageCache = NULL;
    }

    if (_cachedLanguage)
    {
        free(_cachedLanguage);
        _cachedLanguage = NULL;
    }

    _messageCacheSize = 0;
    _cacheLoaded = false;
}

void LoadCustomMessagesIntoCache()
{
    size_t bufferSize = CSV_BUFFER_SIZE;
    char *line = calloc(bufferSize + 1, sizeof(char));
    bool inUnendedQuoteState = false;
    int capacity = 16;
    int count = 0;

    ClearCustomMessageCache();

    InitializeCustomMessageFile();

    if (!line || !_customMultiTextFile)
    {
        if (line)
            free(line);
        return;
    }

    _messageCache = calloc(capacity, sizeof(struct customMessageEntry));
    if (!_messageCache)
    {
        free(line);
        CloseCustomMessageFile();
        return;
    }

    _cachedLanguage = calloc(strlen(GetCurrentListLanguage()) + 1, sizeof(char));
    if (_cachedLanguage)
        strcpy(_cachedLanguage, GetCurrentListLanguage());

    while (!feof(_customMultiTextFile) && fgets_dynamic(OUT & line, OUT & bufferSize, _customMultiTextFile))
    {
        int index = 0;
        int lineLength = strlen(line);
        struct csvLongValue num;
        struct csvTextValue text;

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
        if (num.value == CSV_INVALID_INT_VALUE)
            continue;

        index = num.lastLineIndex;

        if (!isIndexDelimitator(line, lineLength, index))
            continue;

        ++index;

        text = readText(line, lineLength, index);
        if (!text.value)
            continue;

        if (count >= capacity)
        {
            struct customMessageEntry *newCache;
            capacity *= 2;
            newCache = realloc(_messageCache, capacity * sizeof(struct customMessageEntry));
            if (!newCache)
            {
                free(text.value);
                break;
            }
            _messageCache = newCache;
        }

        _messageCache[count].id = num.value;
        _messageCache[count].text = text.value;
        count++;
    }

    _messageCacheSize = count;
    _cacheLoaded = true;

    CloseCustomMessageFile();
    free(line);
}

char *LookupCachedMessage(LONG numParam)
{
    int i;

    for (i = 0; i < _messageCacheSize; i++)
    {
        if (_messageCache[i].id == numParam)
        {
            char *result = calloc(strlen(_messageCache[i].text) + 1, sizeof(char));
            if (result)
                strcpy(result, _messageCache[i].text);
            return result;
        }
    }

    return NULL;
}

/*
 * Function to get text not defined in original assets
 * Return Value: A string with the result for the given numParam. This string should be freed from memory by the caller after being used, by calling the <stdlib.h> function 'void free(void *ptr)'.
 *
 * Parameters:
 *    numParam: the int key code to identify the string that needs to be fetched
 */
char *GetCustomizedMultiText(LONG numParam)
{
    char *currentLanguage = GetCurrentListLanguage();
    char *result;

    if (!_cacheLoaded || !_cachedLanguage || strcmp(_cachedLanguage, currentLanguage) != 0)
    {
        LoadCustomMessagesIntoCache();
    }

    result = LookupCachedMessage(numParam);

    return result ? result : "";
}
