#include "orm/transaction.hpp"
#include "orm/exceptions.hpp"
#include "orm/mysql_driver.hpp"

using namespace orm;


Transaction::Transaction(Session& s, std::shared_ptr<IConnection> conn)
    : session_(&s), conn_(std::move(conn)), committed_(false) {
    conn_->beginTransaction();
}

Transaction::~Transaction() {
    if (!committed_) {
        try { conn_->rollback(); } catch (...) {}
    }
}

void Transaction::commit() {
    conn_->commit();
    committed_ = true;
}

void Transaction::rollback() {
    conn_->rollback();
    committed_ = false;
}
