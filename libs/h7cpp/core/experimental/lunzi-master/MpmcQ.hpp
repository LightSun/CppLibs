#pragma once
#include <atomic>
#include <vector>

/*
* 单生产多消费
* 需要预创建生产者 和 消费者
* 并发粒度调整到每个元素层次
* 生产者数量为n 队列长度限制L = n * m
* 存在背压，受消费最慢的消费者影响
Example:
typedef PQ::MpmcQ<int, 1024> MpmcQ_;
MpmcQ_ TestQ;
PQ::CProducer<MpmcQ_> producer1(TestQ);
PQ::CProducer<MpmcQ_> producer2(TestQ);
void testW(PQ::CProducer<MpmcQ_>* producer)
{
	for (int i = 0; i < 1000000; ++i)
	{
		uint64_t pos_ = producer.producePos();
		while (!producer.produce(pos_, int(i)))
			std::this_thread::yield();
	}
}

PQ::CConsumer<MpmcQ_> consumer1(TestQ);
PQ::CConsumer<MpmcQ_> consumer2(TestQ);
void testR(PQ::CConsumer<MpmcQ_>* consumer)
{
	PQ::CConsumer<MpmcQ_> consumer(TestQ);
	int val = -1;
    int cnt = consumer->fanOutCnt();
    while (1)
    {
        for (int i = 0; i < cnt; ++i)
        {
            uint64_t pos_ = consumer.consumePos(i);
            while (!consumer.consume(i, pos_, val))
                std::this_thread::yield();
        }
    }
}
std::thread threadR1(testR, &consumer1);
std::thread threadR2(testR, &consumer2);
std::thread threadW1(testW, &producer1);
std::thread threadW1(testW, &producer2);
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
			if (consume_r.load(std::memory_order_acquire) > 0)
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

	//位置协调
	struct SeqHandler
	{
		//! 写入的位置
		alignas(64) std::atomic<uint64_t> next_pos = { 0 };
	public:
		uint64_t getWtPos()
		{
			return next_pos.fetch_add(1, std::memory_order_release);
		}
		uint64_t getWtCnt()
		{
			return next_pos.load(std::memory_order_relaxed);
		}
	};

	template <typename T, int Size = 1024>
	class MpmcQ
	{
		alignas(64) std::atomic<uint32_t> consume_num;
		//! pos
		SeqHandler seq_;
		//! 缓存区
		Slot<T> slots_[Size];
	public:
		MpmcQ() : consume_num(0)
		{
		}
		void readerInc()
		{
			consume_num.fetch_add(1, std::memory_order_release);
		}
	public:
		typedef T ValType;
		SeqHandler& seq()
		{
			return seq_;
		}
		bool write(int64_t pos_, T&& val)
		{
			Slot<T>& slot_ = slots_[pos_ % Size];
			bool ret = slot_.update(std::forward<T>(val), pos_, consume_num.load(std::memory_order_relaxed));
			return ret;
		}

		bool read(uint64_t pos_, T& val)
		{
			Slot<T>& slot_ = slots_[pos_ % Size];
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
			pos_w = q_.seq().getWtPos();
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
			pos_w.fetch_add(q_.seq().getWtCnt(), std::memory_order_release);
			return true;
		}
	};

	template <typename Q>
	class CConsumer
	{
		Q& q_;
        std::vector<uint64_t> pos_r_s;
	public:
		CConsumer(Q& q) : q_(q)
		{
			q_.readerInc();
            for (int i = 0; i < q_.seq().getWtCnt(); ++i)
			{
				pos_r_s.push_back(i);
			}
		}

	public:
    	int fanOutCnt()
		{
			return q_.seq().getWtCnt();
		}
		uint64_t consumePos(int idx)
		{
            return pos_r_s[idx];
		}
		bool consume(uint64_t pos_, typename Q::ValType& val)
		{
			if (!q_.read(pos_, val))
			{
				return false;
			}
            pos_r_s[pos_] += fanOutCnt();
			return true;
		}
	};
}

