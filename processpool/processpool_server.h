//
// Created by xmjyever on 2021/1/12.
// 文件描述：本文件是用进程池实现的CGI服务器头文件
//

#ifndef MYTINYSERVER_PROCESSPOOL_SERVER_H
#define MYTINYSERVER_PROCESSPOOL_SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

#include "processpool.h"

/* 用于处理客户CGI请求的类，它可以作为processpool类的模板参数 */
class cgi_conn
{
public:
    cgi_conn(){}
    ~cgi_conn(){}
    void init(int epollfd, int sockfd, const sockaddr_in& client_addr)
    {
        m_epollfd = epollfd;
        m_sockfd = sockfd;
        m_address = client_addr;
        memset(m_buf, '\0', BUFFER_SIZE);
        m_read_idx = 0;
    }
    void process()
    {
        int idx = 0;
        int ret = -1;
        /* read and analyse the user's data */
        while(true)
        {
            idx = m_read_idx;
            ret = recv(m_sockfd, m_buf+idx, BUFFER_SIZE-1-idx, 0);
            if(ret < 0)
            {
                if(errno != EAGAIN)
                {
                    removefd(m_epollfd, m_sockfd);
                }
                break;
            }
            else if(ret == 0)
            {
                removefd(m_epollfd, m_sockfd);
                break;
            }
            else
            {
                m_read_idx += ret;
                printf("user content is: %s\n", m_buf);
                for(; idx < m_read_idx; ++idx)
                {
                    if((idx >= 1) && (m_buf[idx-1] == '\r') && (m_buf[idx] == '\n'))
                        break;
                }
                if(idx == m_read_idx)
                {
                    continue;
                }
                m_buf[idx-1] = '\0';

                char* file_name = m_buf;
				/* 判断客户要运行的CGI程序是否存在 */
				if(access(file_name, F_OK) == -1)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				/* 创建子进程来执行CGI程序 */
				ret = fork();
				if(ret == -1)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else if(ret > 0)
				{
					/* 父进程只需关闭连接 */
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else
				{
					/*子进程将标准输出定向到m_sockfd, 并执行CGI程序*/
					close(STDOUT_FILENO);
					dup(m_sockfd);
					execl(m_buf, m_buf, 0);
					exit(0);
				}
            }
        }
    }

private:
    // buffer size
    static const int BUFFER_SIZE = 1024;
    static int m_epollfd;
    int m_sockfd;
    sockaddr_in m_address;
    char m_buf[BUFFER_SIZE];
    int m_read_idx;
};

int cgi_conn::m_epollfd = -1;

#endif //MYTINYSERVER_PROCESSPOOL_SERVER_H
