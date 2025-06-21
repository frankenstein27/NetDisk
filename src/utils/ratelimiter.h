#pragma once

#include "./connection.h"
#include "./tokenbucket.h"


class RateLimiter
{
public:
    RateLimiter();
    ~RateLimiter();

    bool CheckLimit(Connection* conn);
private:
    TokenBucket bucket_;
};