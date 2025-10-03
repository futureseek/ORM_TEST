下面列出仓库中每个头文件（和对应模块）应包含的核心类型、函数和 API 行为说明。你可以根据这些说明在代码中逐个实现。我在每个条目里说明：类型/函数名字建议、职责、输入/输出、错误处理和实现注意点。

> 说明：本文件只描述接口和职责，不实现代码逻辑。

## include/orm/orm.hpp
- 目的：统一导出库的常用符号，作为单个包含点（umbrella header）。
- 内容建议：
  - 包含并 re-export 下列头文件：`model_meta.hpp`、`session.hpp`、`query_builder.hpp`、`transaction.hpp`、`connection_pool.hpp`、`mysql_driver.hpp`、`exceptions.hpp`。
  - 在 `orm` 命名空间中提供版本信息常量（例如 `ORM_VERSION`）。

## include/orm/model_meta.hpp
- 目的：定义元编程映射契约，用户通过特化 `metadata<T>()` 提供类型到表/列的映射。
- 核心类型/API：
  - struct FieldOpts { bool primary; bool nullable; std::string column_name; /* 可选默认值等 */ }：字段选项。
  - template<typename T> struct FieldInfo { const char* name; auto member_ptr; FieldOpts opts; }：表示一个字段的描述（member_ptr 使用模板方式保持类型安全）。
  - template<typename T> constexpr auto metadata() -> tuple<...>：用户为每个模型特化此函数，返回字段描述的 tuple；还应能提供静态 table_name() 或同等函数/trait。
- 职责与注意点：
  - 明确主键字段（primary），并支持复合主键的扩展点（首版可以只要求单主键）。
  - 要提供将 FieldInfo 中保存的成员指针用于读取/写入实体字段的工具函数签名（例如：get_member<T, M>(&T::member) 风格的 helper）。
  - 当没有为类型特化 `metadata<T>` 时，应触发静态断言或在运行时抛 MappingException。

## include/orm/session.hpp
- 目的：对外提供以 Session 为中心的数据库工作单元 API（UnitOfWork）。
- 核心类型/API：
  - class Session { explicit Session(std::shared_ptr<ConnectionPool> pool); ~Session(); }。
  - static Session Session::open(std::shared_ptr<ConnectionPool> pool)（可选工厂）。
  - template<typename T> std::optional<T> Session::find(const typename PrimaryKeyType<T>& id)：根据主键查询并反序列化为 T。
  - template<typename T> void Session::save(T &entity)：插入或更新，根据主键是否存在决定行为；在插入时回填主键到实体。
  - template<typename T> void Session::remove(const typename PrimaryKeyType<T>& id)：删除指定主键记录。
  - template<typename T> QueryBuilder<T> Session::query()：返回绑定到该 Session 的 QueryBuilder，用于构建查询。
  - std::unique_ptr<Transaction> Session::beginTransaction()：开始事务并返回 Transaction RAII 对象。
- 职责与注意点：
  - Session 管理 Identity Map（可选实现），保持同一主键在同一 Session 中返回相同对象或保证一致性。首版可先返回值语义的实体。
  - Session 方法在执行失败时应抛出 `orm::DBException`，包含底层 SQL 错误信息。
  - Session 非线程安全；文档中需明确这一点。

## include/orm/query_builder.hpp
- 目的：提供链式查询构造器，生成 SQL 与绑定参数，然后通过 Session/Connection 执行并映射为实体。
- 核心类型/API：
  - template<typename T> class QueryBuilder {
      QueryBuilder& where(MemberPtr<T> member, Op op, const ValueType& value);
      QueryBuilder& andWhere(...);
      QueryBuilder& orWhere(...);
      QueryBuilder& orderBy(MemberPtr<T> member, Order o);
      QueryBuilder& limit(size_t n);
      QueryBuilder& offset(size_t n);
      std::vector<T> all();
      std::optional<T> one();
      long count();
    };
  - MemberPtr<T> 类型为示例：`auto member = &T::age;` 的类型，用于通过成员指针推导列名。
- 职责与注意点：
  - QueryBuilder 不直接持有数据库连接（或可以持有 Session 的弱引用），在执行时从 Session 获取 Connection。
  - where 等方法应支持复合表达与参数绑定，不直接做字符串拼接。
  - one()/all() 在查询到错误或多行冲突时抛异常或返回 empty/错误，需在文档中明确。

## include/orm/transaction.hpp
- 目的：封装数据库事务的 RAII 行为。
- 核心类型/API：
  - class Transaction { explicit Transaction(Session &s); ~Transaction(); void commit(); void rollback(); }。
- 职责与注意点：
  - 析构时如果未调用 commit，应自动 rollback。
  - commit/rollback 失败应抛出 `orm::DBException`。
  - 事务须在 Session 的上下文内执行，确保使用相同 Connection（保持事务上下文）。

## include/orm/connection_pool.hpp
- 目的：线程安全的 Connection 池，管理底层 mysqlcppconn 连接对象并复用它们。
- 核心类型/API：
  - class ConnectionPool { using Options = ...; static std::shared_ptr<ConnectionPool> createMysql(const std::string &conn_uri, Options opts = {}); std::unique_ptr<ConnectionGuard> get(); }。
  - class ConnectionGuard（RAII）：离开作用域时自动将连接归还池中；提供 `IConnection* operator->()` 或 `IConnection& get()` 访问底层连接。
  - IConnection 抽象接口（在 `mysql_driver.hpp` 定义）用于统一底层驱动的 execute/prepare/transaction 等操作。
- 职责与注意点：
  - 池应支持最大连接数、获取超时等配置选项。超时或空闲耗尽时返回明确错误（抛异常或返回空）。
  - 连接获取应尽量异常安全，Guard 析构时必须归还连接。

## include/orm/mysql_driver.hpp
- 目的：mysqlcppconn 驱动适配层，负责把 mysqlcppconn 的 API 包装成 `IConnection`、`IPreparedStatement`、`IResultRow` 等抽象接口。
- 核心类型/API：
  - class IConnection { virtual ~IConnection(); virtual std::unique_ptr<IPreparedStatement> prepare(const std::string &sql) = 0; virtual void execute(const std::string &sql) = 0; virtual void beginTransaction() = 0; virtual void commit() = 0; virtual void rollback() = 0; }。
  - class IPreparedStatement { virtual void bind(size_t index, const ValueVariant &v) = 0; virtual std::unique_ptr<IResultSet> executeQuery() = 0; virtual size_t executeUpdate() = 0; }。
  - factory: std::shared_ptr<IConnection> createMysqlConnection(const std::string &conn_uri)；用于 ConnectionPool 的实现。
- 职责与注意点：
  - 封装 mysqlcppconn 的连接/statement/resultset，暴露统一的异常类型映射（将底层异常转换为 `orm::DBException`）。
  - 实现应注意资源释放（PreparedStatement/ResultSet）以及线程安全（mysqlcppconn 的连接通常不可在多个线程间共享）。

## include/orm/exceptions.hpp
- 目的：定义库的异常层次与错误类型。
- 建议的异常类：
  - class ORMException : public std::runtime_error
  - class DBException : public ORMException
  - class MappingException : public ORMException
  - class TransactionException : public ORMException
- 职责与注意点：
  - 异常应携带足够的上下文（SQL、错误码、底层错误消息）以便调试。

## src/（实现文件建议）
- 目的：实现上面头文件声明的底层逻辑。
- 建议的源文件：
  - `src/connection_pool.cpp`：实现 ConnectionPool、ConnectionGuard。
  - `src/mysql_driver.cpp`：实现基于 mysqlcppconn 的 IConnection、IPreparedStatement、IResultSet 的包装。
  - `src/session.cpp`：实现 Session 的方法（find/save/remove/query 的基础流程）、与元信息的结合逻辑（序列化/反序列化）。
  - `src/query_builder.cpp`：实现 SQL 生成与参数收集逻辑。
  - `src/transaction.cpp`：实现 Transaction 的 RAII 行为。

实现注意点：
- 在实现初期可采用“最小可工作实现”，例如：
  - `find` 使用 `SELECT * FROM table WHERE id = ?` 并把 ResultSet 映射到实体字段；
  - `save` 简化为：若 `id` 为 0 或默认值则做 INSERT（并读取回填自增 id），否则做 UPDATE。
- 先实现值语义的实体返回（std::optional<T> / vector<T>），后续再考虑 Session 内部缓存或 Identity Map。

## examples/quick_start_mysql.cpp（示例说明）
- 目的：给出一个最小可运行示例，展示如何声明元信息、构造 ConnectionPool、打开 Session、执行 CRUD。
- 示例应包含：
  - minimal `User` struct 与 `metadata<User>()` 特化示例；
  - 使用 `ConnectionPool::createMysql(...)` 创建池；
  - 使用 `Session::open(pool)`、`session.save(user)`、`session.find<User>(id)`、`session.query<User>()...all()` 的示范代码（注释说明哪些是真实执行，哪些只是接口展示）。

## tests/
- 建议：使用 Catch2，编写单元测试覆盖：
  - metadata 特化与字段解析；
  - QueryBuilder SQL 生成（不必真的连接 DB，仅验证生成的 SQL 与绑定参数）；
  - Session 的高层逻辑（可用 mock IConnection）；
  - 简单集成测试在本地 MySQL 上运行（可选在 CI 中运行）。

## CMake（构建说明）
- `CMakeLists.txt`（根）应设置 C++17 标准并提供选项：`ORM_BUILD_EXAMPLES`, `ORM_BUILD_TESTS`。
- `src/CMakeLists.txt` 应定义 `orm_core` 静态/共享库目标并链接 mysqlcppconn（通过 `find_package(MySQL)` 或者自己查找 `mysqlcppconn`），并公开 include 路径。
- `examples/CMakeLists.txt`、`tests/CMakeLists.txt` 分别构建示例和测试。测试可选择性启用 Catch2（作为外部依赖或子模块）。