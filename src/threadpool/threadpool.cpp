#include "../../include/threadpool.h"

ThreadPool::ThreadPool(int thread_count, int max_tasks_size, std::vector<ConnectionPool> conn_pool)
    : thread_count_(thread_count),
      max_tasks_size_(max_tasks_size),
      conn_pool_(std::move(conn_pool)),
      running_(true),
      pending_tasks_(0)
{
    // 创建工作线程
    workers_.reserve(thread_count_);
    for (int i = 0; i < thread_count; ++i)
    {
        workers_.emplace_back(ThreadPool::WorkerThread, this, i);
    }
}

void ThreadPool::WorkerThread(int thread_id)
{
    // 获取当前线程对应的连接池
    ConnectionPool &own_conn_pool = conn_pool_[thread_id];
    while (running_)
    {
        TaskType task;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]
            {
                return !tasks_.empty() || !running_;
            });

            if(!running_)
                break;

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(thread_id, own_conn_pool);
        --pending_tasks_;
    }
}

const size_t ThreadPool::PendingTasks()
{
    return pending_tasks_.load();
}


void ThreadPool::shutdown()
{
    // 唤醒所有等待线程
    cv_.notify_all();
    for (auto& work : workers_)
    {
        if(work.joinable())
            work.join();
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