/*
Copyright (c) 2016 Raspberry Pi (Trading) Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int main (int argc, char **argv)
{
  FILE  *inFile;
  FILE  *outFile = stdout;
  unsigned int word;
  int ch, i;

  if (argc != 2)
  {
     fprintf(stderr, "Usage: %s bin-file [> result]\n", argv[0]);
     return 1;
  }
  if ((inFile = fopen(argv[1],"rb")) == NULL)
  {
     fprintf(stderr, "Cannot open %s\n", argv[1]);
     return 1;
  }
  i = 0;
  word = 0;
  while ((ch = fgetc(inFile)) != EOF)
  {
    word |= ch << (8 * (i & 3));
    if (i==0)
       fputs ("  ", outFile);
    else if (i % 32 == 0)
       fputs("\n  ", outFile);
    i++;
    if (i % 4 == 0)
    {
       fprintf(outFile, "0x%08X,", word);
       if (i % 32 != 0)
          fputs(" ", outFile);
       word = 0;
    }
  }
  if (i % 4)
     fprintf(outFile, "0x%08X,", word);
  fputc('\n', outFile);
  fclose(inFile);
  return 0;
}


