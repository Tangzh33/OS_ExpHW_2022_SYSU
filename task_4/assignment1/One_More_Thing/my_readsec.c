#include "mydef.h"
// mydef has self-defined inb and outb
#define SECTSIZE        512

/* wait - wait for disk ready */
void
wait(void) {
    // Tang: The output of port 0x1F7 is the status of disk
    // Tang: output[7] == 0; output[3] == 1; output[0] == 0 means disk is ready
    while ((inb(0x1F7) & 0xC0) != 0x40)
        /* Waiting*/;
}

void
myreadsec(void * target_addr, uint32_t port_addr) {
    // wait for disk to be ready
    wait();

    outb(0x1F2, 1);                         // count = 1
    outb(0x1F3,   port_addr & 0xFF);
    outb(0x1F4,  (port_addr >> 8) & 0xFF);
    outb(0x1F5,   port_addr >> 16);
    outb(0x1F6, ((port_addr >> 24) & 0xF) | 0xE0);
    outb(0x1F7, 0x20);                      // cmd 0x20 - read sectors

    wait();

    // read a sector
    insl(0x1F0, target_addr, SECTSIZE / 4);
    // insl read 4 byte into target_addr

}