#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384 // 16KB buffer size for reading/writing

//compress file to file.gz
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    // Open the input file for reading
    FILE* input_file = fopen(input_filename, "rb");
    if (!input_file) {
        printf("Failed to open input file '%s'\n", input_filename);
        return 1;
    }

    // Open the output file for writing
    FILE* output_file = fopen(output_filename, "wb");
    if (!output_file) {
        fclose(input_file);
        printf("Failed to open output file '%s'\n", output_filename);
        return 1;
    }

    // Initialize zlib stream for gzip compression
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    int ret = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        fclose(input_file);
        fclose(output_file);
        printf("Failed to initialize zlib compression stream (%d)\n", ret);
        return 1;
    }

    // Read input file and write compressed data to output file
    unsigned char in_buffer[CHUNK_SIZE];
    unsigned char out_buffer[CHUNK_SIZE];
    stream.next_out = out_buffer;
    stream.avail_out = CHUNK_SIZE;

    int flush;
    do {
        stream.next_in = in_buffer;
        stream.avail_in = fread(in_buffer, 1, CHUNK_SIZE, input_file);

        if (ferror(input_file)) {
            printf("Error reading from input file '%s'\n", input_filename);
            deflateEnd(&stream);
            fclose(input_file);
            fclose(output_file);
            return 1;
        }

        flush = feof(input_file) ? Z_FINISH : Z_NO_FLUSH;

        do {
            ret = deflate(&stream, flush);
            if (ret == Z_STREAM_ERROR) {
                printf("Zlib compression error (%d)\n", ret);
                deflateEnd(&stream);
                fclose(input_file);
                fclose(output_file);
                return 1;
            }

            size_t have = CHUNK_SIZE - stream.avail_out;
            if (fwrite(out_buffer, 1, have, output_file) != have || ferror(output_file)) {
                printf("Error writing compressed data to output file '%s'\n", output_filename);
                deflateEnd(&stream);
                fclose(input_file);
                fclose(output_file);
                return 1;
            }

            stream.next_out = out_buffer;
            stream.avail_out = CHUNK_SIZE;
        } while (stream.avail_out == 0);

    } while (flush != Z_FINISH);

    // Clean up
    deflateEnd(&stream);
    fclose(input_file);
    fclose(output_file);

    return 0;
}
