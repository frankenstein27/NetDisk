#include "../../include/epollmanager.h"

EpollManager::EpollManager(bool enable_et, std::string ip, uint16_t port, ThreadPool& thread_pool)
 :  enable_et_(enable_et),
    running_(false),
    thread_pool_(thread_pool)
{
    logger_ = spdlog::get("logger");
    logger_->trace("Init EpollManager successfully!Server port is: " + std::to_string(port) + ".And Server ip is: " + ip);

    InitServer(port, ip);
    InitEpoll();
    WaitEvents();
}


void EpollManager::WaitEvents()
{
    // 从连接池中分配一个连接，然后从线程池中分配一个线程，此后关于此连接的操作都在分配的线程中运行
    // 在此处处理将ET转移到子线程中运行
    while (running_)
    {
        int ret = epoll_wait(epoll_fd_, events_, MAX_EVENT_NUMBER, -1);
        if (ret < 0)
        {
            logger_->error("epoll failure.");
            break;
        }
        ETWorkingMode(ret);
    }
}

void EpollManager::ETWorkingMode(int active_number)
{
    // 循环处理每个连接
    for (int i = 0; i < active_number; i++)
    {
        logger_->trace("active_number: " + std::to_string(active_number));
        int sockfd = events_[i].data.fd;
        // 有新连接请求
        if (sockfd == listen_fd_)
        {
            HandleNewConnection(sockfd);
            continue;
        }
    }
}
// 处理新连接
void EpollManager::HandleNewConnection(int sockfd)
{
    // ET 模式需要循环 accept
    while (true)
    {
        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);
        int connfd = accept(listen_fd_, (sockaddr *)&client_address, &client_addrlen);
        if (-1 == connfd)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                logger_->warn("connection error: " + std::string(strerror(errno)));
                continue;
            }
        }
        else
        {
            SetNonblocking(connfd);
            // 将 connfd 和 client_address 传给工作线程即可
            thread_pool_.EnqueueTask(
                [this, sockfd, connfd, client_address]
                (int thread_id, ConnectionPool& own_pool)
            {
                // 1. 从工作线程的连接池获取连接对象
                Connection *conn = own_pool.GetConnection();
                conn->InitConnection(connfd, client_address);

                // 2. 创建工作线程的epoll实例
                int worker_epoll = epoll_create(1);

                // 3. 添加连接到工作epoll
                epoll_event ev;
                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                ev.data.ptr = conn;
                epoll_ctl(worker_epoll, EPOLL_CTL_ADD, connfd, &ev);
                // 4.处理读写事件、业务逻辑等...
                while (true)
                {
                    ssize_t ret = conn->ReadData();
                    // 获取读取数据结果
                    Connection::Status status = conn->GetStatus();

                    if (status == Connection::Status::ERROR)
                    {
                        RemoveSocket(sockfd);
                        break;
                    }
                    else if (status == Connection::Status::WAIT)
                    {
                        ResetOneShot(sockfd);
                        break;
                    }
                    else if (status == Connection::Status::CLOSED)
                    {
                        // 对方关闭连接
                        RemoveSocket(sockfd);
                        logger_->info("A connection is shutdown.");
                        break;
                    }
                    else if (status == Connection::Status::OK)
                    {
                        // 读取数据
                        logger_->info("recv message: " + conn->GetRecvBuf());
                    }
                }

                // 5. 清理和归还连接
                close(worker_epoll);
                conn->Reset();
                own_pool.ReleaseConnection(conn);
            });
        }
    }
}

void EpollManager::InitEpoll()
{
    // 创建 epoll 实例，得到其在内核事件表中的标识符（epoll_fd_）
    epoll_fd_ = epoll_create(1);
    // 将当前服务器监听的文件描述符加入到内核事件表中，且设置为ET模式、非阻塞IO，且不开启EPOLLONESHOT 模式
    AddSocked(listen_fd_, address_);
    running_ = true;
}

void EpollManager::InitServer(int port, std::string ip)
{
    // 写入服务器端口、ip
    int ret = 0;

    bzero(&address_, sizeof(address_));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &address_.sin_addr);

    // 创建监听文件描述符
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ >= 0);

    // 绑定服务器网络配置和监听文件描述符
    socklen_t addr_size = sizeof(sockaddr);

#ifdef DEBUG
    bool opt = true;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
#endif

    if (-1 == bind(listen_fd_, (const sockaddr *)&address_, addr_size))
        assert(-1 != ret);

    // 开始监听 
    ret = listen(listen_fd_, BACKLOG);
    assert(-1 != ret);
}


void EpollManager::AddSocked(int fd, sockaddr_in& addr)
{
    SetNonblocking(fd);
    // 如果是监听 socket ，不设置 oneshot 且无需创建 Connection对象
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et_)
        event.events |= EPOLLET;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

void EpollManager::RemoveSocket(int fd)
{
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollManager::ResetOneShot(int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT;
    if (enable_et_)
        event.events |= EPOLLET;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
}

int EpollManager::SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    // 设置非阻塞模式
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

EpollManager::~EpollManager()
{
    running_ = false;
}