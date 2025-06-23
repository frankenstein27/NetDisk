#pragma once

#include <queue>
#include <vector>
#include "../server/connectionpool.h"
#include "./workerthread.h"

class ThreadPool
{
public:
    ThreadPool(int thread_count, int max_tasks_size, std::vector<ConnectionPool*> conn_pools);
    ~ThreadPool();

    // 禁止拷贝构造函数和赋值符号
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    bool DispatchNewConnection(int connfd, const sockaddr_in &client_addr);

    // 停止线程池并等待剩余任务完成
    void shutdown();

private:
    // 线程池配置
    int thread_count_;
    int max_tasks_size_;
    // 工作线程向量
    std::vector<WorkerThread*> workers_;

    std::atomic<size_t> next_worker_index_; // 轮询分配索引

    // 线程池状态控制
    std::atomic<bool> running_;
};