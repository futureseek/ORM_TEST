#pragma once

#include <memory>
#include "session.hpp"

namespace orm {

class Session;
class IConnection;
class Transaction {
public:
    explicit Transaction(Session& s);
    Transaction(Session& s,std::shared_ptr<IConnection> conn);
    ~Transaction();

    // 提交事务；若失败抛出 orm::DBException
    void commit();

    // 回滚事务；可在异常处理中调用
    void rollback();

private:
    struct Impl;
    Session* session_;
    //std::unique_ptr<Impl> impl_;
    std::shared_ptr<IConnection> conn_;
    bool committed_;
};

} // namespace orm
