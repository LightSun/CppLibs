#pragma once

#include <atomic>

/*
操作	原子操作看到的状态	接下来做什么	返回值
increment	fetch_add 返回的旧值没有 IS_ZERO	加一已经完成	true
increment	旧值带 IS_ZERO	低位虽然被加一，但计数仍解释为零	false
decrement	fetch_sub 返回值不是 1	这次没有负责归零	false
decrement	返回值是 1，随后 CAS 成功写入 IS_ZERO	当前线程提交归零	true
decrement	CAS 失败，expected 不带 HELPED	没有人留下“帮助完成”的记号	false
decrement	expected 带 HELPED，exchange 的旧值也带 HELPED	当前线程抢到唯一 credit	true
decrement	本地 expected 带 HELPED，但 exchange 的旧值已经没有它	另一个 decrement 先抢到 credit	false
read	load 到普通正数	直接返回该值	正数
read	load 到带 IS_ZERO 的状态	忽略低位	0
read	load 到普通 0，CAS 成功	写入 `IS_ZERO	HELPED`，替 decrement 完成归零
read	load 到普通 0，但 CAS 失败	CAS 把新状态写回 value，再按正数或 zero flag 解释	新逻辑值
*/
//一半 load、一半 store：优势已经很小
//10% load、90% store：Lock-Free 开始反超
struct Counter {
    static constexpr std::uint64_t IS_ZERO = 1ULL << 63;
    static constexpr std::uint64_t HELPED = 1ULL << 62;

    bool incrementIfNotZero() {
        return (counter_.fetch_add(1) & IS_ZERO) == 0;
    }

    bool decrement() {
        if (counter_.fetch_sub(1) != 1) {
            return false;
        }

        std::uint64_t expected = 0;
        if (counter_.compare_exchange_strong(expected, IS_ZERO)) {
            return true;
        }

        if ((expected & HELPED) != 0 && (counter_.exchange(IS_ZERO) & HELPED) != 0) {
            return true;
        }

        return false;
    }

    std::uint64_t read() {
        auto value = counter_.load();
        if (value == 0 && counter_.compare_exchange_strong(value, IS_ZERO | HELPED)) {
            return 0;
        }
        return (value & IS_ZERO) != 0 ? 0 : value;
    }

    std::atomic<std::uint64_t> counter_{1};
};
