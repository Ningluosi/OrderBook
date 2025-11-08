#include "engine/matching_engine.h"
#include "dispatch/dispatcher.h"
#include "utils/logger.h"

using namespace engine;
using namespace core;
using namespace utils;
using namespace dispatch;

int main() {
    // ========= 1️⃣ 创建 dispatcher =========
    Dispatcher dispatcher(/*线程数*/2, /*队列容量*/4096);

    // 绑定一个简单的 sender（测试输出）
    dispatcher.setSender([](int fd, const std::string& data) {
        std::cout << "[SEND] to fd=" << fd << " payload=" << data << std::endl;
        return true;
    });

    dispatcher.start();

    // ========= 2️⃣ 创建匹配引擎（绑定 symbol）=========
    MatchingEngine engine(&dispatcher, "BTCUSDT", 100000);

    // ========= 3️⃣ 模拟下单 =========
    DispatchMsg msg1;
    msg1.type = MsgType::NEW_ORDER;
    msg1.symbol = "BTCUSDT";
    msg1.side = Side::BUY;
    msg1.price = 100.0;
    msg1.qty = 10;
    msg1.fd = 1;  // 模拟客户端 fd
    engine.handleOrderMessage(std::move(msg1));

    // ========= 4️⃣ 模拟对手方挂单 =========
    DispatchMsg msg2;
    msg2.type = MsgType::NEW_ORDER;
    msg2.symbol = "BTCUSDT";
    msg2.side = Side::SELL;
    msg2.price = 99.0;
    msg2.qty = 5;
    msg2.fd = 2;
    engine.handleOrderMessage(std::move(msg2));

    // ========= 5️⃣ 模拟撤单 =========
    DispatchMsg cancel;
    cancel.type = MsgType::CANCEL_ORDER;
    cancel.symbol = "BTCUSDT";
    cancel.orderId = 1;  // 撤销买单 #1
    cancel.fd = 1;
    engine.handleOrderMessage(std::move(cancel));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    dispatcher.stop();
    return 0;
}
