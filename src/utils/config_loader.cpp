#include "./config_loader.h"

// 必须初始化静态成员变量
ConfigLoader *ConfigLoader::p_configloader_ = nullptr;
std::once_flag ConfigLoader::init_flag_;

ConfigLoader::ConfigLoader()
{
    global_logger_ = spdlog::get("logger");
    LoadConfig();
}

void ConfigLoader::LoadConfig()
{
    // 读取配置文件，写入config_map_
    std::string con_path = GetUserConfigDir();
    std::fstream config_file;
    // 以只读方式打开文件
    config_file.open(con_path + "/netdisk.ini", std::ios::in);
    // 第一次运行此程序必定进入此路径（因为配置文件不存在）
    if (!config_file.is_open())
    {
        if (errno == ENOENT)
        {
            global_logger_->warn("config file does not exist.Create it and initialize.");
            config_file.open(con_path + "/netdisk.ini", std::ios::in | std::ios::out);
            if (!config_file.is_open())
            {
                global_logger_->error("open config file failed.");
                return;
            }
            else
            {
                global_logger_->trace("create config file success.");
                // 写入初始配置
                config_file.seekp(0L, std::ios::beg);
            }
        }
        else if (errno == EACCES)
        {
            global_logger_->error("Insufficient permissions. Please set the file permissions: ~/.config/HeBo/netdisk.ini");
            return;
        }
        else
        {
            global_logger_->error("Unknown error.");
            return;
        }
    }

    std::string line, cur_section;
    while (std::getline(config_file, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';') // 跳过注释
            continue;
        line = line.substr(0, line.find(';') - 1);
        line = line.substr(0, line.find('#') - 1);
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        if (line.front() == '[') // 处理节
        {
            cur_section = line.substr(1, line.find(']') - 1);
        }
        else // 处理键值对的配置：port=8080
        {
            size_t pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config_map_[cur_section + '.' + key] = value;
            }
            else
            {
                global_logger_->error("config file has a format error.");
                return;
            }
        }
    }
    config_file.close();
    // for(auto m : config_map_)
    // {
    //     std::cout << m.first << ": " << m.second << std::endl;
    // }
}

ConfigLoader *ConfigLoader::GetInstance()
{
    /**
     * @brief std::call_once 存在 bug，会抛出 Unknown error -1 的错误并且向系统发出错误信号 6:SIGABRT
     * 编译时加上选项 -pthread 即可解决，详见文档：
     * https://stackoverflow.com/questions/67945806/getting-exception-when-calling-stdcall-once
     */
    // 写法一：
    std::call_once(init_flag_, &init);
    // 写法二：
    // std::call_once(init_flag_, [](){ p_configloader_ = new ConfigLoader(); });
    return p_configloader_;
}

void ConfigLoader::init()
{
    p_configloader_ = new ConfigLoader();
}

int ConfigLoader::GetInt(const std::string &key)
{
    // 根据key查找对应的值，下个函数同理
    return atoi(config_map_[key].c_str());
}

std::string ConfigLoader::GetString(const std::string &key)
{
    return config_map_[key];
}

std::string ConfigLoader::GetUserConfigDir()
{
    const char *home = std::getenv("HOME");
    if (!home)
    {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    if (home)
    {
        return std::string(home) + "/.config/HeBo";
    }
    return ".";
}

bool DirExists(const std::string &path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

ConfigLoader::~ConfigLoader()
{
    config_map_.clear();
    p_configloader_ = nullptr;
}