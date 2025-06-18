#include "../../include/threadpool.h"

ThreadPool::ThreadPool(int thread_count, int max_tasks_size, std::vector<ConnectionPool *> conn_pools)
    : thread_count_(thread_count),
      max_tasks_size_(max_tasks_size),
      running_(true),
      next_worker_index_(0)
{

    // 创建工作线程
    for (int i = 0; i < thread_count; ++i)
    {
        WorkerThread *work = new WorkerThread(i, conn_pools[i]);
        workers_.push_back(work);
    }
    for (auto &worker : workers_)
    {
        worker->Start();
    }
}
bool ThreadPool::DispatchNewConnection(int connfd, const sockaddr_in &client_addr)
{
    size_t idx = next_worker_index_.fetch_add(1) % workers_.size();
    workers_[idx]->AddNewConnection(connfd, client_addr);
    return true;
}

void ThreadPool::shutdown()
{
    for (auto& work : workers_)
    {
        work->Stop();
    }
}

ThreadPool::~ThreadPool()
{
    if(running_)
    {
        running_ = false;
        shutdown();
    }
}