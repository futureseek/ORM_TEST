#pragma once

#include <memory>
#include <string>

namespace orm {

class IConnection;

class ConnectionPool {
public:
    struct Options { size_t max_size = 10; int timeout_ms = 3000; };

    static std::shared_ptr<ConnectionPool> createMysql(const std::string& conn_str, Options opts = {});

    // RAII wrapper for a connection
    class Guard {
    public:
        Guard(std::shared_ptr<IConnection> conn);
        ~Guard();
        IConnection* operator->();
    private:
        std::shared_ptr<IConnection> conn_;
    };

    std::unique_ptr<Guard> get();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace orm
