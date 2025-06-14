#pragma once

#include <string>
#include <map>

class HttpRequest
{

public:
    HttpRequest();
    ~HttpRequest();

    std::string GetHearder(const std::string& key);

private:
    std::string method_;
    std::string url_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};