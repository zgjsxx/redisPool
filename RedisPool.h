#pragma once
#include <string>
#include <queue>
#include <mutex>
#include "hiredis.h"
using std::string;

class RedisPool
{
public:
    RedisPool();
    ~RedisPool();

public:
    static RedisPool* GetInstance();

public:
    void Init(string host, int port, string pwd, int timeout, int poolsize);
    redisContext* getConnFromPool();
    redisContext* createNewConn();
    redisContext* getConnWithRetryTime(int retryTime);
    bool isStillConnected(redisContext* conn);
    int returnConn(redisContext *conn);
    int initIdleConn();
    int len();

public:
    int Get(const string& key, string& value);
    int Set(const string& key, const string& value);

private:
    static RedisPool* m_pInstance;
    string m_host;
    int m_port;
    string m_pwd;
    std::queue<redisContext *>m_conns;
    std::mutex m_mtx;
    int m_timeout;
    int m_poolSize;
};
