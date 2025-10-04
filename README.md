# C++ ORM — 说明

本仓库目标是实现一个轻量、类型安全且可扩展的 C++ ORM（首版只支持 MySQL，基于 mysqlcppconn）。


## 注意
- 数据库路径采用硬编码，在mysql_driver部分，直接使用了tcp://127.0.0.1:3306,user:root,pass:qwer0122，请根据实际情况修改。


## 已确认的设计决策（回顾）
- 首版只实现 MySQL 驱动，依赖 mysqlcppconn (MySQL Connector/C++)。
- 数据映射采用元编程（tuple-of-members / 函数模板）策略，尽量避免宏。
- 暂不实现迁移系统（migration），后续独立添加。
- C++17 作为最低编译标准。
- 给出核心类型与API的设计


