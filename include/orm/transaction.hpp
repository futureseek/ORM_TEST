#pragma once

#include <memory>

namespace orm {

class Session;

class Transaction {
public:
    explicit Transaction(Session& s);
    ~Transaction();

    // 提交事务；若失败抛出 orm::DBException
    void commit();

    // 回滚事务；可在异常处理中调用
    void rollback();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
