#include "StripedCuckooHashSet.h"
#include <thread>
#include <iostream>
#include <sys/time.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <stdlib.h>

using namespace std;

#define TASKS 10000

int** records;

void do_work(int id, PhasedCuckooHashSet<int>* hs, int tasks){
    cout << tasks << endl;
    srand(time(NULL));
    for(int i = 0; i < tasks; ++i){
        if(abs(rand() % 10) < 0){
            if(hs->remove(rand()))records[id][0]++;
            else records[id][1]++;
        }
        else{
            if(hs->add(rand()))records[id][2]++;
            else records[id][3]++;
        }
    }
}

int main(){
    int thread_num = 1;

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    PhasedCuckooHashSet<int>* hs = new StripedCuckooHashSet<int>();
    //PhasedCuckooHashSet<int>* hs = new PhasedCuckooHashSet<int>();
    hs->populate();

    std::thread* threads[thread_num];

    struct timeval start;
    gettimeofday(&start, NULL);

    int left = TASKS;
    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new std::thread(do_work, i, hs, TASKS / thread_num);
        else threads[i] = new std::thread(do_work, i, hs, left);
        left -= (int)(TASKS / thread_num);
    }


    for(int i = 0; i < thread_num; ++i)
        threads[i]->join();

    struct timeval end;
    gettimeofday(&end, NULL);

    int expect = 1024;
    for(int i = 0; i < thread_num; ++i){
        expect = expect - records[i][0] + records[i][2];
    }

    cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    cout << "expect size: " << expect << endl;
    cout << "size: " << hs->size() << endl;

    return 0;
}
