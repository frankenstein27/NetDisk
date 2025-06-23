#pragma once

#include "../../include/third_lib/spdlog/sinks/rotating_file_sink.h"
#include "../../include/third_lib/spdlog/spdlog.h"

#include <unistd.h>
#include <iostream>

#define MAX_FILE_NAME_LENGTH 512

class Logger
{
public:
    Logger();
    ~Logger();

private:
    std::shared_ptr<spdlog::logger> logger_;
};