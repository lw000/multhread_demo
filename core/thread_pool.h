#pragma once

#include <thread>
#include <string>
#include <mutex>
#include <deque>
#include <vector>
#include <functional>
#include <condition_variable>

class ThreadPool;
class ThreadTask;
class ThreadStdLogging;

using ThreadFunc = std::function<void(void* userData)>;

using ThreadLogCaches = std::deque<std::string>;

using TaksQueueCaches = std::deque<ThreadTask*>;

using WorkThreads = std::vector<std::unique_ptr<std::thread>>;

class noncopyable {
protected:
	noncopyable() = default;
	~noncopyable() = default;

public:
	noncopyable(noncopyable&) = delete;
	const noncopyable& operator = (const noncopyable&) = delete;
};

class BaseLogging {
public:
	virtual void debug(const std::string& msg) = 0;
	virtual void info(const std::string& msg) = 0;
	virtual void err(const std::string& msg) = 0;
};

class ThreadStdLogging : public BaseLogging
{
	enum class LEVEL
	{
		DEBUG = 0,
		INFO = 1,
		ERROR = 2,
	};

public:
	ThreadStdLogging();
	~ThreadStdLogging();

public:
	void start();
	void stop();

public:
	virtual void debug(const std::string& msg) override;
	virtual void info(const std::string& msg) override;
	virtual void err(const std::string& msg) override;

private:
	void run();

	void emptyWait();

	void _msg(LEVEL level, const std::string& msg);

private:
	int need_quit;
	int running;

	std::mutex mu;
	std::condition_variable cv;
	std::unique_ptr<std::thread> mainThread;

	ThreadLogCaches logCaches;
};

class ThreadTask {
public:
	ThreadTask(const ThreadFunc& f, void* userData) : f(f), userData(userData) {

	}

	~ThreadTask() {
		f = nullptr;
		userData = nullptr;
	}

public:
	void setFunc(const ThreadFunc& f, void* userData) {
		this->f = f,
		this->userData = userData;
	}

	void run() {
		f(userData);
	}

private:
	ThreadFunc f;
	void* userData;
};

typedef struct ThreadPoolOption {
	BaseLogging* logging;

}ThreadPoolOption;

class ThreadPool final : public noncopyable
{
	typedef struct {
		int need_quit;
	} ThreadCoreData;

public:
	explicit ThreadPool(const ThreadPoolOption&option);
	~ThreadPool();

public:
	void start(int thread_num = 0);
	void stop();
	void postTask(ThreadTask* data);

private:
	void run();
	void work_run();

private:
	int thread_num;
	int running;

	TaksQueueCaches data_queue;
	TaksQueueCaches work_queue;

	std::mutex data_queue_mu;
	std::mutex work_queue_mu;
	
	std::condition_variable cv;
	std::condition_variable work_cv;

	ThreadCoreData thread_data;

	ThreadPoolOption option;
	std::unique_ptr<std::thread> main_thread;

	WorkThreads work_threads;
};

ThreadPoolOption WithStdLogging(BaseLogging* logging);