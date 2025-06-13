#pragma once

#include "./connection.h"

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    Connection *GetConnection();
    void ReleaseConnection(Connection *conn);

private:
    int thread_count_;
};