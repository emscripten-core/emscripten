#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int MOD_ADLER = 65521;
 
uint64_t adler32(unsigned char *data, int32_t len) /* where data is the location of the data in physical memory and 
                                                       len is the length of the data in bytes */
{
    uint64_t a = 1, b = 0;
    int32_t index;
 
    /* Process each byte of the data in order */
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
 
    return (b << 16) | a;
}

int main(int argc, char* argv[]) {
    long bufsize;

    if (argc != 2) {
	fputs("Need 1 argument\n", stderr);
	return (EXIT_FAILURE);
    }

    unsigned char *source = NULL;
    FILE *fp = fopen(argv[1], "rb");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            bufsize = ftell(fp);
            if (bufsize == -1) { fputs("Couldn't get size\n", stderr); return (EXIT_FAILURE); }
    
            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));
            if (source == NULL) { fputs("Couldn't allocate\n", stderr); return (EXIT_FAILURE); }
    
            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) == -1) { fputs("Couldn't seek\n", stderr); return (EXIT_FAILURE); }
    
            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0) {
                fputs("Error reading file\n", stderr);
                //return (EXIT_FAILURE);
            } else {
                source[++newLen] = '\0'; /* Just to be safe. */
            }
        } else {
          fputs("Couldn't seek to end\n", stderr);
          return (EXIT_FAILURE);
        }
        fclose(fp);
    } else {
      fputs("Couldn't open\n", stderr);
      return (EXIT_FAILURE);
    }

    printf("%d\n", (uint32_t) adler32(source, bufsize));
    
    free(source); /* Don't forget to call free() later! */

    return (EXIT_SUCCESS);

}
