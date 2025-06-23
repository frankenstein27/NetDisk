#include "workerthread.h"

#include <unistd.h>
#include <fcntl.h>

WorkerThread::WorkerThread(int thread_id, ConnectionPool *conn_pool)
    : thread_id_(thread_id),
      conn_pool_(conn_pool),
      running_(false)
{
    logger_ = spdlog::get("logger");
    epoll_fd_ = epoll_create(1);
    if (-1 == epoll_fd_)
    {
        logger_->error("worker thread:" + std::to_string(thread_id_) + " create epoll failed.");
    }
    SetNonblocking(epoll_fd_);
}

void WorkerThread::Start()
{
    if (running_)
        return;
    running_ = true;
    thread_ = std::thread([this]()
    {
        logger_->info("worker thread: " + std::to_string(thread_id_) + " started.");
        EventLoop(); 
    });
}

void WorkerThread::Stop()
{
    if (!running_)
        return;
    running_ = false;
    new_conn_cv_.notify_one();
    if (thread_.joinable())
    {
        thread_.join();
    }
    logger_->info("worker thread: " + std::to_string(thread_id_) + " stopped.");
}

void WorkerThread::AddNewConnection(int connfd, const sockaddr_in &client_addr)
{
    std::lock_guard<std::mutex> lock(new_conn_mtx_);
    waiting_connfds_.push(connfd);
    waiting_sockaddr_.push(client_addr);
}
const int WorkerThread::GetThreadId()
{
    return thread_id_;
}

void WorkerThread::EventLoop()
{
    while (running_)
    {
        // 1. 非阻塞处理新连接队列
        std::vector<std::pair<int, sockaddr_in>> connections_to_add;
        {
            std::unique_lock<std::mutex> lock(new_conn_mtx_, std::try_to_lock);
            if (lock.owns_lock())
            {
                while (!waiting_connfds_.empty())
                {
                    connections_to_add.push_back({waiting_connfds_.front(), waiting_sockaddr_.front()});
                    waiting_connfds_.pop();
                    waiting_sockaddr_.pop();
                }
            }
        }

        // 添加新连接到epoll
        for (auto &conn_pair : connections_to_add)
        {
            Connection *conn = conn_pool_->GetConnection();
            if (!conn)
            {
                logger_->error("failed to get connection from connection pool.");
                close(conn_pair.first);
                continue;
            }
            conn->InitConnection(conn_pair.first, conn_pair.second);
            AddSocket(conn);
        }

        // 2. 等待epoll事件（设置超时避免永久阻塞）
        int ret = epoll_wait(epoll_fd_, events_, MAX_EVENTS, 10); // 10ms超时
        if (ret == 0)
        { // 超时
            continue;
        }
        if (-1 == ret)
        {
            if (errno != EINTR)
            {
                logger_->error("epoll wait error:" + std::string(strerror(errno)));
            }
            continue;
        }

        // 3. 处理epoll事件
        for (int i = 0; i < ret; ++i)
        {
            epoll_event event = events_[i];
            if (event.events & EPOLLIN)
            {
                HandleReadEvent(event);
            }
            else if (event.events & EPOLLOUT)
            {
                HandleWriteEvent(event);
            }
            else if (event.events & (EPOLLRDHUP | EPOLLHUP))
            {
                HandleCloseEvent(event);
            }
            else if (event.events & EPOLLERR)
            {
                HandleErrorEvent(event);
            }
        }
    }
}

void WorkerThread::HandleReadEvent(const epoll_event &event)
{
    
    Connection *conn = static_cast<Connection *>(event.data.ptr);
    Connection::Status status;
    while (true)
    {
        
        ssize_t ret = conn->ReadData();
        // 获取读取数据结果
        status = conn->GetStatus();
        if (status == Connection::Status::ERROR)
        {
            
            RemoveSocket(conn->GetSocket());
            break;
        }
        else if (status == Connection::Status::WAIT)
        {
            
            ResetOneShot(conn);
            break;
        }
        else if (status == Connection::Status::CLOSED)
        {
            
            // 对方关闭连接
            RemoveSocket(conn->GetSocket());
            logger_->info("A connection is shutdown.");
            break;
        }
    }
    
    status = conn->GetStatus();
    if (status != Connection::Status::CLOSED && status != Connection::Status::ERROR)
    {
        logger_->info("recv message from {}:{} : {}",
                      conn->GetRemoteIp(),
                      conn->GetRemotePort(),
                      conn->GetRecvBuf());
    }
    conn->ClearBuffer();
}

// 暂时简单化处理
void WorkerThread::HandleWriteEvent(const epoll_event &event)
{
    Connection *conn = static_cast<Connection *>(event.data.ptr);
    std::string msg = "你好，这是一条测试信息。";
    conn->SendData(msg.c_str(), sizeof(msg));
}

void WorkerThread::HandleCloseEvent(const epoll_event &event)
{
    Connection *conn = static_cast<Connection *>(event.data.ptr);
    RemoveSocket(conn->GetSocket());
    close(conn->GetSocket());
    conn_pool_->ReleaseConnection(conn);
}

void WorkerThread::HandleErrorEvent(const epoll_event &event)
{
    Connection *conn = static_cast<Connection *>(event.data.ptr);
    logger_->error("Connection error: " + std::to_string(errno));
    RemoveSocket(conn->GetSocket());
    close(conn->GetSocket());
    conn_pool_->ReleaseConnection(conn);
}

void WorkerThread::HandleBusinessLogic(Connection *conn)
{
    // 解析请求
    // auto request = conn->GetNextRequest();

    // 处理业务逻辑
    // TODO: 根据具体业务实现
    // std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";

    // 准备响应数据
    // conn->SetResponse(response);

    // 需要立即发送响应，添加EPOLLOUT事件
    // epoll_event ev;
    // ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
    // ev.data.ptr = conn;

    // epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn->GetSocket(), &ev);
}

void WorkerThread::AddSocket(Connection *conn)
{
    SetNonblocking(conn->GetSocket());
    epoll_event event;
    event.data.ptr = conn;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn->GetSocket(), &event);
}
void WorkerThread::RemoveSocket(int fd)
{
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void WorkerThread::ResetOneShot(Connection *conn)
{
    epoll_event event;
    event.data.ptr = conn;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET | EPOLLRDHUP;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn->GetSocket(), &event);
}

void WorkerThread::SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    // 设置非阻塞模式
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

WorkerThread::~WorkerThread()
{
    Stop();
    if (-1 != epoll_fd_)
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    delete conn_pool_;
}