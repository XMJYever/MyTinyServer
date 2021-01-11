//
// Created by xmjyever on 2021/1/10.
//
//ncapsulates three thread synchronization model
//

#ifndef MYTINYSERVER_LOCKER_H
#define MYTINYSERVER_LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* semaphore */
class sem{
public:
    sem(){  // constructor function
        if(sem_init(&m_sem, 0, 0) != 0){
            throw std::exception();
        }
    }
    /* destroy sem */
    ~sem(){
        sem_destroy(&m_sem);
    }
    /* wait sem */
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }
    /* increase sem */
    bool post(){
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

/* mutex */
class locker{
public:
    locker(){
        if(pthread_mutex_init(&m_mutex, NULL) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&m_mutex);
    }
    /* get locker */
    bool lock(){
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get(){
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;
};
/* condition variables */
class cond{
public:
    cond(){
        if(pthread_mutex_init(&m_mutex, NULL) != 0){
            throw std::exception();
        }
        if(pthread_cond_init(&m_cond, NULL) != 0){
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond(){
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    /* wait condition vari */
    bool wait(){
        int ret= 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal(){
        return pthread_cond_signal(&m_cond)==0;
    }
    bool broadcast(){
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};
#endif //MYTINYSERVER_LOCKER_H
