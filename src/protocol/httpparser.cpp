#include "./httpparser.h"


/// @brief 解析请求
/// @param conn 连接对象
/// @return Http请求对象
HttpRequest HttpParser::ParseRequest(Connection *conn)
{ 
    // 从 conn 读取原始数据
    const char *original_data = conn->GetRecvBuf().c_str();

    // 使用状态机(FSMState)解析请求行/头部/正文
    HTTP_CODE request_line_code = HTTP_CODE::NO_REQUEST;
    int checked_index = 0;
    CHECK_STATE checkstate = CHECK_STATE::CHECK_STATE_REQUESTLINE;
    int read_index = sizeof(original_data) + 1;
    int start_line = 0;
    // 解析请求行
    request_line_code = ParseContent(original_data, checked_index, checkstate, read_index, start_line);
    // 根据返回的请求行解析内容进行对应处理
    switch (request_line_code)
    {
        // 请求不完整需要继续读取
    case HTTP_CODE::NO_REQUEST:
        /* code */
        break;
        // 得到完整客户请求
    case HTTP_CODE::GET_REQUEST:
        /* 解析头部  --->  解析正文 */
        break;
        // 客户请求有语法错误
    case HTTP_CODE::BAD_REQUEST:
        /* code */
        break;
        // 客户没有足够权限
    case HTTP_CODE::FORBIDDEN_REQUEST:
        /* code */
        break;
        // 服务器内部错误
    case HTTP_CODE::INTERNAL_ERROR:
        /* code */
        break;
        // 客户端请求已关闭
    case HTTP_CODE::CLOSED_CONNECTION:
        /* code */
        break;
    default:
        break;
    }

    // 返回填充好的HttpRequest对象
}

/// @brief 构建HttpResponse
/// @param res HttpResponse请求对象
/// @return HttpResponse对象序列化的字符串
std::string HttpParser::BuildResponse(const HttpResponse &res)
{
    // 将HttpResponse对象序列化为字符串
    // 格式示例： "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello"
}

HttpParser::LINE_STATE HttpParser::ParseLine(const char *buffer, int &checked_index, int &read_index)
{
    for (; checked_index < read_index; ++checked_index)
    {
        char ch = buffer[checked_index];
        if(ch == CR)
        {
            // 如果已经是最后一个字符，则数据不完整
            if(checked_index + 1 >= read_index)
            {
                return LINE_STATE::LINE_OPEN;
            }
            if(buffer[checked_index + 1] == LF)
            {
                // 跳过CRLF
                checked_index += 2;
                return LINE_STATE::LINE_OK;
            }
            else
            {
                // 行有语法错误
                return LINE_STATE::LINE_BAD;
            }
        }
        // LF 不在 CR 后面
        else if(ch == LF)
        {
            return LINE_STATE::LINE_BAD;
        }
    }
    // 未找到完整的行
    return LINE_STATE::LINE_OPEN;
}

HttpParser::HTTP_CODE HttpParser::ParseRequestLine(const char *temp, CHECK_STATE &checkstate)
{

}

HttpParser::HTTP_CODE HttpParser::ParseHeaders(const char *temp)
{

}

HttpParser::HTTP_CODE HttpParser::ParseContent(const char *buffer,
                                               int &checked_index,
                                               HttpParser::CHECK_STATE &checkstate,
                                               int &read_index,
                                               int &start_line)
{
    // 用于记录当前行的读取状态
    LINE_STATE line_status = LINE_STATE::LINE_OK;
    // 用于记录 HTTP 请求的处理结果
    HTTP_CODE ret_code = HTTP_CODE::NO_REQUEST;
    // 从 buffer 中取出所有完整的行
    while ((line_status = ParseLine(buffer, checked_index, read_index)) == LINE_STATE::LINE_OK)
    {
        const char *tmp = buffer + start_line;        // start_line: 行在buffer中的起始位置
        start_line = checked_index;             // 记录下一行的起始位置
        switch (checkstate)                     // 主状态机状态
        {
            // 第一种状态：分析请求行ing
        case CHECK_STATE::CHECK_STATE_REQUESTLINE:
        // 调用请求行分析函数
            ret_code = ParseRequestLine(tmp, checkstate);
            if (ret_code == HTTP_CODE::BAD_REQUEST)
            {
                return HTTP_CODE::BAD_REQUEST;
            }
            break;
            // 第二种状态：分析请求头部
        case CHECK_STATE::CHECK_STATE_HEADER:
        // 调用头部分析函数
            ret_code = ParseHeaders(tmp);
            if (ret_code == HTTP_CODE::BAD_REQUEST)
            {
                return HTTP_CODE::BAD_REQUEST;
            }
            else if(ret_code == HTTP_CODE::GET_REQUEST)
            {
                return HTTP_CODE::GET_REQUEST;
            }
            break;
            // 第三种状态：分析请求数据
        case CHECK_STATE::CHECK_STATE_CONTENT:
        // 调用正文分析函数
        // ......
        // if(ret_code == ...)
        // ......

        break;
        }
    }
    // 没有读到一个完整的行
    if(line_status == LINE_STATE::LINE_OPEN)
    {
        return HTTP_CODE::NO_REQUEST;
    }
    // 行有语法错误
    else
    {
        return HTTP_CODE::BAD_REQUEST;
    }
}