#pragma once

#include <vector>
#include <string>

namespace orm {

enum class Op { Eq, Ne, Lt, Le, Gt, Ge };
enum class Order { Asc, Desc };

template<typename T>
class QueryBuilder {
public:
    QueryBuilder& where(const char* member, Op op, const std::string& value);
    QueryBuilder& orderBy(const char* member, Order o);
    QueryBuilder& limit(size_t n);
    std::vector<T> all();
    std::optional<T> one();
};

} // namespace orm
