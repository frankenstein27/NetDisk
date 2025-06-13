#include <stdio.h>

#include "../include/config_loader.h"
#include "../include/logger.h"
#include "../include/connection.h"
#include "../include/connectionpool.h"
#include "../include/threadpool.h"
#include "../include/epollmanager.h"

int main(int argc, char *argv[])
{
    // 日志初始化
    Logger *logger = new Logger();
    std::shared_ptr<spdlog::logger> global_logger = spdlog::get("logger");
    if (global_logger)
    {
        try
        {
            global_logger->trace("this is a test context");
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    // 配置文件初始化
    ConfigLoader *config_loader = ConfigLoader::GetInstance();

    // 参数代表是否启用 ET 模式，此处启用
    EpollManager *epoll_manager = new EpollManager(true);
    epoll_manager->WaitEvents();

    delete logger;
    delete config_loader;
    return 0;
}