#include "PhasedCuckooHashSet.h"
#include <mutex>
#include <thread>

template <class T>
class StripedCuckooHashSet : public PhasedCuckooHashSet<T> {
    private:
        mutex*** locks;
        mutex* resizeLock;
        int lockNum;

        void acquire(T x){

            //cout << "start acquire " << this->hash0(x) % lockNum << " " << this->hash1(x) % lockNum << endl;
            mutex* lock0 = locks[0][this->hash0(x) % lockNum];
            mutex* lock1 = locks[1][this->hash1(x) % lockNum];

            while(true){

                lock0->lock();

                if(lock1->try_lock())break;
                else lock0->unlock();
                std::this_thread::yield();
            }

            //cout << "acquire " << this->hash0(x) % lockNum << " " << this->hash1(x) % lockNum << endl;
        }

        void release(T x){
            mutex* lock0 = locks[0][this->hash0(x) % lockNum];
            mutex* lock1 = locks[1][this->hash1(x) % lockNum];
            lock1->unlock();
            lock0->unlock();
            //cout << "release " << this->hash0(x) % lockNum << " " << this->hash1(x) % lockNum << endl;
        }

        void resizeLocks(){
            lockNum *= 2;
            mutex*** newlocks = (mutex***)malloc(sizeof(void*) * 2);

            for(int i = 0; i < 2; ++i){
                newlocks[i] = (mutex**)malloc(sizeof(void*) * lockNum);
                for(int j = 0; j < lockNum / 2; ++j)
                    newlocks[i][j] = locks[i][j];
                for(int j = lockNum / 2; j < lockNum; ++j){
                    newlocks[i][j] = new mutex();
                    if(i==0)newlocks[i][j]->lock();
                }
                this->locks[i] = newlocks[i];
            }
        }

        void resize(int oldCapacity, bool useLock){

            if(useLock){
                for(int i = 0; i < lockNum; ++i){
                    while(!locks[0][i]->try_lock())std::this_thread::yield();
                    //locks[1][i]->lock();
                    //cout << "lock" << i << endl;
                }
                if(oldCapacity != this->capacity){
                    for(int i = 0; i < lockNum; ++i){
                        //locks[1][i]->unlock();
                        locks[0][i]->unlock();
                    }
                    return;
                }
            }

            //this->resizeLocks();

            //cout << "finish resize locks" << endl;

            this->resizeTables();

            //cout << "finish resize tables " << this->capacity << lockNum << endl;

            if(useLock){
                for(int i = 0; i < lockNum; ++i){
                    //locks[1][i]->unlock();
                    locks[0][i]->unlock();
                    //cout << "release " << i << endl;
                }
            }
        }

    public:
        StripedCuckooHashSet() : PhasedCuckooHashSet<T>(){
            lockNum = N;
            locks = (mutex***)malloc(sizeof(void*) * 2);

            for(int i = 0; i < 2; ++i){
                locks[i] = (mutex**)malloc(sizeof(void*) * lockNum);
                for(int j = 0; j < lockNum; ++j)
                    locks[i][j] = new mutex();
            }
        }
};
