#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unordered_map>
#include <iostream>
#include "./config_loader.h"
#include "./logger.h"
#include "./connectionpool.h"

#define MAX_EVENT_NUMBER 1024
#define BACKLOG 5

#define DEBUG true

class EpollManager
{
public:
    EpollManager(bool enable_et = false);
    ~EpollManager();

    // event 指定事件类型，一般为EPOLLIN (+ EPOLLET)
    void AddSocked(int fd, sockaddr_in& addr, bool one_shot);
    void ResetOneShot(int fd);
    void RemoveSocket(int fd);
    void ETWorkingMode(int active_number);
    void WaitEvents();

private:
    int SetNonblocking(int fd);
    void InitEpoll();
    void InitServer(int port, std::string ip);

    struct sockaddr_in address_;
    int listen_fd_;
    ConfigLoader *config_loader_;
    std::shared_ptr<spdlog::logger> logger_;
    // 可以考虑拆分为一个主 Epoll 和 多个工作线程的 Epoll
    int epoll_fd_;
    epoll_event events_[MAX_EVENT_NUMBER];
    std::unordered_map<int, Connection *> connections_;
    ConnectionPool* conn_pool_;
    bool enable_et_;
    bool running_;
};