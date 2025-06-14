#pragma once

#include <string>
#include <map>

class HttpResponse
{

public:
    HttpResponse();
    ~HttpResponse();

    void AddHeader(const std::string &key, const std::string &value);

private:
    int status_code_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};