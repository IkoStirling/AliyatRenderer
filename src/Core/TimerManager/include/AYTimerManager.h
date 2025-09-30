#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
    废案，与现实事件相关，与游戏时间脱离
*/

class TimerHandle {
public:
    void Cancel() {
        if (timer_) {
            std::cout << "Cancelling timer...\n";
            timer_->cancel();
            timer_.reset();
        }
    }

private:
    friend class TimerManager;
    explicit TimerHandle(std::shared_ptr<boost::asio::steady_timer> timer)
        : timer_(std::move(timer)) {
    }

    std::shared_ptr<boost::asio::steady_timer> timer_;
};

class TimerManager {
public:
    TimerManager(size_t thread_count = 1) 
        : work_guard_(boost::asio::make_work_guard(io_context_)) 
    {
        std::cout << "Creating io_context...\n";
        std::atomic<int> threads_ready = 0;
        // 初始化 io_context 和线程池
        boost::asio::post(io_context_, [] {});
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this, &threads_ready]() {
                std::cout << "IO thread started: " << std::this_thread::get_id() << "\n";
                threads_ready++;
                io_context_.run();
                std::cout << "IO thread exited: " << std::this_thread::get_id() << "\n";
                });
        }
        while (threads_ready < thread_count) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "io_context created.\n";
    }

    ~TimerManager() {
        std::cout << "Stopping io_context...\n";
        work_guard_.reset();
        io_context_.stop();  // 停止 io_context
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        std::cout << "io_context stopped.\n";
    }

    TimerHandle StartTimer(int milliseconds, std::function<void()> callback) {
        std::cout << "StartTimer!\r\n";
        auto timer = std::make_shared<boost::asio::steady_timer>(io_context_,
            std::chrono::milliseconds(milliseconds));

        timer->async_wait([timer, callback](const boost::system::error_code& ec) {
            std::cout << "exec StartTimer!\r\n";
            if (!ec) {
                callback();
            }
        });

        return TimerHandle(timer);
    }

    TimerHandle StartRepeatingTimer(int milliseconds, std::function<void()> callback) {
        std::cout << "StartRepeatingTimer!\r\n";
        auto timer = std::make_shared<boost::asio::steady_timer>(io_context_,
            std::chrono::milliseconds(milliseconds));

        auto repeat = std::make_shared<std::function<void(const boost::system::error_code & ec)>>(nullptr);
        *repeat = [timer, callback, milliseconds, repeat](const boost::system::error_code& ec) {
            std::cout << "exec StartRepeatingTimer!\r\n";
            if (!ec)
            {
                callback();
                timer->expires_after(std::chrono::milliseconds(milliseconds));
                timer->async_wait(*repeat);
            }
        };

        timer->async_wait(*repeat);
        return TimerHandle(timer);
    }

private:
    boost::asio::io_context io_context_;
    std::vector<std::thread> threads_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};

