#pragma once


class TokenBucket
{
public:
    TokenBucket();
    ~TokenBucket();

    TokenBucket(double capacity, double refill_rate);
    void Refill();              // 根据时间更新令牌数量
    bool Consume(double tokens);
    void SetCapacity(double capacity);
    void SetRefillRate(double refill_rate);

private:
    double capacity_;           // 桶的最大容量（令牌数量）
    double tokens_;             // 当前桶中的令牌数量
    double refill_rate_;        // 每秒令牌补充速率（单位：令牌/秒）
    double last_refill_time_;   // 上次补充令牌的时间戳（秒）
};