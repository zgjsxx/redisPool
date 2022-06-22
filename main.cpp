#include <pthread.h>
#include <unistd.h>
#include "RedisPool.h"
#include "ServerLog.h"


void* thread_get(void* param)
{
    string value;
    RedisPool* redisClient = static_cast<RedisPool*>(param); 
    for(int i = 0;i < 10; i++)
    {
        redisClient->Get("hello1", value);
        LOG_DEBUG("value is %s", value.c_str());
    }
}

void* thread_set(void* param)
{
    RedisPool* redisClient = static_cast<RedisPool*>(param); 
    for(int i = 0;i < 10; i++)
    {
        redisClient->Set("hello1","world");
        LOG_DEBUG("set value");
    }
}

void* thread_monitor(void* param)
{
    RedisPool* redisClient = static_cast<RedisPool*>(param); 
    for(int i = 0;i < 10; i++)
    {
        LOG_DEBUG("the idle connection num is %d", redisClient->len());
        sleep(0.05);
    }
}

int main()
{
    LOG_DEBUG("redis client is starting..");
    LOG_INFO("redis client is starting..");
    LOG_WARN("redis client is starting..");
    LOG_ERROR("redis client is starting..");
    RedisPool* redisClient = RedisPool::GetInstance();
    if(nullptr == redisClient)
    {
        LOG_DEBUG("redis client is null");
        return 0;
    }
    redisClient->Init("127.0.0.1", 6379, "none", 2, 10);
    LOG_DEBUG("the idle connection num is %d", redisClient->len());
    pthread_t th_monitor;
    pthread_create(&th_monitor, nullptr, thread_monitor, (void*)redisClient);
    pthread_t th[10];
    for(int i = 0;i < 10; i++)
    {
        pthread_create(&th[i], nullptr, thread_set, (void*)redisClient);
    }
    
    pthread_t th2[10];
    for(int i = 0;i < 10; i++)
    {
        pthread_create(&th2[i], nullptr, thread_get, (void*)redisClient);
    }


    int time = 10;
    while(time--)
    {
        //LOG_DEBUG("the idle connection num is %d", redisClient->len());
        sleep(2);
    }

    LOG_DEBUG("the idle connection num is %d", redisClient->len());
	void* ret = nullptr;
	for (int i = 0; i < 10; i++) 
    {
		pthread_join(th[i], &ret);
	}
 	for (int i = 0; i < 10; i++) 
    {
		pthread_join(th2[i], &ret);
	}
    delete redisClient;
    redisClient = nullptr;
    delete ServerLog::get();
}