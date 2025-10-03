#pragma once

#include <string>
#include <memory>
#include <vector>

namespace orm {

class IResultSet;
class IPreparedStatement;

class IConnection {
public:
    virtual ~IConnection() = default;
    virtual std::unique_ptr<IPreparedStatement> prepare(const std::string& sql) = 0;
    virtual void execute(const std::string& sql) = 0;
    virtual void beginTransaction() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

class IPreparedStatement {
public:
    virtual ~IPreparedStatement() = default;
    virtual void bind(size_t index, const std::string& value) = 0;
    virtual void bind(size_t index, int value) = 0;
    virtual void bind(size_t index, long value) = 0;
    virtual std::unique_ptr<IResultSet> executeQuery() = 0;
    virtual size_t executeUpdate() = 0;
};

class IResultSet {
public:
    virtual ~IResultSet() = default;
    virtual bool next() = 0;
    virtual std::string getString(size_t col) = 0;
    virtual int getInt(size_t col) = 0;
    virtual long getLong(size_t col) = 0;
};

// Factory helpers for mysqlcppconn-backed connection implementation
std::shared_ptr<IConnection> createMysqlConnection(const std::string& conn_uri);

} // namespace orm
