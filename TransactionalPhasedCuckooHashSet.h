#ifndef   TPHS_H
#define   TPHS_H

#include <functional>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

template <class T>
class TransactionalPhasedCuckooHashSet{
    private:
        T*** tables[2];
        int capacity;
        bool isResize;

        int N;
        int LIMIT;
        int THRESHOLD;
        int PROBE_SIZE;

        int hash0(T x) transaction_safe {
            int hashvalue = hash<T>{}(x);
            return abs(hashvalue * 1567 % 16759);
        }

        int hash1(T x) transaction_safe {
            int hashvalue = hash<T>{}(x);
            return abs(hashvalue * 1913 % 19841);
        }

        void resizeTables(){

            T*** tmp_tables[2];
            tmp_tables[0] = tables[0];
            tmp_tables[1] = tables[1];

            __transaction_atomic{

                capacity *= 2;

                tables[0] = (T***)malloc(sizeof(void*) * capacity);
                tables[1] = (T***)malloc(sizeof(void*) * capacity);

                for(int i = 0; i < 2; ++i){
                    for(int j = 0; j < capacity; ++j){
                        tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                        for(int k = 0; k < PROBE_SIZE; ++k)tables[i][j][k] = NULL;
                    }
                }

                for(int i = 0; i < 2; ++i){
                    for(int j = 0; j < capacity / 2; ++j){
                        for(int z = 0; z < PROBE_SIZE; ++z){
                            if(tmp_tables[i][j][z] != NULL){
                                add(*tmp_tables[i][j][z]);
                            }
                        }
                    }
                    free(tmp_tables[i]);
                }
            }
        }

        void resize(int oldCapacity){
            resizeTables();
        }

        int setSize(T** s){
            if(s == NULL)return 0;
            int res = 0;
            for(int i = 0; i < PROBE_SIZE; ++i)
                if(s[i] != NULL)res++;
            return res;
        }

        bool setAdd(T** s, T x){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] == NULL){
                    s[i] = (T*) malloc(sizeof(T));
                    *s[i] = x;
                    return true;
                }
            }
            return false;
        }

        bool setRemove(T** s, T x){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] != NULL && *s[i] == x){
                    free(s[i]);
                    for(int j = i + 1; j < PROBE_SIZE; ++j)s[j - 1] = s[j];
                    s[PROBE_SIZE - 1] = NULL;
                    return true;
                }
            }
            return false;
        }

        bool setContains(T** s, T x){
            if(s == NULL)return false;
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] != NULL && *s[i] == x)return true;
            }
            return false;
        }

        bool relocate(int i, int hi, int oldCapacity){
            //cout << "in relocate  " << i << hi << useLock << endl;

            int hj = 0;
            int j = 1 - i;

            for(int round = 0; round < LIMIT; ++round){

                T** iSet = tables[i][hi];

                //resizing
                if(iSet == NULL || iSet[0] == NULL)return true;

                T y = *iSet[0];

                __transaction_atomic{
                    //atomic_commit{

                    if(oldCapacity != capacity){
                        return true;
                    }

                    switch(i){
                        case 0:
                            hj = hash1(y) % capacity;
                            break;
                        case 1:
                            hj = hash0(y) % capacity;
                            break;
                    }

                    T** jSet = tables[j][hj];

                    if(jSet == NULL){
                        return false;
                    }

                    if(setRemove(iSet, y)){
                        if(setSize(jSet) < THRESHOLD){
                            setAdd(jSet, y);
                            return true;
                        }
                        else if(setSize(jSet) < PROBE_SIZE){
                            setAdd(jSet, y);
                            i = 1 - i;
                            hi = hj;
                            j = 1 - j;
                        }
                        else{
                            setAdd(iSet, y);
                            return false;
                        }
                    }
                    else if(setSize(iSet) >= THRESHOLD){
                        continue;
                    }
                    else{
                        return true;
                    }
                }
                }

                return false;
            }

            public:
            TransactionalPhasedCuckooHashSet(int n = 1000, int limit = 10, int threshold = 2, int probe_size = 4){
                N = n;
                LIMIT = limit;
                THRESHOLD = threshold;
                PROBE_SIZE = probe_size;

                capacity = N;

                tables[0] = (T***)malloc(sizeof(void*) * capacity);
                tables[1] = (T***)malloc(sizeof(void*) * capacity);

                for(int i = 0; i < 2; ++i){
                    for(int j = 0; j < capacity; ++j){
                        tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                        memset (tables[i][j],0,sizeof(void*) * PROBE_SIZE);
                    }
                }

                isResize = false;
            }

            bool contains(T x){
                if( setContains(tables[0][hash0(x) % capacity], x) )return true;
                if( setContains(tables[1][hash1(x) % capacity], x) )return true;
                return false;
            }

            bool add(T x){

                //if(useLock)cout << "try add " << x << endl;
                //else cout << "move " << x << endl;
                //std::this_thread::sleep_for (std::chrono::milliseconds(1));
                bool mustResize = false;
                int oldCapacity = capacity;
                int i = -1, h = -1;

                __transaction_atomic{

                    if(contains(x)){
                        return false;
                    }

                    T *tmp = (T*)malloc(sizeof(T));
                    *tmp = x;

                    int h0 = hash0(x) % capacity, h1= hash1(x) % capacity;

                    T** set0 = tables[0][h0];
                    T** set1 = tables[1][h1];

                    if( setSize(set0) < THRESHOLD){
                        setAdd(set0, x);
                        //cout << "finish in 1 " << x <<endl;
                        return true;
                    }
                    else if( setSize(set1) < THRESHOLD){
                        setAdd(set1, x);
                        //cout << "finish in 2 " << x <<endl;
                        return true;
                    }
                    else if( setSize(set0) < PROBE_SIZE){
                        setAdd(set0, x);
                        i = 0;
                        h = h0;
                    }
                    else if( setSize(set1) < PROBE_SIZE){
                        setAdd(set1, x);
                        i = 1;
                        h = h1;
                    }
                    else{
                        mustResize = true;
                    }
                }

                if(mustResize){
                    resize(oldCapacity);
                    return add(x);    
                }
                else if(!relocate(i,h,oldCapacity)){
                    //cout << "relocate    " << i << " " << h << endl;
                    resize(oldCapacity);
                }

                //cout << "finish add " << x << endl;

                return true;
            }

            bool remove(T x){
                __transaction_atomic{
                    int h0 = hash0(x) % capacity;
                    if(setRemove(tables[0][h0], x)){
                        return true;
                    }
                    int h1 = hash1(x) % capacity;
                    if(setRemove(tables[1][h1], x)){
                        return true;
                    }
                }
                return false;
            }

            int size(){
                int res = 0;

                for(int i = 0; i < capacity; ++i){
                    res += setSize(tables[0][i]);
                    res += setSize(tables[1][i]);
                }

                return res;
            }

            void populate(){
                int count = 0;

                srand(time(NULL));
                while(count < 1024){
                    if(add(rand())){
                        count++;
                    }
                }
            }

            void print(){
                for(int  i = 0; i < 2; ++i){
                    for(int  j = 0; j < capacity; ++j){
                        if(tables[i][j] != NULL){
                            for(int z = 0; z < PROBE_SIZE; ++z){
                                if(tables[i][j][z] != NULL)cout << *tables[i][j][z] << ",";
                                else cout << "NULL,";
                            }
                        }
                        else{
                            for(int z = 0; z < PROBE_SIZE; ++z){
                                cout << "NULL,";
                            }
                        }
                        cout << endl;
                    }
                    cout << endl;
                }
            }
        };

#endif
