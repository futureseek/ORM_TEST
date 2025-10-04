#include "orm/connection_pool.hpp"
#include "orm/mysql_driver.hpp"
#include "orm/exceptions.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace orm;

struct ConnectionPool::Impl {
    std::queue<std::shared_ptr<IConnection>> pool;
    std::mutex mtx;
    std::condition_variable cv;
    std::string conn_uri;
    Options opts;
};

std::shared_ptr<ConnectionPool> ConnectionPool::createMysql(const std::string &conn_str, Options opts) {
    auto p = std::shared_ptr<ConnectionPool>(new ConnectionPool());
    p->impl_ = std::unique_ptr<Impl>(new Impl());
    p->impl_->conn_uri = conn_str;
    p->impl_->opts = opts;

    // 初始化连接
    for (size_t i = 0; i < opts.max_size; ++i) {
        auto c = createMysqlConnection(conn_str);
        p->impl_->pool.push(c);
    }
    return p;
}



std::shared_ptr<IConnection> ConnectionPool::get(std::chrono::milliseconds timeout){
    std::unique_lock<std::mutex> lock(impl_->mtx);
    if (!impl_->cv.wait_for(lock, timeout, [this]() { return !impl_->pool.empty(); })) {
        throw orm::DBException("Timeout waiting for connection");
    }
    auto sconn = impl_->pool.front();
    impl_->pool.pop();
    // 创建一个带 custom deleter 的 shared_ptr 返回给调用者。
    // deleter 捕获弱引用到 ConnectionPool（通过 shared_from_this）和 sconn（被推回池中使用的 shared_ptr）
    std::weak_ptr<ConnectionPool> weak_pool = shared_from_this();
    std::shared_ptr<IConnection> client_ptr(
        sconn.get(),
        [weak_pool,sconn](IConnection*) mutable{
            if(auto pool = weak_pool.lock()) {
                std::lock_guard<std::mutex> lock(pool->impl_->mtx);
                pool->impl_->pool.push(sconn);
                pool->impl_->cv.notify_one();
            }
            else{
                // 池已经销毁，sconn 会在 lambda 退出时释放
            }
        }
    );
    return client_ptr;
}


/*

ConnectionPool::Guard::Guard(std::shared_ptr<ConnectionPool> parent, std::shared_ptr<IConnection> conn)
    : conn_(std::move(conn)), parent_(std::move(parent)) {}

ConnectionPool::Guard::~Guard() {
    if (conn_) {
        // 将连接放回池
        std::lock_guard<std::mutex> lock(parent_->impl_->mtx);
        parent_->impl_->pool.push(conn_);
        parent_->impl_->cv.notify_one();
    }
}

IConnection* ConnectionPool::Guard::operator->() { return conn_.get(); }
IConnection& ConnectionPool::Guard::get() { return *conn_.get(); }

std::unique_ptr<ConnectionPool::Guard> ConnectionPool::get(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(impl_->mtx);
    if (!impl_->cv.wait_for(lock, timeout, [this]() { return !impl_->pool.empty(); })) {
        throw orm::DBException("Timeout waiting for connection");
    }
    auto conn = impl_->pool.front();
    impl_->pool.pop();
    return std::unique_ptr<Guard>(new Guard(shared_from_this(), conn));
}
*/