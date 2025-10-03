#include "orm/mysql_driver.hpp"
#include "orm/exceptions.hpp"

#include <memory>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>

using namespace orm;

namespace {

class MysqlResultSet : public IResultSet {
public:
    MysqlResultSet(std::unique_ptr<sql::ResultSet> rs) : rs_(std::move(rs)) {}
    bool next() override { return rs_->next(); }
    std::string getString(size_t col) override { return rs_->getString((unsigned int)col); }
    int getInt(size_t col) override { return rs_->getInt((unsigned int)col); }
    long getLong(size_t col) override { return rs_->getInt64((unsigned int)col); }
private:
    std::unique_ptr<sql::ResultSet> rs_;
};

class MysqlPreparedStatement : public IPreparedStatement {
public:
    MysqlPreparedStatement(std::unique_ptr<sql::PreparedStatement> ps) : ps_(std::move(ps)) {}
    void bind(size_t index, const std::string& value) override { ps_->setString((unsigned int)index, value); }
    void bind(size_t index, int value) override { ps_->setInt((unsigned int)index, value); }
    void bind(size_t index, long value) override { ps_->setInt64((unsigned int)index, value); }
    std::unique_ptr<IResultSet> executeQuery() override { return std::unique_ptr<IResultSet>(new MysqlResultSet(std::unique_ptr<sql::ResultSet>(ps_->executeQuery()))); }
    size_t executeUpdate() override { return (size_t)ps_->executeUpdate(); }
private:
    std::unique_ptr<sql::PreparedStatement> ps_;
};

class MysqlConnection : public IConnection {
public:
    explicit MysqlConnection(std::unique_ptr<sql::Connection> c) : conn_(std::move(c)) {}
    std::unique_ptr<IPreparedStatement> prepare(const std::string& sql) override {
        try {
            return std::unique_ptr<IPreparedStatement>(new MysqlPreparedStatement(std::unique_ptr<sql::PreparedStatement>(conn_->prepareStatement(sql))));
        } catch (sql::SQLException &e) {
            throw orm::DBException(e.what());
        }
    }
    void execute(const std::string& sql) override {
        try { std::unique_ptr<sql::Statement> st(conn_->createStatement()); st->execute(sql); } catch (sql::SQLException &e) { throw orm::DBException(e.what()); }
    }
    void beginTransaction() override { conn_->setAutoCommit(false); }
    void commit() override { conn_->commit(); conn_->setAutoCommit(true); }
    void rollback() override { conn_->rollback(); conn_->setAutoCommit(true); }
private:
    std::unique_ptr<sql::Connection> conn_;
};

} // namespace

std::shared_ptr<IConnection> orm::createMysqlConnection(const std::string& conn_uri) {
    try {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        // conn_uri expected formats: "tcp://host:port" or "tcp://host:port;user;pass" or full jdbc-style
        std::string host = conn_uri;
        std::string user = "";
        std::string pass = "";
        // simple parse: if contains ';' treat as host;user;pass
        size_t p1 = conn_uri.find(';');
        if (p1 != std::string::npos) {
            host = conn_uri.substr(0, p1);
            size_t p2 = conn_uri.find(';', p1 + 1);
            if (p2 != std::string::npos) {
                user = conn_uri.substr(p1 + 1, p2 - (p1 + 1));
                pass = conn_uri.substr(p2 + 1);
            } else {
                user = conn_uri.substr(p1 + 1);
            }
        }
        sql::Connection* raw = nullptr;
        if (!user.empty() || !pass.empty()) {
            raw = driver->connect(host, user, pass);
        } else {
            // try connect with host only (some driver versions accept just host)
            raw = driver->connect(host, "", "");
        }
        std::unique_ptr<sql::Connection> rawConn(raw);
        return std::shared_ptr<IConnection>(new MysqlConnection(std::move(rawConn)));
    } catch (sql::SQLException &e) {
        throw orm::DBException(e.what());
    }
}
