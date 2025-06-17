#pragma once

#include <sys/epoll.h>
#include <vector>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include "./connectionpool.h"
#include "./connection.h"
#include "./logger.h"

#define MAX_EVENTS 64

#define DEBUG std::cout << "thread id: " << std::this_thread::get_id << " funtion: " << __FUNCTION__ << " line: " << __LINE__ << std::endl;

class WorkerThread
{
public:
    WorkerThread(int thread_id, ConnectionPool* conn_pool);
    ~WorkerThread();
    
    // 启动事件循环
    void Start();

    // 停止事件循环
    void Stop();

    // 添加新连接到线程的事件循环
    void AddNewConnection(int connfd, const sockaddr_in &client_addr);

    // 获取线程 id
    const int GetThreadId();

private:
    
    // 工作线程主循环
    void EventLoop();

    // 处理可读事件
    void HandleReadEvent(const epoll_event &event);

    // 处理可写事件
    void HandleWriteEvent(const epoll_event &event);

    // 处理关闭事件
    void HandleCloseEvent(const epoll_event &event);

    // 处理错误事件
    void HandleErrorEvent(const epoll_event &event);

    // 分发任务到线程池
    void HandleBusinessLogic(Connection* conn);

    // 设置非阻塞模式
    void SetNonblocking(int fd);
    // 加入到epoll内核事件表
    void AddSocket(Connection *conn);

    void RemoveSocket(int fd);

    void ResetOneShot(Connection *conn);

private:
    // 线程 ID
    int thread_id_;

    // 原语
    std::atomic<bool> running_;
    std::thread thread_;

    // 连接池
    ConnectionPool *conn_pool_;

    int epoll_fd_ = -1;
    epoll_event events_[MAX_EVENTS];

    // 待处理连接队列
    std::mutex new_conn_mtx_;
    std::condition_variable new_conn_cv_;
    std::queue<int> waiting_connfds_;
    std::queue<sockaddr_in> waiting_sockaddr_;

    // 日志
    std::shared_ptr<spdlog::logger> logger_;
};