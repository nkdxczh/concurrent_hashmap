#include "StripedCuckooHashSet.h"
#include "CuckooHashSet.h"
//#include "TransactionalCuckooHashSet.h"
#include "TransactionalPhasedCuckooHashSet.h"
#include <thread>
#include <iostream>
#include <sys/time.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <stdlib.h>

using namespace std;

#define TASKS 10000
//#define TASKS 5000

int** records;

void do_work(int id, StripedCuckooHashSet<int>* hs, int tasks){
    //srand(time(NULL));
    for(int i = 0; i < tasks; ++i){
        if(abs(rand() % 10) < 4){
            if(hs->remove(rand()))records[id][0]++;
            else records[id][1]++;
        }
        else{
            //cout << id << " add" << endl;
            if(hs->add(rand()))records[id][2]++;
            else records[id][3]++;
        }
    }
}

void do_work1(int id, TransactionalPhasedCuckooHashSet<int>* hs, int tasks){
    //return;
    //srand(time(NULL));
    for(int i = 0; i < tasks; ++i){
        if(abs(rand() % 10) < 4){
            if(hs->remove(rand()))records[id][0]++;
            else records[id][1]++;
        }
        else{
            //cout << id << " add" << endl;
            if(hs->add(rand()))records[id][2]++;
            else records[id][3]++;
        }
    }
}

int main(){
    srand(time(0));
    
    CuckooHashSet<int>* chs = new CuckooHashSet<int>();
    chs->populate();
    struct timeval start1;
    gettimeofday(&start1, NULL);
    for(int i = 0; i < TASKS; ++i){
        if(abs(rand() % 10) < 4){
            chs->remove(rand());
        }
        else{
            chs->add(rand());
        }
    }
    struct timeval end1;
    gettimeofday(&end1, NULL);

    cout << "total time: " << (end1.tv_sec - start1.tv_sec) * 1000000 + ((int)end1.tv_usec - (int)start1.tv_usec) << endl;
    cout << chs->size() << endl;

    free(chs);

    int thread_num = 500;

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    StripedCuckooHashSet<int>* hs = new StripedCuckooHashSet<int>();
    hs->populate();

    std::thread* threads[thread_num];


    int left = TASKS;
    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new std::thread(do_work, i, hs, TASKS / thread_num);
        else threads[i] = new std::thread(do_work, i, hs, left);
        left -= (int)(TASKS / thread_num);
    }

    struct timeval start;
    gettimeofday(&start, NULL);

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

    free(hs);

    TransactionalPhasedCuckooHashSet<int>* ths = new TransactionalPhasedCuckooHashSet<int>();
    ths->populate();
    thread_num = 50;

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    left = TASKS;
    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new std::thread(do_work1, i, ths, TASKS / thread_num);
        else threads[i] = new std::thread(do_work1, i, ths, left);
        left -= (int)(TASKS / thread_num);
    }

    gettimeofday(&start, NULL);

    for(int i = 0; i < thread_num; ++i)
        threads[i]->join();

    gettimeofday(&end, NULL);

    expect = 1024;
    for(int i = 0; i < thread_num; ++i){
        expect = expect - records[i][0] + records[i][2];
    }


    cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    cout << "expect size: " << expect << endl;
    cout << "size: " << ths->size() << endl;

    free(ths);

    return 0;
}
