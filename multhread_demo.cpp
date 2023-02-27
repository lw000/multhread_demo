// multhread_demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <future>
#include <string>
#include <vector>
#include <list>
#include <cstdarg>

#include "core/thread_pool.h"

using ThreeSeconds = std::chrono::duration<float, std::ratio<3, 1>>;

// 用户定义字面量
long double operator"" _mm(long double x) { return x / 1000; }
long double operator"" _Pa(long double n) { return n * 3.1415926; }

typedef struct TestData {
    int id;
    std::string s;

    TestData(int id, const std::string s) : id(id), s(s) {

    }
    
    ~TestData() {
        std::cout << "~TestData() " << id << " " << s << std::endl;
    }

} TestData;

int main()
{
    {
        long double ff = 1.0_mm;

        std::cout << ff << std::endl; //0.001
    }

    {
        long double ff = 2.0_Pa;

        std::cout << ff << std::endl;
    }


    {
        std::vector<std::unique_ptr<TestData>> vs;
        vs.reserve(100);
        vs.emplace_back(std::make_unique<TestData>(10, "test"));
        vs.emplace_back(std::make_unique<TestData>(11, "test"));
        vs.emplace_back(std::make_unique<TestData>(12, "test"));
        vs.emplace_back(std::make_unique<TestData>(13, "test"));
        //while (!vs.empty())
        //{
        //    vs.pop_back();
        //}

        vs.clear();
    }

    {
        std::list<std::unique_ptr<TestData>> vs;
        vs.emplace_back(std::make_unique<TestData>(10, "test"));
        vs.emplace_back(std::make_unique<TestData>(11, "test"));
        vs.emplace_back(std::make_unique<TestData>(12, "test"));
        vs.emplace_back(std::make_unique<TestData>(13, "test"));
        /*while (!vs.empty())
        {
            vs.pop_front();
        }*/
        vs.erase(vs.begin(), vs.end());
    }

    std::unique_ptr<ThreadStdLogging> stdLogging = std::make_unique<ThreadStdLogging>();
    stdLogging->start();

    TestData testData(10, "test");

    {
        ThreadPool pool(WithStdLogging(stdLogging.get()));
        pool.start();
        
        std::thread t1(([&]() {
                stdLogging->debug("work start >>>>>>>>>>> 1");
                int index = 0;
                do {
                    ThreadTask* t = new ThreadTask([&](void* userData) {
                        stdLogging->debug("working >>>>>>>>>>> 1");
                        }, (void*)&testData);
                    pool.postTask(t);
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                } while (index++ < 1000);

                stdLogging->debug("work end >>>>>>>>>>> 1");
            }));

        std::thread t2([&]() {
                stdLogging->debug("work start >>>>>>>>>>> 2");
                int index = 0;
                do {
                    ThreadTask* t = new ThreadTask([&](void* userData) {
                        stdLogging->debug("working >>>>>>>>>>> 2");
                        }, (void*)&testData);
                    pool.postTask(t);
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                } while (index++ < 1000);

                stdLogging->debug("work end >>>>>>>>>>> 2");
            });

        t1.join();
        t2.join();
        
        std::cin.get();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}