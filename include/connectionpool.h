#pragma once
#include <queue>

#include "./connection.h"


class ConnectionPool
{
public:
    ConnectionPool();
    ~ConnectionPool();

    Connection *GetConnection();
    void ReleaseConnection(Connection *conn);

private:

    int connection_count_;
    std::queue<Connection *> pool_;
    std::mutex mtx_;
    std::condition_variable cv_;
};