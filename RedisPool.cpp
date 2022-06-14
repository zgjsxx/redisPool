#include <unistd.h>
#include <string.h>
#include "defer.h"
#include "RedisPool.h"
#include "ServerLog.h"

const int MAX_RETRY_TIME = 3;

std::mutex mux;
static string toString(int type)
{
	switch(type)
	{
		case REDIS_REPLY_STATUS:
			return "status reply";
		case REDIS_REPLY_ERROR:
			return "error reply";
		case REDIS_REPLY_STRING:
			return "string reply";
		case REDIS_REPLY_INTEGER:
			return "int reply";
		case REDIS_REPLY_ARRAY:
			return "array reply";
		case REDIS_REPLY_NIL:
			return "nil reply";
		default:
			return "invalid reply";
	}
}

RedisPool* RedisPool::m_pInstance = NULL;
RedisPool::RedisPool()
{

}

RedisPool::~RedisPool()
{
	std::lock_guard<std::mutex> lck(m_mtx);
	while(!m_conns.empty())
	{
		redisContext *ctx = m_conns.front();
		redisFree(ctx);
		m_conns.pop();
	}
}

RedisPool* RedisPool::GetInstance()
{
	if(nullptr == m_pInstance)
	{
		mux.lock();
		if(nullptr == m_pInstance)
		{
			m_pInstance = new RedisPool();
		}
		mux.unlock();
	}

	return m_pInstance;
}
	
void RedisPool::Init(string host, int port, string pwd, int timeout, int poolsize)
{
	m_host = host;
	m_port = port;
	m_pwd = pwd;
	m_timeout = timeout;
	m_poolSize = poolsize;
	initIdleConn();
}

int RedisPool::initIdleConn()
{
	//init connection in the pool
	{
		std::lock_guard<std::mutex> lck(m_mtx);
		for(int i = 0;i < m_poolSize; i++)
		{
			redisContext *ctx = createNewConn();
			if(ctx != nullptr)
			{
				m_conns.push(ctx);
			}
		}
	}

	if(m_conns.size() < m_poolSize)
	{
		//init failed
		return -1;
	}

	return 0;
}

bool RedisPool::isStillConnected(redisContext* conn)
{
    redisReply *reply = nullptr;
    reply = (redisReply*)redisCommand(conn, "PING");
    if(reply != nullptr && 
            reply->type != REDIS_REPLY_ERROR) 
    {
        if(!strcmp(reply->str,"PONG"))
        {
            LOG_DEBUG("connection is valid, can still use it");
            freeReplyObject(reply);
            return true;
        }
    }
	LOG_DEBUG("connection seems has some problem, we need to reconnect");
    return false;
}

redisContext* RedisPool::getConnWithRetryTime(int retryTime)
{
	redisContext* conn = nullptr;
	conn = getConnFromPool();
	LOG_DEBUG("get connection from pool");
	if(!conn)
	{
		//cannot get connection;
		return nullptr;
	}
    if(isStillConnected(conn))
    {
        //conn is still valid , just use it
        return conn;
    }
    else
    {
        //conn is lost at a certain time, need to reconnect it 
		while(retryTime--)
		{
			int ret = redisReconnect(conn);
			if(ret < 0)
			{
				if(retryTime == 0)
				{
					//reach max retry time
					LOG_DEBUG("reach max reconnect times, return null");
					return nullptr;
				}
				
				sleep(2);
				continue;
			}
			else
			{
				//reconnect successful
				LOG_DEBUG("reconnect successfully");
				break;
			}
		}	
		return conn;
	}
}

int RedisPool::len()
{
	return m_conns.size();
}

int RedisPool::Get(const string& key, string& value) {
	redisContext* conn = getConnWithRetryTime(MAX_RETRY_TIME);
	if(!conn)
	{
		//cannot get connection
		return -1;
	}
	redisReply *reply = nullptr;
	LOG_DEBUG("the idle connection num is %d", len());
	
	defer ( [&] {
			LOG_DEBUG("return to pool");
			returnConn(conn);
			if(reply != nullptr){
				freeReplyObject(reply);
			}
		}
	);
	// redisReply *reply = (redisReply*)redisCommand(m_pRedisContext, "AUTH %s", m_pwd.c_str());
	// if (nullptr == reply || reply->type == REDIS_REPLY_ERROR)
	// {
	// 	msg = string("redisCommand AUTH failed, pwd=").append(m_pwd);
	// 	freeReplyObject(reply);
	// 	return -1;
	// }

	reply = (redisReply*)redisCommand(conn, "GET %s", key.c_str());
	if (nullptr == reply
			|| reply->type == REDIS_REPLY_ERROR
			|| nullptr == reply->str)
	{
		return -1;
	}
	LOG_DEBUG("redis reply type: %s",toString(reply->type).c_str());
	value = reply->str;	

	return 0;
}

int RedisPool::Set(const string &key, const string& value) {
	redisContext* conn = getConnWithRetryTime(MAX_RETRY_TIME);
	if(!conn)
	{
		//cannot get connection
		return -1;
	}
	redisReply *reply = nullptr;
	defer ( [&] {
			LOG_DEBUG("return to pool");
			returnConn(conn);
			if(reply != nullptr){
				freeReplyObject(reply);
			}
		}
	);
	LOG_DEBUG("begin redis SetValue, key=%s , value = %s", key.c_str(), value.c_str());

	// redisReply *reply = (redisReply*)redisCommand(m_pRedisContext, "AUTH %s", m_pwd.c_str());
	// if (nullptr == reply || reply->type == REDIS_REPLY_ERROR)
	// {
	// 	msg = "redisCommand AUTH failed";
	// 	freeReplyObject(reply);
	// 	return -1;
	// }

	
	reply = (redisReply*)redisCommand(conn, "SET %s %s", key.c_str(), value.c_str());
	if (nullptr == reply || reply->type == REDIS_REPLY_ERROR) 
	{
		// msg = string("redisCommand GET failed, redisCommand=").append(cmd);
		return -1;
	}
	LOG_DEBUG("end redis SetValue, key=%s", key.c_str());
	return 0;
}

redisContext* RedisPool::getConnFromPool()
{
	std::lock_guard<std::mutex> lck(m_mtx);
	if(!m_conns.empty())
	{
		redisContext *ctx = m_conns.front();
		m_conns.pop();
		return ctx;
	}
	else
	{
		redisContext *ctx = createNewConn();
		return ctx;
	}
}

int RedisPool::returnConn(redisContext *conn)
{
	if(nullptr == conn)
	{
		return 0;
	}

	{
		//in the pool
		std::lock_guard<std::mutex> lck(m_mtx);
		if(m_conns.size() < m_poolSize)
		{
			m_conns.push(conn);
		}
		else
		{
			redisFree(conn);
		}
	}
	return 0;
}

redisContext* RedisPool::createNewConn()
{
	struct timeval tv;
	tv.tv_sec = m_timeout;//second /s
	tv.tv_usec = 0;
	redisContext* conn = redisConnectWithTimeout(m_host.c_str(), m_port, tv);

	if (conn == nullptr || conn->err)
	{
		if (conn)
		{
			LOG_DEBUG("Connection error: %s", conn->errstr);
			redisFree(conn);
		}
		else
		{
			LOG_DEBUG("Connection error: can't allocate redis context");
		}
		LOG_ERROR("cannot establish connection to redis");
		return nullptr;
	}
	return conn;
}