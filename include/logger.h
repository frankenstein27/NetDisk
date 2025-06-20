#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <unistd.h>
#include <iostream>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#define MAX_FILE_NAME_LENGTH 512

class Logger
{
public:
    Logger();
    ~Logger();

private:
    std::shared_ptr<spdlog::logger> logger_;
};