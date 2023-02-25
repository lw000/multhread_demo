// multhread_demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <future>
#include <string>
#include <vector>

#include "core/thread_pool.h"

typedef struct TestData {
    int id;
    std::string s;

    TestData(int id, const std::string s) : id(id), s(s) {

    }
    
    ~TestData() {

    }

} TestData;

int main()
{
    {
        std::vector<std::unique_ptr<TestData>> vs;
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        vs.push_back(std::make_unique<TestData>(10, "test"));
        while (!vs.empty())
        {
            vs.pop_back();
        }
    }

    std::unique_ptr<ThreadStdLogging> std_logging = std::make_unique<ThreadStdLogging>();
    std_logging->start();

    {
        ThreadPool pool(std_logging.get());
        pool.start();

        TestData testData(10, "test");

        std::thread t1([&]() {
                std_logging->debug("work start >>>>>>>>>>> 1");
                int index = 0;
                do {
                    ThreadTask* t = new ThreadTask([&](void* userData) {
                        std_logging->debug("working >>>>>>>>>>> 1");
                        }, (void*)&testData);
                    pool.postTask(t);
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                } while (index++ < 1000);

                std_logging->debug("work end >>>>>>>>>>> 1");
            });

        std::thread t2([&]() {
                std_logging->debug("work start >>>>>>>>>>> 2");
                int index = 0;
                do {
                    ThreadTask* t = new ThreadTask([&](void* userData) {
                        std_logging->debug("working >>>>>>>>>>> 2");
                        }, (void*)&testData);
                    pool.postTask(t);
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                } while (index++ < 1000);

                std_logging->debug("work end >>>>>>>>>>> 2");
            });

        t1.join();
        t2.join();
        std::cin.get();
    }
    

    std::this_thread::sleep_for(std::chrono::seconds(1));
}