#pragma once

#include "./httprequest.h"
#include "./httpresponse.h"
#include "./connection.h"

class HttpParser
{
public:
    HttpParser();
    ~HttpParser();

    HttpRequest ParseRequest(Connection *conn);
    std::string BuildResponse(const HttpResponse &res);

    enum class FSMState
    {
        START_LINE,
        HEADERS,
        BODY,
        COMPLETE
    };

private:
    FSMState current_state_;
};