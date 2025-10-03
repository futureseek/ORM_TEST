#pragma once

#include "model_meta.hpp"
#include "session.hpp"
#include "query_builder.hpp"
#include "transaction.hpp"
#include "connection_pool.hpp"
#include "mysql_driver.hpp"
#include "exceptions.hpp"

namespace orm {

// 顶层简易导出
inline constexpr const char ORM_VERSION[] = "0.1.0";
}
