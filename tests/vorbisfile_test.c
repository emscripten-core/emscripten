/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: simple example decoder using vorbisfile

 ********************************************************************/

/* Takes a vorbis bitstream from stdin and writes raw stereo PCM to
   stdout using vorbisfile. Using vorbisfile is much simpler than
   dealing with libvorbis. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

char pcmout[4096]; /* take 4k out of the data segment, not the stack */

int main(int argc, char** argv) {
  OggVorbis_File vf;
  int eof=0;
  int current_section;
  FILE* input = fopen(argv[1], "rb");

  if (input == NULL) {
      fprintf(stderr,"Cannot open input file %s.\n", argv[1]);
      exit(1);
  }

  if(ov_open_callbacks(input, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      exit(1);
  }

  /* Throw the comments plus a few lines about the bitstream we're
     decoding */
  {
    char **ptr=ov_comment(&vf,-1)->user_comments;
    vorbis_info *vi=ov_info(&vf,-1);
    while(*ptr) {
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    printf("\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    printf("\nDecoded length: %ld samples\n", (long)ov_pcm_total(&vf,-1));
    printf("Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
  }

  while(!eof) {
    long ret=ov_read(&vf,pcmout,sizeof(pcmout),0,2,1,&current_section);
    if (ret == 0) {
      /* EOF */
      eof=1;
    } else if (ret < 0) {
      if(ret==OV_EBADLINK) {
        fprintf(stderr,"Corrupt bitstream section! Exiting.\n");
        exit(1);
      }

      /* some other error in the stream.  Not a problem, just reporting it in
         case we (the app) cares.  In this case, we don't. */
    }
  }

  /* cleanup */
  ov_clear(&vf);
  fclose(input);

  printf("ALL OK\n");
  return(0);
}
