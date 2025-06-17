#pragma once

#include <queue>
#include <vector>
#include "./connectionpool.h"

class ThreadPool
{
public:
    ThreadPool(int thread_count, int max_tasks_size, std::vector<ConnectionPool*> conn_pools);
    ~ThreadPool();

    // 禁止拷贝构造函数和赋值符号
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // 获取当前等待的任务数量
    const size_t PendingTasks();

    // 停止线程池并等待剩余任务完成
    void shutdown();

    // 向线程池添加任务
    // 模版函数，必须在头文件中实现
    template <class F>
    bool EnqueueTask(F&& task)
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            // 检查队列是否已满
            if(max_tasks_size_ > 0 && pending_tasks_ >= max_tasks_size_)
            {
                return false;
            }
            // 包装任务以支持连接池访问
            tasks_.push(
                [thread_task = std::forward<F>(task)](int thread_id, ConnectionPool &pool)
                {
                    // 调用任务并传递连接池引用
                    thread_task(thread_id, pool);
                });
            ++pending_tasks_;
        }
        cv_.notify_one();
        return true;
    }

private:
    // 工作线程函数
    void WorkerThread(int thread_id);

private:
    // 任务类型
    using TaskType = std::function<void(int thread_id, ConnectionPool &pool)>;
    // 线程池配置
    int thread_count_;
    int max_tasks_size_;
    // 同步原语
    std::mutex mtx_;
    std::condition_variable cv_;
    // 工作线程向量
    std::vector<std::thread> workers_;
    // 任务队列
    std::queue<TaskType> tasks_;
    // 连接池数量(相当于一个数组，数组中每个元素都是一个连接池指针，与工作线程一一对应)
    std::vector<ConnectionPool*> conn_pools_;
    // 线程池状态控制
    std::atomic<bool> running_;
    std::atomic<size_t> pending_tasks_;
};