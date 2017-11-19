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

int main(int argc, char *argv[]){

    int thread_num = 500;
    int n = 1000;
    int limit = 10;
    int threshold = 4;
    int probe_size = 8;
    int TASKS = 1000;

    int expect = 1024;
    struct timeval start;
    struct timeval end;
    int left;

    std::thread* threads[thread_num];

    if(argc == 6){
        thread_num = atoi(argv[1]);
        n = atoi(argv[2]);
        TASKS = atoi(argv[3]);
        threshold = atoi(argv[4]);
        probe_size = atoi(argv[5]);
    }

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    srand(time(0));
    
    CuckooHashSet<int>* chs = new CuckooHashSet<int>(n, limit);
    chs->populate();
    gettimeofday(&start, NULL);
    for(int i = 0; i < TASKS; ++i){
        if(abs(rand() % 10) < 4){
            chs->remove(rand());
        }
        else{
            chs->add(rand());
        }
    }
    gettimeofday(&end, NULL);

    //cout << "-------------- CuckooHashSet -------------" << endl; 
    //cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    //cout << "size: " << chs->size() << endl;
    cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(chs);

    StripedCuckooHashSet<int>* hs = new StripedCuckooHashSet<int>(n, limit, threshold, probe_size);
    hs->populate();

    left = TASKS;
    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new std::thread(do_work, i, hs, TASKS / thread_num);
        else threads[i] = new std::thread(do_work, i, hs, left);
        left -= (int)(TASKS / thread_num);
    }

    gettimeofday(&start, NULL);

    for(int i = 0; i < thread_num; ++i)
        threads[i]->join();

    gettimeofday(&end, NULL);

    for(int i = 0; i < thread_num; ++i){
        expect = expect - records[i][0] + records[i][2];
    }


    //cout << "-------------- StripedCuckooHashSet -------------" << endl; 
    //cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    //cout << "expect size: " << expect << endl;
    //cout << "size: " << hs->size() << endl;
    cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(hs);

    TransactionalPhasedCuckooHashSet<int>* ths = new TransactionalPhasedCuckooHashSet<int>(n, limit, threshold, probe_size);
    ths->populate();

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


    //cout << "-------------- TransactionalCuckooHashSet -------------" << endl; 
    //cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    //cout << "expect size: " << expect << endl;
    //cout << "size: " << ths->size() << endl;
    cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(ths);

    return 0;
}
