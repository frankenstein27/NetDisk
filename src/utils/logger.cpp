#include "../../include/logger.h"

Logger::Logger()
{
    int max_size = 1024 * 1024 * 10;
    int file_size = 30;
    
    char path[MAX_FILE_NAME_LENGTH];
    if(nullptr == getcwd(path, sizeof(path)))
    {
        if (errno == ENAMETOOLONG)
        {
            printf("pathname string exceeds PATH_MAX bytes.\n");
        }
    }
    std::string all_path = std::string(path) + "/logs/netdisk.log";
    logger_ = spdlog::rotating_logger_mt("logger", all_path, max_size, file_size);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    logger_->set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(3));
}


Logger::~Logger()
{
    spdlog::drop("logger");
    spdlog::shutdown();
}