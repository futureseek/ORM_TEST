#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace orm {

class ConnectionPool;
class Transaction;

class Session {
public:
    explicit Session(std::shared_ptr<ConnectionPool> pool);
    ~Session();

    template<typename T>
    std::optional<T> find(long primary_key);

    template<typename T>
    void save(T& obj);

    template<typename T>
    void remove(long primary_key);

    template<typename T>
    class QueryBuilder query();

    std::unique_ptr<Transaction> beginTransaction();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
