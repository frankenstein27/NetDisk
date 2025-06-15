#include "../../include/connectionpool.h"

ConnectionPool::ConnectionPool()
{
    connection_count_ = 100;
    for (int i = 0; i < connection_count_; i++)
    {
        Connection *conn = new Connection();
        pool_.push(conn);
    }
}


Connection* ConnectionPool::GetConnection()
{

    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]()
    {
        return !pool_.empty(); 
    });

    Connection *conn = pool_.front();
    pool_.pop();
    lock.unlock();
    return conn;
}

void ConnectionPool::ReleaseConnection(Connection *conn)
{
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (pool_.size() < connection_count_)
        {
            conn->Reset();
            pool_.push(conn);
        }
        else
        {
            delete conn;
        }
    }
    cv_.notify_one();
}


ConnectionPool::~ConnectionPool()
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (!pool_.empty())
    {
        Connection *conn = pool_.front();
        pool_.pop();
        delete conn;
    }
}