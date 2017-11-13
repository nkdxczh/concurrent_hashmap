#include <functional>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <string.h>

using namespace std;

#define N 10
#define LIMIT 10
#define THRESHOLD 2
#define PROBE_SIZE 4

#define H1 16759
#define H2 19841

#define TASKS 10000

template <class T>
class PhasedCuckooHashSet{
    T*** tables[2];
    int capacity;

    private:
    int hash0(T x){
        int hashvalue = hash<T>{}(x);
        return abs(hashvalue * 9623 % H1 % capacity);
    }

    int hash1(T x){
        int hashvalue = hash<T>{}(x);
        return abs(hashvalue * 8443 % H2 % capacity);
    }

    void resize(){
        capacity *= 2;

        T*** tmp_tables[2];
        tmp_tables[0] = tables[0];
        tmp_tables[1] = tables[1];

        tables[0] = (T***)malloc(sizeof(void*) * capacity);
        tables[1] = (T***)malloc(sizeof(void*) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j){
                tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                memset (tables[i][j],0,sizeof(void*) * PROBE_SIZE);
            }
        }


        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity / 2; ++j){
                for(int z = 0; z < PROBE_SIZE; ++z){
                    if(tmp_tables[i][j][z] != NULL){
                        add(*tmp_tables[i][j][z]);
                        //free(tmp_tables[i][j][z]);
                    }
                }
                //free(tmp_tables[i][j]);
            }
            free(tmp_tables[i]);
        }
    }

    int setSize(T** s){
        if(s == NULL)return 0;
        int res = 0;
        for(int i = 0; i < PROBE_SIZE; ++i)
            if(s[i] != NULL)res++;
        return res;
    }

    bool setAdd(T** s, T x){
        int length = setSize(s);
        if(length >= PROBE_SIZE)return false;
        s[length] = (T*)malloc(sizeof(T));
        *s[length] = x;
        return true;
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

    void acquire(T x){}

    void release(T x){}

    public:
    PhasedCuckooHashSet(){
        capacity = N;

        tables[0] = (T***)malloc(sizeof(void*) * capacity);
        tables[1] = (T***)malloc(sizeof(void*) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j){
                tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                memset (tables[i][j],0,sizeof(void*) * PROBE_SIZE);
            }
        }
    }

    bool contains(T x){
        if( setContains(tables[0][hash0(x)], x) )return true;
        if( setContains(tables[1][hash1(x)], x) )return true;
        return false;
    }

    bool add(T x){
        acquire(x);

        if(contains(x)){
            release(x);
            return false;
        }

        T *tmp = (T*)malloc(sizeof(T));
        *tmp = x;

        int h0 = hash0(x), h1= hash1(x);
        int i = -1, h = -1;
        bool mustResize = false;

        T** set0 = tables[0][h0];
        T** set1 = tables[1][h1];

        if( setSize(set0) < THRESHOLD){
            setAdd(set0, x);
            release(x);
            return true;
        }
        else if( setSize(set1) < THRESHOLD){
            setAdd(set1, x);
            release(x);
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
        release(x);

        if(mustResize){
            resize();
            add(x);    
        }
        else if(!relocate(i,h))
            resize();

        return true;
    }

    bool relocate(int i, int hi){
        int hj = 0;
        int j = 1 - i;

        for(int round = 0; round < LIMIT; ++round){
            T** iSet = tables[i][hi];
            T y = *iSet[0];
            switch(i){
                case 0:
                    hj = hash1(y);
                    break;
                case 1:
                    hj = hash0(y);
                    break;
            }

            acquire(y);

            T** jSet = tables[j][hj];

            if(setRemove(iSet, y)){
                if(setSize(jSet) < THRESHOLD){
                    setAdd(jSet, y);
                    release(y);
                    return true;
                }
                else if(setSize(jSet) < PROBE_SIZE){
                    setAdd(jSet, y);
                    i = 1 - i;
                    hi = hj;
                    j = 1 - j;
                    release(y);
                }
                else{
                    setAdd(iSet, y);
                    release(y);
                    return false;
                }
            }
            else if(setSize(iSet) >= THRESHOLD){
                release(y);
                continue;
            }
            else{
                release(y);
                return true;
            }
        }

        return false;
    }

    bool remove(T x){
        acquire(x);
        int h0 = hash0(x);
        if(setRemove(tables[0][h0], x)){
            release(x);
            return true;
        }
        int h1 = hash1(x);
        if(setRemove(tables[1][h1], x)){
            release(x);
            return true;
        }
        release(x);
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

int main(int argc, char** argv){
    PhasedCuckooHashSet<int> hs;

    hs.populate();

    int success_remove = 0;
    int fail_remove = 0;
    int success_insert = 0;
    int fail_insert = 0;
    srand(time(NULL));
    for(int i = 0; i < TASKS; ++i){
        if(rand() % 10 < 4){
            if(hs.remove(rand())) success_remove++;
            else fail_remove++;
        }
        else{
            if(hs.add(rand()))success_insert++;
            else fail_insert++;
        }
    }
    cout << "size: " << hs.size() << endl;
    cout << "expect size: " << 1024 + success_insert - success_remove << endl;
    cout << "success remove: " << success_remove << endl;
    cout << "fail remove: " << fail_remove << endl;
    cout << "success insert: " << success_insert << endl;
    cout << "fail insert: " << fail_insert << endl;
}
