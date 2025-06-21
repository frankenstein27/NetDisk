#pragma once 


#include <string>


class DBConnector
{
public:
    DBConnector(/* args */);
    ~DBConnector();

    DBResult ExecuteQuery(const std::string& sql);
    DBStatement PrepareStatement(const std::string &sql);

private:
    /* data */
};

class DBResult
{
public:
    // 移动游标到下一行
    bool Next();
    int GetInt(int col);
    std::string GetString();
    size_t GetColumnCount();
    size_t GetRowCount();

private:
};

class DBStatement
{
public:
    DBStatement();
    ~DBStatement();

    // 绑定整数参数
    void BindInt(int index, int value);
    // 绑定字符串参数
    void BindString(int index, std::string value);
    // 执行语句并返回结果
    DBResult *Execute();
    void Reset();

private:
    
};