#pragma once
#include <atomic>

/*
* 单生产多消费
* 需要应用预先知道消费线程的数量
* 并发粒度调整到每个元素层次
* 存在背压，受消费最慢的消费者影响
Example:
constexpr int ConsumeNum = 2;
typedef PQ::SpmcQ<int, 1024> SpmcQ_;
SpmcQ_ TestQ(ConsumeNum);
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
std::thread threadW(testW);
*/
namespace PQ
{
	template <typename T>
	struct Slot
	{
		T val_;
		alignas(64) std::atomic<uint32_t> consume_ = { 0 };
		alignas(64) std::atomic <uint64_t> pos_ = { 0 };
	public:
		bool update(T&& val, uint64_t pos, uint32_t ConsumeNum)
		{
			if (consume_.load(std::memory_order_relaxed) == ConsumeNum)
			{
				val_ = val;
				pos_.store(pos, std::memory_order_relaxed);
				consume_.store(0, std::memory_order_release);
				return true;
			}
			return false;
		}

		bool get(T& val, uint64_t pos, uint32_t ConsumeNum)
		{
			if (pos > pos_.load(std::memory_order_relaxed))
			{
				return false;
			}

			if (consume_.load(std::memory_order_relaxed) < ConsumeNum)
			{
				val = val_;
				consume_.fetch_add(1, std::memory_order_release);
				return true;
			}
			return false;
		}
	};

	//位置协调
	struct SeqHandler
	{
		//! 写入的位置
		alignas(64) std::atomic<uint64_t> next_pos = { 0 };
	public:
		uint64_t getWtPos()
		{
			return next_pos.load(std::memory_order_relaxed);
		}
		void commitNextWtPos()
		{
			next_pos.fetch_add(1, std::memory_order_release);
		}
	};

	template <typename T, int Size = 1024, int Mod = Size - 1>
	class SpmcQ
	{
		uint32_t consume_num;
		//! pos
		SeqHandler seq_;
		//! 缓存区
		Slot<T> slots_[Size];
	public:
		SpmcQ(uint32_t num) : consume_num(num)
		{
			for (int i = 0; i < Size; ++i)
			{
				slots_[i].consume_.store(consume_num);
			}
		}
	public:
		typedef T ValType;
		SeqHandler& seq()
		{
			return seq_;
		}
		bool write(int64_t pos_, T&& val)
		{
			Slot<T>& slot_ = slots_[pos_ & Mod];
			bool ret = slot_.update(std::forward<T>(val), pos_, consume_num);
			return ret;
		}

		bool read(uint64_t pos_, T& val)
		{
			Slot<T>& slot_ = slots_[pos_ & Mod];
			return slot_.get(val, pos_, consume_num);
		}
	};

	template <typename Q>
	class CProducer
	{
		Q& q_;
	public:
		CProducer(Q& q) : q_(q)
		{
		}
	public:
		uint64_t producePos()
		{
			return q_.seq().getWtPos();
		}
		bool produce(uint64_t pos_, typename Q::ValType&& val)
		{
			if (!q_.write(pos_, std::forward<typename Q::ValType>(val)))
				return false;
			//std::atomic_thread_fence(std::memory_order_acquire);
			q_.seq().commitNextWtPos();
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