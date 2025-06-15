#include "../../include/connection.h"

#include <unistd.h>
#include <fcntl.h>

Connection::Connection()
{
    if (!logger_)
        logger_ = spdlog::get("logger");
}

Connection::Connection(int sockfd, const sockaddr_in &client_addr)
    : sockfd_(sockfd)
{
    if(!logger_)
        logger_ = spdlog::get("logger");
    // 设置非阻塞
    SetNoblocking();
    // 获取 ip port
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    remote_ip_ = ip_str;
    remote_port_ = ntohs(client_addr.sin_port);
    // 初始化收发缓冲区和活跃时间
    last_active_ = time(nullptr);
    recv_buf_ = new char[BUFFER_SIZE];
    send_buf_ = new char[BUFFER_SIZE];
    status_ = Status::WAIT;
}

void Connection::InitConnection(int sockfd, const sockaddr_in& client_addr)
{
    if (!logger_)
        logger_ = spdlog::get("logger");
    sockfd_ = sockfd;
    SetNoblocking();
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    remote_ip_ = ip_str;
    remote_port_ = ntohs(client_addr.sin_port);
    last_active_ = time(nullptr);
    recv_buf_ = new char[BUFFER_SIZE];
    send_buf_ = new char[BUFFER_SIZE];
}

const int Connection::GetSocket()
{
    return sockfd_;
}

const std::string Connection::GetRemoteIp()
{
    return remote_ip_;
}

const uint16_t Connection::GetRemotePort()
{
    return remote_port_;
}
const std::string Connection::GetRecvBuf()
{
    return recv_buf_;
}

ssize_t Connection::ReadData()
{
    char buf[BUFFER_SIZE];
    ssize_t read_bytes = recv(sockfd_, recv_buf_, BUFFER_SIZE, 0);
    if(read_bytes > 0)
    {
        logger_->info("From " + remote_ip_ + " : " + std::to_string(remote_port_) + " received " + std::to_string(read_bytes) + " bytes.");
        last_active_ = time(nullptr);
        status_ = Status::OK;
    }
    else if(!read_bytes)
    {
        // 对方关闭连接
        status_ = Status::CLOSED;
    }
    else
    {
        // 数据已经全部读取完(或者还没有实际的数据来)，如果以后接到数据，epoll 就能再次触发 sockfd 上的 EPOLLIN 事件，以驱动下一次读操作
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            status_ = Status::WAIT;
        }
        else
        {
            status_ = Status::ERROR;
        }
    }
    return read_bytes;
}

ssize_t Connection::SendData(const char *data, size_t len)
{

}

const Connection::Status Connection::GetStatus()
{
    return status_;
}

int Connection::SetNoblocking()
{
    int old_option = fcntl(sockfd_, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(sockfd_, F_SETFL, new_option);
    return old_option;
}

Connection::~Connection()
{
    close(sockfd_);
    delete[] recv_buf_;
    delete[] send_buf_;
}