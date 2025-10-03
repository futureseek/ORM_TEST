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