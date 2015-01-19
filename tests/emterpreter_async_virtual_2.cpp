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
    printf("DOS_Device::Read (this = %i)\n", (int)this);
    return Devices[devnum]->Read(data,size);
}

class device_CON : public DOS_Device {
public:
    bool Read(unsigned char * data,unsigned short * size);
};

bool device_CON::Read(unsigned char * data,unsigned short * size) {
    printf("device_CON::Read (this = %i) Sleep--> \n", (int)this);
    EM_ASM_ARGS({
      Module.the_this = $0;
      Module.print('first this ' + Module.the_this);
    }, this);
    emscripten_sleep(1000);
    EM_ASM_ARGS({
      Module.print('second this ' + $0);
      assert(Module.the_this === $0, 'this must be unchanged');
    }, this);
    printf("<--Sleep (this = %i)\n", (int)this);
    return true;
}

int main(void) {
    device_CON con;
    Devices[0] = &con;
    DOS_Device dev;
    dev.Read(0,0);
    int result = 1;
    REPORT_RESULT();
}

