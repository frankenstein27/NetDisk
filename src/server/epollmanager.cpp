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

void EpollManager::AddSocked(int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et_)
        event.events |= EPOLLET;
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    SetNonblocking(fd);
}

void EpollManager::RemoveSocket(int fd)
{
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
    char buf[BUFFER_SIZE];
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
                char tmp[256];
                logger_->trace("accept a new connection with ip: " + std::string(inet_ntop(AF_INET, &client_address.sin_addr, tmp, INET_ADDRSTRLEN)) + " and port: " + std::to_string(ntohs(client_address.sin_port)));
                AddSocked(connfd, true);
            }
        }
        // 有可读事件
        else if (events_[i].events & EPOLLIN)
        {
            logger_->trace("event trigger once");
            while (1)
            {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if (ret < 0)
                {
                    // 数据已经全部读取完(或者还没有实际的数据来)，如果以后接到数据，epoll 就能再次触发 sockfd 上的 EPOLLIN 事件，以驱动下一次读操作
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        ResetOneShot(sockfd);
                        logger_->info("read later.");
                        break;
                    }
                    RemoveSocket(sockfd);
                    close(sockfd);
                    break;
                }
                // 对方关闭连接
                else if (ret == 0)
                {
                    RemoveSocket(sockfd);
                    logger_->info("A connection is shutdown.");
                    close(sockfd);
                }
                // 读取数据
                else
                {
                    logger_->info("Get " + std::to_string(ret) + " bytes of content: " + std::string(buf));
                    printf("Get %d bytes of content: %s\n", ret, buf);
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
    // 将当前服务器监听的文件描述符加入到内核事件表中，且设置为ET模式、非阻塞IO
    AddSocked(listen_fd_, false);
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