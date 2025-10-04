#include "orm/session.hpp"
#include "orm/exceptions.hpp"
#include "orm/model_meta.hpp"
#include "orm/mysql_driver.hpp"
#include "orm/connection_pool.hpp"
#include "orm/transaction.hpp"

#include <sstream>

using namespace orm;

struct Session::Impl {
    std::shared_ptr<ConnectionPool> pool;
};

Session::Session(std::shared_ptr<ConnectionPool> pool) : impl_(new Impl()) {
    impl_->pool = std::move(pool);
}

Session::~Session() = default;

std::unique_ptr<Transaction> Session::beginTransaction() {
    auto conn = impl_->pool->get(std::chrono::milliseconds{3000});
    return std::unique_ptr<Transaction>(new Transaction(*this, conn));
}

std::shared_ptr<IConnection> Session::acquireConnection(std::chrono::milliseconds timeout) {
    return impl_->pool->get(timeout);
}
