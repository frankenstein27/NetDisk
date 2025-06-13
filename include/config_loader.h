#pragma once
#include "./logger.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>


#define READ_MAX_SIZE 256

class ConfigLoader
{
public:
    ~ConfigLoader();
    // 删除拷贝构造函数
    ConfigLoader(const ConfigLoader &) = delete;
    // 禁用赋值符号
    ConfigLoader &operator=(const ConfigLoader &) = delete;

    // 只能通过此函数获得实例
    static ConfigLoader *GetInstance();
    void LoadConfig();
    int GetInt(const std::string &key);
    std::string GetString(const std::string &key);

private:
    // 构造函数私有实现单例模式
    ConfigLoader();
    static void init();
    std::string GetUserConfigDir();

    static ConfigLoader *p_configloader_;
    std::map<std::string, std::string> config_map_;
    // 保证多线程状态下只执行一次
    static std::once_flag init_flag_;
    // 日志
    std::shared_ptr<spdlog::logger> global_logger;
};