#pragma once

#include <tuple>
#include <string>
#include <type_traits>

namespace orm {

// 字段选项
struct FieldOpts {
    bool primary = false;
    bool nullable = true;
    std::string column_name;
};

// Member pointer 类型别名（用于保持类型信息）
template<typename T, typename M>
using MemberPtr = M T::*;

// FieldInfo 模板，包含成员指针与元数据
template<typename T, typename M>
struct FieldInfo {
    const char* name;                // 字段名（成员名）
    MemberPtr<T, M> member;          // 成员指针
    FieldOpts opts;
};

// metadata<T> 必须被用户特化，返回一个 tuple 包含 FieldInfo<T, ...>...
template<typename T>
constexpr auto metadata() {
    static_assert(sizeof(T) == 0, "metadata<T> must be specialized for your model type");
}

// 帮助类型：从 FieldInfo tuple 中提取信息的工具在实现文件中提供

} // namespace orm
