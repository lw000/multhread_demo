#pragma once

#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

/*
* 1. 内存池
* 2. 线程池
* 3. 多线程管理
* 4. 锁，条件变量，信号
*/

class MsgQueue {

};

class MsgQueueClient
{
public:
	MsgQueueClient();
	~MsgQueueClient();

public:
	void connect();
};

class MsgQueueServer
{
public:
	MsgQueueServer();
	~MsgQueueServer();

public:
	void start();
	void stop();
};