#include "thread_pool.h"
#include <iostream>


ThreadStdLogging::ThreadStdLogging() : need_quit(0), running(0), mainThread(nullptr) {
	
}

ThreadStdLogging::~ThreadStdLogging() {
	stop();
}

void ThreadStdLogging::debug(const std::string &msg) {
	_msg(LEVEL::DEBUG, msg);
}

void ThreadStdLogging::info(const std::string& msg) {
	_msg(LEVEL::INFO, msg);
}

void ThreadStdLogging::err(const std::string& msg) {
	_msg(LEVEL::ERROR, msg);
}

void ThreadStdLogging::start() {
	if (running == 1) {
		return;
	}
	running = 1;
	mainThread = std::make_unique<std::thread>(&ThreadStdLogging::run, this);
}

void ThreadStdLogging::stop() {
	need_quit = 1;
	cv.notify_one();

	mainThread->join();

	running = 0;
}

void ThreadStdLogging::_msg(LEVEL level, const std::string& msg) {
	std::string text;
	switch (level)
	{
	case ThreadStdLogging::LEVEL::DEBUG:
		text = std::move(std::string("debug: " + msg));
		break;
	case ThreadStdLogging::LEVEL::INFO:
		text = std::move(std::string("info: " + msg));
		break;
	case ThreadStdLogging::LEVEL::ERROR:
		text = std::move(std::string("error: " + msg));
		break;
	default:
		text = std::move(std::string("debug: " + msg));
		break;
	}

	{
		std::unique_lock<std::mutex> lock(this->mu);
		this->logCaches.emplace_back(text);
	}

	this->cv.notify_one();
}

void ThreadStdLogging::run() {
	while (1)
	{
		emptyWait();

		if (this->need_quit == 1) {
			break;
		}

		// 获取队列数据，分发给work线程
		std::string msg;
		{
			std::unique_lock<std::mutex> lock(this->mu);
			msg = std::move(this->logCaches.front());
			this->logCaches.pop_front();
		}
		
		std::cout << msg << std::endl;
	}
}

void ThreadStdLogging::emptyWait()
{
	std::unique_lock<std::mutex> lock(this->mu);
	while (this->logCaches.empty() && this->need_quit == 0)
	{
		this->cv.wait(lock);
		if (this->need_quit == 1) {
			break;
		}
	}
}

ThreadPool::ThreadPool(const ThreadPoolOption& option) : thread_num(0),
	running(0),
	main_thread(nullptr)
{
	thread_data.need_quit = 0;
	this->option.logging = option.logging;
}

ThreadPool::~ThreadPool() {
	stop();
}

void ThreadPool::start(int thread_num) {
	if (running == 1) {
		return;
	}

	running = 1;

	if (this->thread_num == 0) {
		this->thread_num = std::thread::hardware_concurrency();
	}

	main_thread = std::make_unique<std::thread>(&ThreadPool::run, this);

	for (int w = 0; w < this->thread_num; w++) {
		work_threads.emplace_back(std::make_unique<std::thread>(&ThreadPool::work_run, this));
	}
}

void ThreadPool::stop() {
	thread_data.need_quit = 1;

	running = 0;

	cv.notify_all();
	work_cv.notify_all();

	main_thread->join();
	
	for (int i = 0; i < work_threads.size(); ++i) {
		work_threads[i]->join();
	}

	while (!work_threads.empty())
	{
		work_threads.pop_back();
	}
}

void ThreadPool::postTask(ThreadTask* data) {
	if (running == 0) {
		return;
	}

	if (thread_data.need_quit == 1) {
		return;
	}

	std::unique_lock<std::mutex> lock(this->data_queue_mu);
	this->data_queue.emplace_back(data);
	this->cv.notify_all();
}

void ThreadPool::run() {
	option.logging->debug("Main Thread Is Running");
	while (1)
	{
		ThreadTask* task = nullptr;
		{
			std::unique_lock<std::mutex> lock(this->data_queue_mu);
			while (this->data_queue.empty() && this->thread_data.need_quit == 0)
			{
				this->cv.wait(lock);
				
				if (this->thread_data.need_quit == 1) {
					break;
				}
			}

			if (this->thread_data.need_quit == 1) {
				break;
			}

			// 获取队列数据，分发给work线程
			task = this->data_queue.front();
			this->data_queue.pop_front();
		}

		if (task != nullptr)
		{
			std::unique_lock<std::mutex> lock(this->work_queue_mu);
			this->work_queue.emplace_back(task);
			this->work_cv.notify_all();
		}
	}

	option.logging->debug("Main Thread Is Leaving");
}

void ThreadPool::work_run() {
	option.logging->debug("Work Thread Is Running");
	while (1)
	{	
		auto start = std::chrono::system_clock::now();

		{
			std::unique_lock<std::mutex> lock(this->work_queue_mu);
			while (this->work_queue.empty() && this->thread_data.need_quit == 0)
			{
				this->work_cv.wait(lock);

				if (this->thread_data.need_quit == 1) {
					break;
				}
			}
		}

		if (this->thread_data.need_quit == 1) {
			break;
		}

		// 获取队列数据执行任务
		ThreadTask* t = nullptr;
		{
			std::unique_lock<std::mutex> lock(this->work_queue_mu);
			if (!this->work_queue.empty()) {
				t = this->work_queue.front();
				this->work_queue.pop_front();
			}
		}

		if (t != nullptr) {
			try
			{
				std::unique_ptr<ThreadTask> task(t);
				task->run();
			}
			catch (const std::exception& e)
			{
				std::cerr << "exception " << e.what() << std::endl;
				abort();
			}
			catch (...)
			{
				std::cerr << "unkonow exception abort." << std::endl;
				abort();
			}
		}
		auto end = std::chrono::system_clock::now();
		
		std::cout << std::chrono::duration<double, std::milli>(end - start).count() << std::endl;
	}

	option.logging->debug("Work Thread Is Leaving");
}

ThreadPoolOption WithStdLogging(BaseLogging* logging)
{
	ThreadPoolOption option{ logging };

	return option;
}