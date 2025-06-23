#pragma once

#include <string>

#include "../biz/user.h"

class FileManager
{
public:
    FileManager();
    ~FileManager();

    int CreateFile(User* user, const std::string& filename);
    bool WriteChunk(int file_id, uint64_t offset, const char *data, size_t len);
    std::pair<char*, size_t> ReadChunk(int file_id, uint64_t offset, size_t len);

private:
    std::string base_path_;
};