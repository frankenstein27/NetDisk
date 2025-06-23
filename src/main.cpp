#include <stdio.h>

#include "./utils/config_loader.h"
#include "./utils/logger.h"
#include "./server/connection.h"
#include "./server/connectionpool.h"
#include "./threadpool/threadpool.h"
#include "./server/epollmanager.h"


int main(int argc, char *argv[])
{
    // 日志初始化
    Logger *logger = new Logger();
    std::shared_ptr<spdlog::logger> global_logger = spdlog::get("logger");

    // 配置文件初始化
    ConfigLoader *config_loader = ConfigLoader::GetInstance();
    // 通过配置文件获取 连接池数量(线程数)和每个连接池大小
    // 创建 conn_count 个连接池，每个连接池中有 conn_size 个连接实例
    int conn_count = 5, conn_size = 20;
    std::vector<ConnectionPool*> conn_pools;
    // 不在此处创建对象，每启动一个线程创建一个对象
    for (ssize_t i = 0; i < conn_count; ++i)
    {
        ConnectionPool *conn_pool = new ConnectionPool(conn_size);
        conn_pools.emplace_back(conn_pool);
    }

    // 通过配置文件获取线程数(连接池数量) 以及最大任务队列大小 0表示无限制
    int max_tasks_size = 100;
    ThreadPool thread_pool(conn_count, max_tasks_size, conn_pools);

    // 通过配置文件获取 ip 和 port
    int port = config_loader->GetInt("network.port");
    std::string ip = config_loader->GetString("network.server_ip");
    // 参数代表是否启用 ET 模式，此处启用
    EpollManager *epoll_manager = new EpollManager(true, ip, port, thread_pool);
    epoll_manager->WaitEvents();

    delete epoll_manager;
    delete config_loader;
    delete logger;

    return 0;
}