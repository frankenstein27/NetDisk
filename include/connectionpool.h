#pragma once
#include <vector>

#include "./connection.h"


class ConnectionPool
{
public:
    ConnectionPool();
    ~ConnectionPool();

    Connection *GetConnection();
    void ReleaseConnection(Connection *conn);

private:
    std::vector<Connection *> pool_;
};