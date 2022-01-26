# Introduction #

The files in this folder are intended to be used for adding localized text into Little Big Adventure's code that does not exist in the original assets of the game. 

Each language has a different file that can be identified by it's prefix (EN_ for English, FR_ for French, DE_ for German, SP_ for Spanish and IT_ for Italian). By default, the text language is named as ___'LANGPREFIX_ctxt.csv'___, although this can be changed through code. So, for English, we would have a file named ___'EN_ctxt.csv'___, for French ___'FR_ctxt.csv'___ and so on.

Each file follows the CSV (Comma-Separated Values) format, where essentially a table is represented with columns separated by field delimitators (in this case a comma ___','___ ). and text inside a column can be placed under quotes to allow special characters (in this case, the quote delimitator is ___"___ ). These delimitators can be customized to be anything else in the code, but ideally we should follow the standard CSV format.

Here's an example of a CSV file read in raw text format:

:: 

    234,Collision Damage On
    34,"Collision Damage Off"

This represents a table with two rows separated by two columns: the first column is a numeric value and the second a textual value. Note that the second line has a text column surrounded by quotes. This allows the addition of special characters such as quotes and commas that you want to be part of the text, and not used as part of delimitators of the CSV file. Additional examples for these cases will be provided further below.

<br/>
<br/>

# Getting Started

To use these localization files, copy them to the same folder where you have built *'LBA0.exe'*. By default, the *ONE_GAME_DIRECTORY* flag is set to *true* in the code. If the flag is set to *false*, copy the localization files to *'..\CommunityAssets\Text'*, on the folder where the built *'LBA0.exe'* executable is located. 


<br/>
<br/>

# Editor Programs

To edit a CSV file, you can either:
    
- Use a raw text editor, if you choose to add the delimitators manually, or 
- Use a spreadsheet editor (such as LibreOffice Calc or Microsoft Excel) if you prefer to have the program add the delimitators automatically.

>**IMPORTANT NOTE**: Whichever editor you choose, you must ensure that it can read and save files in the appropriate encoding format for LBA's engine. If you are unsure which to use, [LibreOffice Calc](https://www.libreoffice.org/) is available for free and seems to offer some of the best options for this scenario.  

<br/>
<br/>

# Encoding

Characters in LBA's engine follow the ___extended ASCII standard___, more specifically the ___Western Europe (DOS/OS2-437/US)___ character set. The list of available characters and their respective codes can be accessed [here](https://www.asciitable.com/). 

It is imperative that whenever you edit a CSV file, you add characters to it following this standard. Depending on the editor you use, you may have to use [ALT Codes](https://www.lookuptables.com/text/alt-codes) for certain additions such as special characters with accents (e.g. ___é___ ). Any character that does not belong in the ASCII table or is encoded incorrectly will have an erroneous display on the engine.

It is also mandatory that whenever you open or save a CSV file, the correct encoding is selected:

- in ___Microsoft Excel___ 
    - On save CSV: click Save As and select the file type to be ___'CSV (MS-DOS)'___, and then click *'Tools' > 'WebOptions'* to find an *'Encoding'* tab where you can set *'Save this document as'* value to *'EUA-ASCII'*. Once finished, save the document by pressing *OK*. Some ASCII characters tend to not be displayed correctly in Excel under this case, but even so, if you added them with the correct encoding and save options, they can be displayed correctly on the engine. LibreOffice Calc, however, appears to make things easier for this issue.

- in ___LibreOffice Calc___
    - On open CSV: a *Text Import* dialog (Img. 1) should appear. Under *'Import'*, set the *'Character Set:'* value to  ___Western Europe (DOS/OS2-437/US)___, and *'Language'* to ___'English (USA)'___.  Under *'Separator Options'*, verify that the *'Separated By'* radio button is selected, and of all the checkboxes beneath only *'Comma'* is selected. The *'String Delimiter'* value should be set to: ___"___ . 
    - On save CSV: choose *Save As* option and select the file type *Text CSV (*.csv)*. Check *Edit Filter Settings* and press *Save*. A *Confirm File Format* dialog may appear, select *Use Text CSV Format*. Then a *Export Text File* dialog (Img. 2) should appear. Under *Field Options*, set the *'Character Set:'* value to  ___Western Europe (DOS/OS2-437/US)___. Verify if the field and string delimiters are correct then press *OK*. Quick saves should use this format after this.


<br/>
<div style="text-align:center">
  <img width="40%" src="docs/fig1.PNG" alt="LibreOffice Text Import Dialog">
  <p>Img. 1: LibreOffice Text Import Dialog
</div>

<br/>
<div style="text-align:center">
  <img width="40%" src="docs/fig2.PNG" alt="LibreOffice Export Text File Dialog">
  <p>Img. 2: LibreOffice Text Import Dialog
</div>

<br/>
<br/>

# Guidelines

<br/>
<br/>

> ### **Adding a new text in the CSV for your mod** ###

<br/>

Create a new line in the CSV with two columns (the first for an integer to be used as a key parameter in the code, the second for the actual text). If the key is not an integer (e.g. contains characters not in 0 and 9 range), this will be ignored by the code.

The end result of your added line, in raw text form, should look like this:

:: 

    123,Example Text

In the code, it is possible to fetch the text of this line by calling the following function:

::

    char* GetCustomizedMultiText(int numParam);

Where if in this example you passed *123* as the numParam, the function would return the string *"Example Text"*.

The key integer value should be something you choose to identify the text you add, and it should be unique in the CSV (no repeating integers). Furthermore, it is also a good idea not to use values that are being used for texts already existing in LBA (a search for the number in 2point21's repository can share good results on whether this value is being used or not).

 <br/>
 <br/>
 <br/>
 <br/>

> ### **Adding special characters to a text such as commas and quote marks** ###

<br/>

If you're adding these to a column in a spreadsheet editor, once you save the file, they should be converted automatically to the correct format.

If you're writing in raw text, the column of the text must be wrapped in quotation marks (e.g. "text1, text2"). Any commas will be considered as part of the text itself and not as a CSV delimitator. If you want to add quotation marks as part of the text itself, these have to be double-quoted.

Example of a text with quotes:

::
    
    321,"Twinsen says ""Hello""."

Example of a text with commas:

:: 
    
    22,"Twinsen looks at you, and says ""Hello""."

 <br/>
 <br/>
 <br/>
 <br/>

> ### **Adding paragraphs to your text in the CSV** ###

<br/>

If you're using a spreadsheet editor, you can add a paragraph by editing the column itself. This is usually accomplished by entering a combination of keys, such as ALT+ENTER or CTRL+ENTER.

In raw text format, paragraphs should look like the following:

::

    111,"Example text
    with paragraphs,
    until the quote ends"

The result string of the function in code would be: "Example text\nwith paragraphs,\nuntil the quote ends".

 <br/>
 <br/>
 <br/>
 <br/>

> ### **When adding a certain character in the CSV, it does not appear correctly when displayed in the game** ###

<br/>

This usually happens when you add a character with accents on it (e.g. é, è, ê, ñ, â, à, ó, ...) or a greek letter or something of the sort. Some of these characters are supported, check if they exist and conform to the standard in the [ASCII Table](https://www.lookuptables.com/text/alt-codes). If they do exist but still appear funny, try using [ALT Codes](https://www.lookuptables.com/text/alt-codes) or, if you are using Windows, the Character Map app to insert these characters. 

Ultimately, the editor you are using could not be conforming to this standard. Try finding one that does. As a personal recommendation, [LibreOffice Calc](https://www.libreoffice.org/) offers good support for editing these files. Follow the instructions in the *Encoding* section of this file to have it working.

It's very important that your additions conform to these standards, especially if you're editing a CSV file that already has previously added characters from other community members with a very specific encoding in it. Having them in the wrong format could result in you having to review your Pull Requests to this repository before they get merged.
