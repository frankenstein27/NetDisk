#pragma once

#include <string>

class FileMeta
{
public:
    FileMeta();
    ~FileMeta();

private:
    int id_;
    std::string name_;
    uint64_t size_;
    time_t create_at_;
    time_t modified_at_;
};