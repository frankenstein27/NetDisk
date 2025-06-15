#include "../../include/epollmanager.h"

EpollManager::EpollManager(bool enable_et) : 
    enable_et_(enable_et),
    running_(false)
{
    logger_ = spdlog::get("logger");
    config_loader_ = ConfigLoader::GetInstance();
    // 通过配置文件获取 ip 和 port
    int port = config_loader_->GetInt("network.port");
    std::string ip = config_loader_->GetString("network.server_ip");
    logger_->trace("Init EpollManager successfully!Server port is: " + std::to_string(port) + ".And Server ip is: " + ip);

    InitServer(port, ip);
    InitEpoll();
    WaitEvents();
}

void EpollManager::AddSocked(int fd, sockaddr_in& addr, bool one_shot)
{
    // 如果是监听 socket ，不设置 oneshot 且无需创建 Connection对象
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et_)
        event.events |= EPOLLET;
    if (one_shot)
    {
        // 只有连接 socket 才需要 oneshot 和 Connection 对象
        Connection *conn = new Connection(fd, addr);
        connections_[fd] = conn;
        event.events |= EPOLLONESHOT;
        logger_->trace("New Connection: " + conn->GetRemoteIp() + ":" + std::to_string(conn->GetRemotePort()));
    }
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

void EpollManager::RemoveSocket(int fd)
{
    // auto it = connections_.find(fd);
    std::unordered_map<int, Connection *>::iterator it = connections_.find(fd);
    if(it != connections_.end())
    {
        delete it->second;
        connections_.erase(it);
    }
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
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
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd_, (sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                logger_->warn("connection error: " + std::string(strerror(errno)));
                continue;
            }
            else
            {
                AddSocked(connfd, client_address, true);
            }
        }
        // 有可读事件
        else if (events_[i].events & EPOLLIN)
        {
            Connection *conn = connections_[sockfd];
            while (true)
            {
                ssize_t ret = conn->ReadData();
                // 获取读取数据结果
                Connection::Status status = conn->GetStatus();
                
                if(status == Connection::Status::ERROR)
                {
                    RemoveSocket(sockfd);
                    break;
                }
                else if(status == Connection::Status::WAIT)
                {
                    ResetOneShot(sockfd);
                    break;
                }
                else if(status == Connection::Status::CLOSED)
                {
                    // 对方关闭连接
                    RemoveSocket(sockfd);
                    logger_->info("A connection is shutdown.");
                    break;
                }
                else if(status == Connection::Status::OK)
                {
                    // 读取数据
                    logger_->info("recv message: " + conn->GetRecvBuf());
                }
            }
        }
        else
        {
            RemoveSocket(sockfd);
            logger_->error("something else happened.");
        }
    }
}

void EpollManager::InitEpoll()
{
    // 创建 epoll 实例，得到其在内核事件表中的标识符（epoll_fd_）
    epoll_fd_ = epoll_create(1);
    // 将当前服务器监听的文件描述符加入到内核事件表中，且设置为ET模式、非阻塞IO，且不开启EPOLLONESHOT 模式
    AddSocked(listen_fd_, address_,false);
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