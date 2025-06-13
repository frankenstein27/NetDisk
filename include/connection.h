#pragma once
#include <string>

class Connection
{
public:
    Connection();
    ~Connection();

    ssize_t ReadData();
    ssize_t SendData(const char *data, size_t len);

private:
    int sockfd_;
    std::string remote_ip_;
    time_t last_active_;
    char *recv_buf_;
    char *send_buf_;
};