#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unordered_map>
#include <iostream>
#include "./config_loader.h"
#include "./logger.h"
#include "./threadpool.h"

#define MAX_EVENT_NUMBER 1024
#define BACKLOG 5

class EpollManager
{
public:
    EpollManager(bool enable_et, std::string ip, uint16_t port, ThreadPool &thread_pool);
    ~EpollManager();

    // event 指定事件类型，一般为EPOLLIN (+ EPOLLET)
    void AddSocked(int fd);
    void ResetOneShot(int fd);
    void RemoveSocket(int fd);
    void ETWorkingMode(int active_number);
    void WaitEvents();

private:
    // 统一事件源：主要用于安全结束服务器程序
    // 信号处理函数
    static void SigHandler(int sig);
    // 设置信号的处理函数
    void AddSig(int sig);
    int SetNonblocking(int fd);
    void InitEpoll();
    void InitServer(int port, std::string ip);
    void HandleNewConnection(int sockfd);
    void StopServer();

private:
    static int pipefd_[2];
    struct sockaddr_in address_;
    int listen_fd_;
    std::shared_ptr<spdlog::logger> logger_;
    int epoll_fd_;
    epoll_event events_[MAX_EVENT_NUMBER];
    bool enable_et_;
    bool running_;
    // 线程池引用
    ThreadPool &thread_pool_;
};