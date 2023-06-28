/*
Copyright (c) 2023 Daniel Fedai Larsen

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "decoder.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    /* Get file to process */
    if (argc != 2)
    {
        printf("[ARGS] Invalid number of arguments (%i)\n", argc);
        return -1;
    }
    const char* file_name = argv[1];

    /* Read file */
    FILE* file = fopen(file_name, "rb");
    if (file == NULL)
    {
        printf("[FILE] Failed to open file '%s'\n", file_name);
        return -1;
    }
    fseek(file, 0, SEEK_END);
    uint32_t file_size_original = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* file_data_original = malloc(file_size_original);
    fread(file_data_original, file_size_original, 1, file);
    fclose(file);

    /* Open file to write decoded stream into */
    file = fopen("tmp.asm", "w");
    if (file == NULL)
    {
        printf("[FILE] Failed to open file 'tmp.txt'\n");
        return -1;
    }

    /* Decode instructions */
    decoder_decode_stream(file_data_original, file_size_original, file);

    /* Close file */
    fflush(file);
    fclose(file);

    /* Encode output assembly file */
    system("nasm tmp.asm");

    /* Read in encoded data */
    file = fopen("tmp", "rb");
    if (file == NULL)
    {
        printf("[FILE] Failed to open file 'tmp'\n");
        return -1;
    }
    fseek(file, 0, SEEK_END);
    uint32_t file_size_result = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* file_data_result = malloc(file_size_result);
    fread(file_data_result, file_size_result, 1, file);
    fclose(file);

    /* Compare original and new result */
    if (file_size_original != file_size_result)
    {
        printf("[COMPARE] Original size (%u) and result size (%u) aren't equal\n", file_size_original, file_size_result);
        return;
    }
    if (memcmp(file_data_original, file_data_result, file_size_result) != 0)
    {
        printf("[COMPARE] Content of original file and result file aren't the same\n");
        return;
    }
    printf("[COMPARE] Original and result file are equal\n");

    return 0;
}
