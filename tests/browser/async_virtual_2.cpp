// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

class DOS_Device {
    int devnum;
public:
    DOS_Device() { devnum = 0; }
    virtual bool Read(unsigned char * data,unsigned short * size);
};

DOS_Device *Devices[10];

bool __attribute__((noinline)) DOS_Device::Read(unsigned char * data,unsigned short * size) {
    printf("DOS_Device::Read (this = %ld)\n", (long)this);
    return Devices[devnum]->Read(data,size);
}

class device_CON : public DOS_Device {
public:
    bool Read(unsigned char * data,unsigned short * size);
};

bool device_CON::Read(unsigned char * data,unsigned short * size) {
    printf("device_CON::Read (this = %ld) Sleep--> \n", (long)this);
    EM_ASM({
      Module.the_this = $0;
      out('first this ' + Module.the_this);
    }, this);
    emscripten_sleep(1000);
    EM_ASM({
      out('second this ' + $0);
      assert(Module.the_this === $0, 'this must be unchanged');
    }, this);
    printf("<--Sleep (this = %ld)\n", (long)this);
    return true;
}

int main(void) {
    device_CON con;
    Devices[0] = &con;
    DOS_Device dev;
    dev.Read(0,0);
    return 0;
}

