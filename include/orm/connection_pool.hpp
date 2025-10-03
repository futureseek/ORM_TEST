#pragma once

#include <memory>
#include <string>
#include <chrono>

namespace orm {

class IConnection;

class ConnectionPool : public std::enable_shared_from_this<ConnectionPool> {
public:
    struct Options { size_t max_size = 10; int timeout_ms = 3000; };

    static std::shared_ptr<ConnectionPool> createMysql(const std::string& conn_str, Options opts);

    // RAII wrapper for a connection
    class Guard {
    public:
        Guard(std::shared_ptr<ConnectionPool> parent, std::shared_ptr<IConnection> conn);
        ~Guard();
        IConnection* operator->();
        IConnection& get();
    private:
        std::shared_ptr<IConnection> conn_;
        std::shared_ptr<ConnectionPool> parent_;
    };

    // 获取连接，超时则抛出 orm::DBException
    std::unique_ptr<Guard> get(std::chrono::milliseconds timeout = std::chrono::milliseconds{3000});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
