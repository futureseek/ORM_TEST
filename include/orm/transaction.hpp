#pragma once

#include <memory>

namespace orm {

class Session;

class Transaction {
public:
    explicit Transaction(Session& s);
    ~Transaction();

    void commit();
    void rollback();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
