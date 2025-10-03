#pragma once

#include <stdexcept>
#include <string>

namespace orm {

struct ORMException : public std::runtime_error {
    explicit ORMException(const std::string& msg) : std::runtime_error(msg) {}
};

struct DBException : public ORMException {
    explicit DBException(const std::string& msg) : ORMException(msg) {}
};

struct MappingException : public ORMException {
    explicit MappingException(const std::string& msg) : ORMException(msg) {}
};

} // namespace orm
