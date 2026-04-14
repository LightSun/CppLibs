#pragma once
#include <atomic>

/*
* 单生产多消费
* 需要预先创建消费者 再创建生产者
* 并发粒度调整到每个元素层次
* 存在背压，受消费最慢的消费者影响
Example:
typedef PQ::SpmcQ<int, 1024> SpmcQ_;
SpmcQ_ TestQ;
void testW()
{
	PQ::CProducer<SpmcQ_> producer(TestQ);
	for (int i = 0; i < 1000000; ++i)
	{
		uint64_t pos_ = producer.producePos();
		while (!producer.produce(pos_, int(i)))
			std::this_thread::yield();
	}
}
void testR()
{
	PQ::CConsumer<SpmcQ_> consumer(TestQ);
	int val = -1;
	for (int i = 0; i < 1000000; ++i)
	{
		uint64_t pos_ = consumer.consumePos();
		while (!consumer.consume(pos_, val))
			std::this_thread::yield();
	}
}
std::thread threadR1(testR);
std::thread threadR2(testR);
sleep(1);
std::thread threadW(testW);
*/
namespace PQ
{
	template <typename T>
	struct Slot
	{
		T val_;
		alignas(64) std::atomic<uint32_t> consume_ = { 0 };
		alignas(64) std::atomic<uint32_t> consume_r = { 0 };
		alignas(64) std::atomic <uint64_t> pos_ = { 0 };
	public:
		bool update(T&& val, uint64_t pos, uint32_t ConsumeNum)
		{
			uint32_t consume_n = 0;
			if (!consume_.compare_exchange_weak(consume_n, ConsumeNum, std::memory_order_acq_rel))
			{
				return false;
			}
			val_ = val;
			pos_.store(pos, std::memory_order_relaxed);
			consume_r.store(ConsumeNum, std::memory_order_release);
			return true;
		}

		bool get(T& val, uint64_t pos, uint32_t ConsumeNum)
		{
			if (pos > pos_.load(std::memory_order_relaxed))
			{
				return false;
			}
			if (consume_r.load(std::memory_order_relaxed) > 0)
			{
				val = val_;
				uint32_t v = consume_r.fetch_sub(1, std::memory_order_release);
				if (v == 1)
				{
					consume_.store(0, std::memory_order_release);
				}
				return true;
			}
			return false;
		}
	};

	template <typename T, int Size = 1024, int Mod = Size - 1>
	class SpmcQ
	{
		alignas(64) std::atomic<uint32_t> consume_num;
		//! 缓存区
		Slot<T> slots_[Size];
	public:
		SpmcQ() : consume_num(0)
		{
		}
		void readerInc()
		{
			consume_num.fetch_add(1, std::memory_order_release);
		}
	public:
		typedef T ValType;
		bool write(int64_t pos_, T&& val)
		{
			Slot<T>& slot_ = slots_[pos_ & Mod];
			bool ret = slot_.update(std::forward<T>(val), pos_, consume_num.load(std::memory_order_relaxed));
			return ret;
		}

		bool read(uint64_t pos_, T& val)
		{
			Slot<T>& slot_ = slots_[pos_ & Mod];
			return slot_.get(val, pos_, consume_num.load(std::memory_order_relaxed));
		}
	};

	template <typename Q>
	class CProducer
	{
		Q& q_;
        alignas(64) std::atomic<uint64_t> pos_w = { 0 };
	public:
		CProducer(Q& q) : q_(q)
		{
		}
	public:
		uint64_t producePos()
		{
            return pos_w.load(std::memory_order_relaxed);
		}
		bool produce(uint64_t pos_, typename Q::ValType&& val)
		{
			if (!q_.write(pos_, std::forward<typename Q::ValType>(val)))
				return false;
			pos_w.fetch_add(1, std::memory_order_release);
			return true;
		}
	};

	template <typename Q>
	class CConsumer
	{
		Q& q_;
		uint64_t pos_r = 0;
	public:
		CConsumer(Q& q) : q_(q)
		{
			q_.readerInc();
		}

	public:
		uint64_t consumePos()
		{
			return pos_r;
		}
		bool consume(uint64_t pos_, typename Q::ValType& val)
		{
			if (!q_.read(pos_, val))
			{
				return false;
			}
			pos_r += 1;
			return true;
		}
	};
}