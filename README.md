<p align="right">
  <a href="README.md">中文</a> | <a href="README_EN.md">English</a>
</p>

# MatchingEngine — C++17 高性能撮合引擎原型

![License: MIT](https://img.shields.io/badge/license-MIT-green)

本项目实现了一个基于 **Reactor + Dispatcher + MatchingEngine + OrderBook** 的高性能撮合系统原型，包含完整的撮合链路与订单管理能力。系统特点包括：

- **TCP 网络入口（基于 epoll 的 Reactor）**：事件驱动、非阻塞 I/O
- **多线程消息解析（ThreadPool）**：解析 JSON、构造 DispatchMsg
- **Symbol 路由（EngineRouter）**：多品种映射，一个 MatchingEngine 可管理多个 symbol
- **订单薄（OrderBook）**：双向价格档结构，FIFO 队列，O(1) orderId 查询（哈希表），支持部分成交、完全成交、撤单
- **撮合引擎（MatchingEngine）**：单线程确定性撮合，生成 TradeEvent，确保执行顺序一致性
- **完整 E2E 链路**：客户端下单 → TcpServer → Dispatcher → MatchingEngine 撮合 → 成交回报 → 客户端

适合作为：

- **量化开发 / 交易系统岗面试**
- **高性能 C++ 工程展示**
- **撮合引擎数据结构与系统架构学习项目**


## 📁 项目结构

```
OrderBook/
├── include/               # 公共头文件
│   ├── core/              # 订单薄、价格档、对象池
│   ├── dispatch/          # Dispatcher
│   ├── engine/            # Engine, Router
│   ├── net/               # EpollReactor, TcpServer, TcpConnection
│   └── utils/             # Logger, ThreadPool, LockFreeQueue, Parser
├── src/                   # 核心实现
├── tests/                 # 单元测试
├── third_party/           # 外部库（例如：googletest）
└── CMakeLists.txt         # 构建脚本
```


## 🧩 系统架构
![Matching Engine Architecture](docs/matching_engine_architecture.svg)


## 🔄 消息流
![Matching Engine Msg Flow](docs/sequence.svg)


## 🧵 线程模型
![Matching Engine Thread Model](docs/thread_model.svg)

## 🔁 撮合流程
![Matching Engine Matching Flow](docs/matching_flow.svg)


## 🚀 核心特性

### 网络层（Reactor 模型）
- 基于 **epoll** 的高性能事件驱动网络模型  
- TcpServer 支持非阻塞 I/O  
- 内置线程池负责 JSON 消息解析  

### 订单分发层（Dispatcher + Router）
- Dispatcher 负责 inbound → engine 路由 & outbound → 客户端发送  
- EngineRouter 支持 **多 symbol 映射**，一个 MatchingEngine 可管理多个品种  
- 捕获撮合线程的出队事件，实现异步结果回写  

### 撮合引擎（MatchingEngine）
- 单线程 **确定性撮合**（避免锁与竞态）  
- 支持：
  - NEW_ORDER（下单）
  - CANCEL_ORDER（撤单）
  - TRADE_REPORT（成交回报）  
- 撮合过程中生成 TradeEvent（成交事件），自动推送给 Dispatcher  

### 订单簿（OrderBook：项目核心）
- 使用 **双向价格档结构**（bids/asks）  
- 买一/卖一自动维护（bestBid / bestAsk）  
- 每个价格档使用 **FIFO 队列**（保证交易公平）  
- 内置 **OrderPool（对象池）** 提升性能、减少频繁 new/delete  
- **orderIndex（哈希表）支持 O(1) orderId 查询**  
- 完整实现撮合流程：
  - 主动单匹配挂单（maker-taker）  
  - 生成 TradeEvent  
  - 部分成交 / 全量成交  
  - 撤单、剩余数量处理、价格档清理  


## ⚙️ 构建与运行

### 构建

```bash
mkdir build && cd build
cmake ..
make -j
```

### 启动撮合服务器

```bash
./src/matchengine_main
```

### 用 nc 测试

```bash
nc 127.0.0.1 9000
```

### 📥 示例订单
在 nc 中输入如下 JSON：

下单
```json
{"type":"NEW_ORDER","symbol":"MAOTAI","side":"BUY","price":100,"qty":10}
```

卖单成交
```json
{"type":"NEW_ORDER","symbol":"MAOTAI","side":"SELL","price":99.0,"qty":5}
```

撤单
```json
{"type":"CANCEL_ORDER","symbol":"MAOTAI","orderId":1}
```

## 📈 性能（未来补充）


* 单引擎 → 入站 TPS（吞吐量）

* 纯撮合延迟（match-only latency）

* 端到端往返延迟（RTT，使用 nc 测试）

* 高负载下线程池饱和情况（ThreadPool saturation）


##  🧭 未来规划
- 多引擎分片撮合
- 行情广播模块
- 数据持久化（Snapshot + WAL）
- SO_REUSEPORT 多 Reactor 扩展
- WebSocket 网关
- 单元测试全面覆盖
- 性能基准测试模块


## 📄 许可证
本项目采用 MIT License 开源协议。  
完整内容请参见 [`LICENSE`](./LICENSE) 文件。
