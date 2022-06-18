#include "syscall.h"
#include "interrupt.h"
#include "stdlib.h"
#include "asm_utils.h"
#include "os_modules.h"

int system_call_table[MAX_SYSTEM_CALL];

SystemService::SystemService() {
    initialize();
}

void SystemService::initialize()
{
    memset((char *)system_call_table, 0, sizeof(int) * MAX_SYSTEM_CALL);
    // 代码段选择子默认是DPL=0的平坦模式代码段选择子，DPL=3，否则用户态程序无法使用该中断描述符
    interruptManager.setInterruptDescriptor(0x80, (uint32)asm_system_call_handler, 3);
}

bool SystemService::setSystemCall(int index, int function)
{
    system_call_table[index] = function;
    return true;
}

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

int syscall_1(int a1, int a2, int a3, int a4, int a5){
    printf("this is syscall_1 for %d %d %d %d %d\n",a1,a2,a3,a4,a5);
    int max = a1;
    if(a2 > max){
        max = a2;
    }
    if(a3 > max){
        max = a3;
    }
    if(a4 > max){
        max = a4;
    }
    if(a5 > max){
        max = a5;
    }
    return max;
}

int syscall_2_out_port(int port, int value, int p1, int p2, int p3){
    uint16 port_asm = port;
    uint8 value_asm = value;
    asm_out_port(port_asm,value_asm);
    return 0;
}

int syscall_3_in_port(int port, uint8 * value, int p1, int p2, int p3){
    uint16 port_asm = port;
    asm_in_port(port_asm,value);
    return 0;
}

int syscall_4_screen_print(char * screen, int pos, int c,int color,int p1){
    uint8 c_asm = c;
    uint8 color_asm = color;
    screen[2*pos] = c_asm;
    screen[2*pos + 1] = color_asm;
    return 0;
}

int syscall_5_screen_input(char * screen, int pos, int * c, int * color, int p1){
    *c = screen[2*pos];
    *color = screen[2*pos+1];
    return 0;
}

int fork() {
    return asm_system_call(6);
}

int syscall_6_fork() {
    return programManager.fork();
}

void exit(int ret){
    asm_system_call(7,ret);
}
void syscall_7_exit(int ret){
    programManager.exit(ret);
}

int wait(int *retval){
    return asm_system_call(8,(int)retval);
}
int syscall_8_wait(int *retval){
    return programManager.wait(retval);
}

int processCollector(){
    return asm_system_call(9);
}
int syscall_9_processCollector(){
    return programManager.processCollector();
}