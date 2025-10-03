#pragma once

#include <tuple>
#include <string>

namespace orm {

// Field descriptor，用于元编程风格的映射
struct FieldOpts {
    bool primary = false;
    bool nullable = true;
};

struct Field {
    const char* name;
    // 指向成员的指针，用 template 元信息封装时可保持泛型
    const void* member_ptr = nullptr; // 占位，实际通过模板提取
    FieldOpts opts;
};

// 用户需为每个 Model 特化 metadata<T>()，返回 tuple of Field-like 描述
template<typename T>
constexpr auto metadata() {
    static_assert(sizeof(T) == 0, "metadata<T> must be specialized for your model type");
}

} // namespace orm
