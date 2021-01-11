//
// Created by xmjyever on 2021/1/11.
//

#ifndef MYTINYSERVER_PROCESSPOOL_H
#define MYTINYSERVER_PROCESSPOOL_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

class process{
public:
    process() : m_pid(-1){
    }
private:
    pid_t m_pid;
    int m_pipefd[2];
};

/* class processpool */
template<typename T>
class processpool{
private:
    processpool(int listenfd, int process_number=8);

public:
    /* singleton */
    static processpool<T>* create(int listenfd, int process_number = 8)
    {
        if(!m_instance)
        {
            m_instance = new processpool<T>(listenfd, process_number);
        }
        return m_instance;
    }
    ~processpool()
    {
        delete[] m_sub_process;
    }
    void run();

private:
    void setup_sig_pipe();
    void run_parent();
    void run_child();

private:
    static const int MAX_PROCESS_NUMBER = 16;
    static const int USER_PER_PROCESS = 65536;
    static const int MAX_EVENT_NUMBER = 10000;
    int m_process_number;   // total number of process in this pool
    int m_idx;  //index of process
    int m_epollfd;
    int m_listenfd;
    bool m_stop;
    process* m_sub_process;
    static processpool<T>* m_instance;
};
template<typename T>
processpol<T>* processpool<T>::m_instance = NULL;

static int sig_pipefd[2];   //deal with signal handler

static int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
static void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
static void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}
static void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char* )&msg, 1, 0);
    errno = save_errno;
}
static void addsig(int sig, void(*handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
/* processpool constructor function */
template<typename T>
processpool<T>::processpool(int listenfd, int process_number)
:m_listenfd(listenfd), m_process_number(process_number),m_idx(-1), m_stop(false)
{
    assert((process_number > 0) && (process_number <= MAX_PROCESS_NUMBER));

    m_sub_process = new process[process_number];
    assert(m_sub_process);

    // create process number's process
    for(int i=0; i<process_number; i++)
    {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);
        assert(ret == 0);

        m_sub_process[i].m_pid = fork();
        assert(m_sub_process[i].m_pid >= 0);
        if(m_sub_process[i].m_pid > 0)
        {
            close(m_sub_process[i].m_pipefd[1]);
            continue;
        }
        else
        {
            close(m_sub_process[i].m_pipefd[0]);
            m_idx = i;
            break;  //jump out child process
        }
    }
}
template<typename T>
void processpool<T>::setup_sig_pipe()
{
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);
    setnonblocking(sig_pipefd[1]);
    addfd(m_epollfd, sig_pipefd[0]);

    // set signal handler function
    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, SIG_IGN);
}
template<typename T>
void processpool<T>::run()
{
    if(m_idx != -1)
    {
        run_child();
        return;
    }
    run_parent();
}
template<typename T>
void processpool<T>::run_child()
{
    setup_sig_pipe();
    int pipefd = m_sub_process[m_idx].m_pipefd[1];
    addfd(m_epollfd, pipefd);

    epoll_event events[MAX_EVENT_NUMBER];
    T* users = new T[USER_PER_PROCESS];
    assert(users);
    int number = 0;
    int ret = -1;

    while(!m_stop)
    {
        number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0)&&(errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for(int i=0; i<number; i++)
        {

        }
    }
}
#endif //MYTINYSERVER_PROCESSPOOL_H
