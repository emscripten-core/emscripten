// memdjpeg - A super simple example of how to decode a jpeg in memory
// Kenneth Finnegan, 2012
// blog.thelifeofkenneth.com
//
// After installing jpeglib, compile with:
// cc memdjpeg.c -ljpeg -o memdjpeg
//
// Run with:
// ./memdjpeg filename.jpg
//
// Version	   Date		Time		  By
// -------	----------	-----		---------
// 0.01		2012-07-09	11:18		Kenneth Finnegan
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <jpeglib.h>


int main (int argc, char *argv[]) {
	int rc, i, j;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s filename.jpg\n", argv[0]);
		exit(EXIT_FAILURE);
	}

//   SSS    EEEEEEE  TTTTTTT  U     U  PPPP
// SS   SS  E           T     U     U  P   PP
// S        E           T     U     U  P    PP
// SS       E           T     U     U  P   PP
//   SSS    EEEE        T     U     U  PPPP
//      SS  E           T     U     U  P
//       S  E           T     U     U  P
// SS   SS  E           T      U   U   P
//   SSS    EEEEEEE     T       UUU    P

	// Variables for the source jpg
	struct stat file_info;
	unsigned long jpg_size;
	unsigned char *jpg_buffer;

	// Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// Variables for the output buffer, and how long each row is
	unsigned long bmp_size;
	unsigned char *bmp_buffer;
	int row_stride, width, height, pixel_size;


	// Load the jpeg data from a file into a memory buffer for
	// the purpose of this demonstration.
	// Normally, if it's a file, you'd use jpeg_stdio_src, but just
	// imagine that this was instead being downloaded from the Internet
	// or otherwise not coming from disk
	rc = stat(argv[1], &file_info);
	if (rc) {
		fprintf(stderr, "FAILED to stat source jpg\n");
		exit(EXIT_FAILURE);
	}
	jpg_size = file_info.st_size;
	jpg_buffer = (unsigned char*) malloc(jpg_size + 100);

	int fd = open(argv[1], O_RDONLY);
	i = 0;
	while (i < jpg_size) {
		rc = read(fd, jpg_buffer + i, jpg_size - i);
		printf("Input: Read %d/%lu bytes\n", rc, jpg_size-i);
		i += rc;
	}
	close(fd);

//   SSS    TTTTTTT     A     RRRR     TTTTTTT
// SS   SS     T       A A    R   RR      T
// S           T      A   A   R    RR     T
// SS          T     A     A  R   RR      T
//   SSS       T     AAAAAAA  RRRR        T
//      SS     T     A     A  R RR        T
//       S     T     A     A  R   R       T
// SS   SS     T     A     A  R    R      T
//   SSS       T     A     A  R     R     T

	printf("Proc: Create Decompress struct\n");
	// Allocate a new decompress struct, with the default error handler.
	// The default error handler will exit() on pretty much any issue,
	// so it's likely you'll want to replace it or supplement it with
	// your own.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);


	printf("Proc: Set memory buffer as source\n");
	// Configure this decompressor to read its data from a memory
	// buffer starting at unsigned char *jpg_buffer, which is jpg_size
	// long, and which must contain a complete jpg already.
	//
	// If you need something fancier than this, you must write your
	// own data source manager, which shouldn't be too hard if you know
	// what it is you need it to do. See jpeg-8d/jdatasrc.c for the
	// implementation of the standard jpeg_mem_src and jpeg_stdio_src
	// managers as examples to work from.
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);


	printf("Proc: Read the JPEG header\n");
	// Have the decompressor scan the jpeg header. This won't populate
	// the cinfo struct output fields, but will indicate if the
	// jpeg is valid.
	rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1) {
		fprintf(stderr, "File does not seem to be a normal JPEG\n");
		exit(EXIT_FAILURE);
	}

	printf("Proc: Initiate JPEG decompression\n");
	// By calling jpeg_start_decompress, you populate cinfo
	// and can then allocate your output bitmap buffers for
	// each scanline.
	jpeg_start_decompress(&cinfo);

	width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;

	printf("Proc: Image is %d by %d with %d components\n",
			width, height, pixel_size);

	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*) malloc(bmp_size);

	// The row_stride is the total number of bytes it takes to store an
	// entire scanline (row).
	row_stride = width * pixel_size;


	printf("Proc: Start reading scanlines\n");
	//
	// Now that you have the decompressor entirely configured, it's time
	// to read out all of the scanlines of the jpeg.
	//
	// By default, scanlines will come out in RGBRGBRGB...  order,
	// but this can be changed by setting cinfo.out_color_space
	//
	// jpeg_read_scanlines takes an array of buffers, one for each scanline.
	// Even if you give it a complete set of buffers for the whole image,
	// it will only ever decompress a few lines at a time. For best
	// performance, you should pass it an array with cinfo.rec_outbuf_height
	// scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and
	// at the default high quality decompression setting is always 1.
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}
	printf("Proc: Done reading scanlines\n");


	// Once done reading *all* scanlines, release all internal buffers,
	// etc by calling jpeg_finish_decompress. This lets you go back and
	// reuse the same cinfo object with the same settings, if you
	// want to decompress several jpegs in a row.
	//
	// If you didn't read all the scanlines, but want to stop early,
	// you instead need to call jpeg_abort_decompress(&cinfo)
	jpeg_finish_decompress(&cinfo);

	// At this point, optionally go back and either load a new jpg into
	// the jpg_buffer, or define a new jpeg_mem_src, and then start
	// another decompress operation.

	// Once you're really really done, destroy the object to free everything
	jpeg_destroy_decompress(&cinfo);
	// And free the input buffer
	free(jpg_buffer);

// DDDD       OOO    N     N  EEEEEEE
// D  DDD    O   O   NN    N  E
// D    DD  O     O  N N   N  E
// D     D  O     O  N N   N  E
// D     D  O     O  N  N  N  EEEE
// D     D  O     O  N   N N  E
// D    DD  O     O  N   N N  E
// D  DDD    O   O   N    NN  E
// DDDD       OOO    N     N  EEEEEEE

	// Write the decompressed bitmap out to a ppm file, just to make sure
	// it worked.
	fd = open("output.ppm", O_CREAT | O_WRONLY, 0666);
	char buf[1024];

	rc = sprintf(buf, "P6 %d %d 255\n", width, height);
	write(fd, buf, rc); // Write the PPM image header before data
	write(fd, bmp_buffer, bmp_size); // Write out all RGB pixel data

	close(fd);
	free(bmp_buffer);

	printf("End of decompression\n");
	return EXIT_SUCCESS;
}
