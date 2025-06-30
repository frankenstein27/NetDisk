#pragma once

#include "./httprequest.h"
#include "./httpresponse.h"
#include "../server/connection.h"

#define CR '\r'
#define LF '\n'

class HttpParser
{
public:
    HttpParser();
    ~HttpParser();

    HttpRequest ParseRequest(Connection *conn);
    std::string BuildResponse(const HttpResponse &res);

    // 主状态机的三种状态：解析请求行、解析请求头部字段、解析消息体（仅用于解析 POST 请求）
    enum class CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    // 从状态机的三种状态：完整读取一行、行出错、行读取不完整
    enum class LINE_STATE
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    // 处理 HTTP 的结果：请求不完整需要继续读取客户数据、获得完整客户请求、客户请求有语法错误、没有足够访问权限、服务器内部错误、客户端已关闭连接
    enum class HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        FORBIDDEN_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

private:
    /// @brief 从状态机，用于解析出一行内容，buffer中 0 ~ checked_index 字节都已经分析完毕，第 checked_index ~ （read_index - 1）由从状态机分析
    /// @param buffer   读缓冲区
    /// @param checked_index    当前正在分析的字节
    /// @param read_index   数据尾部的下一字节
    /// @return 从状态机解析完成的状态
    LINE_STATE ParseLine(const char *buffer, int &checked_index, int &read_index);

    /// @brief 主状态机，分析请求行(和消息体)
    /// @param temp 请求行内容
    /// @param checkstate 状态机当前状态
    /// @return 请求行的结果
    HTTP_CODE ParseRequestLine(const char *temp, CHECK_STATE &checkstate);

    /// @brief 分析头部字段
    /// @param temp 头部字段内容
    /// @return 分析请求头部的结果
    HTTP_CODE ParseHeaders(const char *temp);

    /// @brief 分析 HTTP 请求的入口函数
    /// @param buffer 读缓冲区
    /// @param checked_index 正在分析的字节
    /// @param checkstate 主状态机当前状态
    /// @param read_index 读缓冲区尾部下一字节
    /// @param start_line 行在buffer中的起始位置
    /// @return
    HTTP_CODE ParseContent(const char *buffer, int &checked_index, CHECK_STATE &checkstate, int &read_index, int &start_line);

private:
};