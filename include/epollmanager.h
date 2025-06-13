#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include "./connection.h"
#include "./config_loader.h"
#include "./logger.h"

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10
#define BACKLOG 5

#define DEBUG true

class EpollManager
{
public:
    EpollManager(bool enable_et = false);
    ~EpollManager();

    // event 指定事件类型，一般为EPOLLIN (+ EPOLLET)
    void AddSocked(int fd, bool one_shot);
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
    int epoll_fd_;
    epoll_event events_[MAX_EVENT_NUMBER];
    std::map<int, Connection *> connections_;
    bool enable_et_;
    bool running_;
};