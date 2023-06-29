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

#define ENCODED_ASSEMBLY_FILE_COUNT 4

static const char* encoded_assembly_files[ENCODED_ASSEMBLY_FILE_COUNT] = {
    "./test_files/listing_0037_single_register_mov",
    "./test_files/listing_0038_many_register_mov",
    "./test_files/listing_0039_more_movs",
    "./test_files/listing_0039_more_movs",
};

int main(int argc, char** argv)
{
    /* Decode all files */
    for (uint8_t i = 0; i < ENCODED_ASSEMBLY_FILE_COUNT; i++)
    {
        /* Print current file */
        printf("Decoding '%s'\n", encoded_assembly_files[i]);

        /* Read file */
        FILE* file = fopen(encoded_assembly_files[i], "rb");
        if (file == NULL)
        {
            printf("\t[FILE] Failed to open file '%s'\n", encoded_assembly_files[i]);
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
            printf("\t[FILE] Failed to open file 'tmp.txt'\n");
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
            printf("\t[FILE] Failed to open file 'tmp'\n");
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
            printf("\t[COMPARE] Original size (%u) and result size (%u) aren't equal\n", file_size_original, file_size_result);
            return;
        }
        if (memcmp(file_data_original, file_data_result, file_size_result) != 0)
        {
            printf("\t[COMPARE] Content of original file and result file aren't the same\n");
            return;
        }
        printf("\t[COMPARE] Original and result file are equal\n\n");
    }

    return 0;
}
