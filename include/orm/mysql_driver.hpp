#pragma once

#include <string>
#include <memory>

namespace orm {

class IConnection {
public:
    virtual ~IConnection() = default;
    virtual void execute(const std::string& sql) = 0;
};

// Factory helpers for mysqlcppconn-backed connection implementation
std::shared_ptr<IConnection> createMysqlConnection(const std::string& conn_uri);

} // namespace orm
