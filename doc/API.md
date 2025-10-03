# ORM API 参考（接口契约）

下面是每个模块的接口契约、类型与实现注意点，供实现时参考。

## model_meta.hpp
- FieldOpts
  - 字段属性：primary、nullable、column_name
- MemberPtr<T, M> = M T::*
- FieldInfo<T, M>
  - name
  - MemberPtr<T, M> member
  - FieldOpts opts
- metadata<T>() must be specialized per model and return tuple of FieldInfo entries

实现注意：提供 helper 模板用于从元组中枚举字段、读写成员。

## session.hpp
- Session(std::shared_ptr<ConnectionPool> pool)
- template<typename T> std::optional<T> find(const PrimaryKeyType<T>& id)
- template<typename T> void save(T& entity)
- template<typename T> void remove(const PrimaryKeyType<T>& id)
- template<typename T> QueryBuilder<T> query()
- std::unique_ptr<Transaction> beginTransaction()

实现注意：Session 管理连接获取并负责把查询结果映射回实体；抛出 orm::DBException 来报告错误。

## query_builder.hpp
- template<typename T>
  - where(MemberPtr<T,M> member, Op op, const Value& v)
  - orderBy(MemberPtr<T,M> member, Order)
  - limit/offset/all/one/count

实现注意：构建参数化 SQL 与绑定列表，不应直接拼接用户数据。

## transaction.hpp
- Transaction(Session& s)
- ~Transaction() // 自动 rollback 如果没有 commit
- commit()/rollback()

实现注意：Transaction 使用 Session 提供的 Connection，并在析构时自动 rollback。

## connection_pool.hpp
- createMysql(conn_uri, Options)
- Guard RAII 类型，提供 get()/operator->()
- get(timeout) 获取连接，超时抛出异常

实现注意：线程安全、超时明确、保证归还连接。

## mysql_driver.hpp
- IConnection / IPreparedStatement / IResultSet 接口
- createMysqlConnection(conn_uri) 工厂函数

实现注意：把 mysqlcppconn 的异常转换成 orm::DBException；注意资源释放。

## exceptions.hpp
- ORMException, DBException, MappingException, TransactionException

实现注意：异常应带有 SQL 和底层错误信息以方便调试。
