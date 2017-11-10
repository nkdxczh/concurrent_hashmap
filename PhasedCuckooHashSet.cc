#include <functional>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>

using namespace std;

#define N 100
#define LIMIT 10
#define THRESHOLD 2
#define PROBE_SIZE 4

#define H1 16759
#define H2 100621

template <class T>
class PhasedCuckooHashSet{
    T*** tables[2];
    int capacity;

    private:
    int hash0(T x){
        int hashvalue = hash<T>{}(x);
        return abs(hashvalue % H1 % capacity);
    }

    int hash1(T x){
        int hashvalue = hash<T>{}(x);
return abs(hashvalue % H2 % capacity);
    }

    void resize(){
        capacity *= 2;

        T*** tmp_tables[2];
        tmp_tables[0] = tables[0];
        tmp_tables[1] = tables[1];

        tables[0] = (T***)malloc(sizeof(void*) * capacity);
        tables[1] = (T***)malloc(sizeof(void*) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j)
                tables[i][j] = NULL;
        }


        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity / 2; ++j){
                if(tmp_tables[i][j] != NULL){
                    for(int z = 0; z < PROBE_SIZE; ++z){
                        if(tmp_tables[i][j][z] != NULL)add(*tmp_tables[i][j][z]);
                        else break;
                        free(tmp_tables[i][j][z]);
                    }
                }
                free(tmp_tables[i][j]);
            }
            free(tmp_tables[i]);
        }
    }

    int setSize(T** s){
        if(s == NULL)return 0;
        int res = 0;
        while(res < PROB_SIZE){
            if(s[res] != NULL)++res;
            break;
        }
        return res;
    }

    public:
    PhasedCuckooHashSet(){
        capacity = N;
        
        tables[0] = (T***)malloc(sizeof(void*) * capacity);
        tables[1] = (T***)malloc(sizeof(void*) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j)
                tables[i][j] = NULL;
        }
    }

    bool contains(T x){
        if( (T* tmp = tables[0][hash0(x)]) != NULL){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(tmp[i] != NULL && *tmp[i] == x)return true;
            }
        }
        if( (T* tmp = tables[1][hash0(x)]) != NULL){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(tmp[i] != NULL && *tmp[i] == x)return true;
            }
        }
        return false;
    }

    bool add(T x){
        if(contains(x))return false;

        T *tmp = (T*)malloc(sizeof(T));
        *tmp = x;

        int h0 = hash0(x), h1= hash1(x);
        int i = -1, h = -1;
        bool mustResize = false;

        T** set0 = tables[0][h0];
        T** set1 = tables[1][h1];

        if( (int index = setSize(set0)) < THRESHOLD){
            if(set0 == NULL)tables[0][h0] = (T**)malloc(sizeof(void*) * PROB_SIZE);
            set0[index] = tmp;
            return true;
        }
        else if( (int index = setSize(set1)) < THRESHOLD){
            if(set1 == NULL)tables[1][h1] = (T**)malloc(sizeof(void*) * PROB_SIZE);
            set1[index] = tmp;
            return true;
        }
        else if( (int index = setSize(set0)) < PROB_SIZE){
            set0[index] = tmp;
            i = 0;
            h = h0;
        }
        else if( (int index = setSize(set1)) < PROB_SIZE){
            set1[index] = tmp;
            i = 1;
            h = h1;
        }
        else{
            mustResize = true;
        }

        for(int i = 0; i < LIMIT; ++i){
            if(tables[0][hash0(*tmp)] == NULL){
                tables[0][hash0(*tmp)] = tmp;
                return true;
            }
            else{
                T *t = tables[0][hash0(*tmp)];
                tables[0][hash0(*tmp)] = tmp;
                tmp = t;
            }

            if(tables[1][hash1(*tmp)] == NULL){
                tables[1][hash1(*tmp)] = tmp;
                return true;
            }
            else{
                T *t = tables[1][hash1(*tmp)];
                tables[1][hash1(*tmp)] = tmp;
                tmp = t;
            }
        }

        if(mustResize)
            resize();
        else if(!relocate(i,h))
            resize();

        return true;
    }

    bool relocate(int i, int hi){
        int hj = 0;
        int j = 1 - i;
    }

    bool remove(T x){
        if(tables[0][hash0(x)] != NULL && *tables[0][hash0(x)] == x){
            free(tables[0][hash0(x)]);
            tables[0][hash0(x)] = NULL;
            return true;
        }
        if(tables[1][hash1(x)] != NULL && *tables[1][hash1(x)] == x){
            free(tables[1][hash1(x)]);
            tables[1][hash1(x)] = NULL;
            return true;
        }
        return false;
    }

    int size(){
        int res = 0;

        for(int i = 0; i < capacity; ++i){
            if(tables[0][i] != NULL)res++;
            if(tables[1][i] != NULL)res++;
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
            for(int  j = 0; j < N; ++j){
                if(tables[i][j] != NULL)cout << *tables[i][j] << "," ;
                else cout << "NULL" << ",";
            }
            cout << endl;
        }
    }
};



int main(int argc, char** argv){
    PhasedCuckooHashSet<int> hs;

    /*for(int i = 55; i < 100; ++i){
        cout << "put " << i << ": " << hs.add((char)i) << endl;
    }

    cout << hs.contains(70) << endl;*/
    hs.populate();
    cout << hs.size() <<endl;

}
