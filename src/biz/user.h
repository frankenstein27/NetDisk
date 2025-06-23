#pragma once


#include <vector>
#include "./filemeta.h"


class User
{

public:
    User();
    ~User();

    std::vector<FileMeta> GetFileList();
    std::tuple<uint64_t, uint64_t> GetQuota();

private:
    int id_;
    std::string username_;
    uint64_t total_space_;
    uint64_t used_space_;
};