#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <type_traits>
#include <chrono>
#include "mysql_driver.hpp"

namespace orm {

class ConnectionPool;
class Transaction;
template<typename T>
class QueryBuilder;

// Primary key type trait helper；默认 long，可为类型特化
template<typename T>
using PrimaryKeyType = long;

class Session {
public:
    explicit Session(std::shared_ptr<ConnectionPool> pool);
    ~Session();

    // 查找主键对应实体（按值返回）
    template<typename T>
    std::optional<T> find(const PrimaryKeyType<T>& id);

    // 保存实体：insert 或 update（根据主键）
    template<typename T>
    void save(T& obj);

    // 删除主键记录
    template<typename T>
    void remove(const PrimaryKeyType<T>& id);

    // 获取 QueryBuilder（绑定到该 Session）
    template<typename T>
    QueryBuilder<T> query();

    // 开始事务（返回 RAII Transaction）
    std::unique_ptr<Transaction> beginTransaction();

    std::shared_ptr<IConnection> acquireConnection(std::chrono::milliseconds timeout = std::chrono::milliseconds{3000});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
