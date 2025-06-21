#pragma once
#include <string>
#include <arpa/inet.h>

#include "./logger.h"

#define BUFFER_SIZE 1024

class Connection
{
public:
    Connection();
    Connection(int sockfd, const sockaddr_in& client_addr);
    ~Connection();

    void InitConnection(int sockfd, const sockaddr_in &client_addr);

    // Getter and Setter
    const int GetSocket();
    const std::string GetRemoteIp();
    const uint16_t GetRemotePort();
    const std::string& GetRecvBuf();

    // Data dispose
    ssize_t ReadData();
    ssize_t SendData(const char *data, size_t len);

    void Reset();
    void ClearBuffer();

    // Data dispose result
    enum class Status
    {
        OK,
        WAIT,
        CLOSED,
        ERROR,
    };
    const Status GetStatus();
    void SetStatus(Status status);

private:
    int SetNoblocking();

private:
    int sockfd_;
    std::string remote_ip_;
    // 端口 2 字节即可保存
    uint16_t remote_port_;
    time_t last_active_;
    std::string recv_buf_;
    char *send_buf_;
    Status status_;
    std::shared_ptr<spdlog::logger> logger_;
};