#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"


class deadlockSolver
{
public:
    int relationMap[10][10]; //储存关系的有向图
    // 拓扑排序的入读与出度
    int outDegree[10];
    int inDegree[10];
    // 筷子信号量管理器
    Semaphore* chopstickArray;
public:
    // 初始化
    void init(Semaphore* chopsticks);
    deadlockSolver(Semaphore* chopsticks); 
    void show();// 展示各哲学家依赖关系
    int topoSort();// 拓扑排序： 通过有无环来确定是否死锁
    void addEdge(int begin,int end);
    void removeEdge(int begin,int end);
};