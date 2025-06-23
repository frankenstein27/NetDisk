#include "./httpparser.h"


HttpRequest HttpParser::ParseRequest(Connection *conn)
{ 
    // 从 conn 读取原始数据
    const char *original_data = conn->GetRecvBuf().c_str();

    // 使用状态机(FSMState)解析请求行/头部/正文 


    // 返回填充好的HttpRequest对象
}

std::string HttpParser::BuildResponse(const HttpResponse &res)
{
    // 将HttpResponse对象序列化为字符串
    // 格式示例： "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello"
}

HttpParser::LINE_STATE HttpParser::ParseLine(char *buffer, int &checked_index, int &read_index)
{

}

HttpParser::HTTP_CODE HttpParser::ParseRequestLine(char *temp, CHECK_STATE &checkstate)
{

}

HttpParser::HTTP_CODE HttpParser::ParseHeaders(char *temp)
{

}

HttpParser::HTTP_CODE HttpParser::ParseContent(char *buffer,
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
        char *tmp = buffer + start_line;        // start_line: 行在buffer中的起始位置
        start_line = checked_index;             // 记录下一行的起始位置
        switch (checkstate)                     // 主状态机状态
        {
            // 第一种状态：分析请求行ing
        case CHECK_STATE::CHECK_STATE_REQUESTLINE:
            ret_code = ParseRequestLine(tmp, checkstate);
            if (ret_code == HTTP_CODE::BAD_REQUEST)
            {
                return HTTP_CODE::BAD_REQUEST;
            }
            break;
            // 第二种状态：分析请求头部
        case CHECK_STATE::CHECK_STATE_HEADER:
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
            // code......


            break;
        }
    }
    // 没有读到一个完整的行
    if(line_status == LINE_STATE::LINE_OPNE)
    {
        return HTTP_CODE::NO_REQUEST;
    }
    else
    {
        return HTTP_CODE::BAD_REQUEST;
    }
}