#pragma once

#include <vector>
#include <string>
#include <optional>

namespace orm {

enum class Op { Eq, Ne, Lt, Le, Gt, Ge };
enum class Order { Asc, Desc };

template<typename T>
class QueryBuilder {
public:
    // member: pointer-to-member, e.g. &T::age
    template<typename M>
    QueryBuilder& where(M T::* member, Op op, const std::string& value);
    template<typename M>
    QueryBuilder& orderBy(M T::* member, Order o);
    QueryBuilder& limit(size_t n);
    std::vector<T> all();
    std::optional<T> one();
};

} // namespace orm
