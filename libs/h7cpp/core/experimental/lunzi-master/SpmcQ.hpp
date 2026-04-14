#pragma once
#ifdef min
#undef min
#endif
#include <atomic>
#include <algorithm>

/*
* 单生产多消费
* 存在背压，即生产者会受消费最慢的消费者影响
*
Example:
//typedef PQ::SpmcQ<int> SpmcQ_;
typedef PQ::SpmcQ<int, PQ::Rd_Repeat, 1024> SpmcQ_;
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

void testR1()
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
*/

namespace PQ
{
	template <typename T>
	struct Slot
	{
		T val_;
	public:
		void update(T&& val)
		{
			val_ = val;
		}

		bool get(T& val)
		{
			val = val_;
			return true;
		}
	};

	enum
	{
		Rd_Once,         //唯一消费
		Rd_Repeat       //广播式消费
	};

	//位置协调
	template <int RdType>
	struct SeqHandler
	{
		//! 写入的位置
		alignas(64) volatile uint64_t cur_pos_w = 0;
		alignas(64) std::atomic<uint64_t> next_pos = { 0 };
		//
		template <int RdType>
		struct RdPos 
		{
			int index() { return 0; }
			uint64_t curMinPosR() { return 0; }
			uint64_t getRdPos(int idx) { return 0; }
			void commitNextRdPos(int idx) {}
		};

		struct RdPosBase
		{
			alignas(64) std::atomic<int> rd_size = { 0 };
			alignas(64) std::atomic<uint64_t> rd_pos_arr[128] = { 0 };    //最大128个消费
			//
			int index()
			{
				int index_ = rd_size.fetch_add(1, std::memory_order_release);
				return index_;
			}
			uint64_t curMinPosR()
			{
				uint64_t pos_ = rd_pos_arr[0].load(std::memory_order_relaxed);
				int cnt = rd_size.load(std::memory_order_relaxed);
				for (int i = 1; i < cnt; ++i)
				{
					pos_ = std::min(pos_, rd_pos_arr[i].load(std::memory_order_relaxed));
				}
				return pos_;
			}
            void initRdPos(int idx, uint64_t pos_)
			{
				rd_pos_arr[idx].store(pos_, std::memory_order_release);
			}
		};
		//! 读取的位置
		template<> struct RdPos<Rd_Once> : public RdPosBase
		{
			alignas(64) std::atomic<uint64_t> rd_pos = { 0 };
			uint64_t getRdPos(int idx)
			{
				uint64_t last_pos = rd_pos.fetch_add(1, std::memory_order_release);
				RdPosBase::rd_pos_arr[idx].store(last_pos, std::memory_order_release);
				return last_pos;
			}
			void commitNextRdPos(int idx) {}
		};

		template<> struct RdPos<Rd_Repeat> : public RdPosBase
		{
			uint64_t getRdPos(int idx)
			{
				uint64_t last_pos = RdPosBase::rd_pos_arr[idx].load(std::memory_order_relaxed);
				return last_pos;
			}
			void commitNextRdPos(int idx)
			{
				RdPosBase::rd_pos_arr[idx].fetch_add(1, std::memory_order_release);
			}
		};
		//! 读取的位置
		RdPos<RdType> rd_pos_;
	public:
		uint64_t curPosW()
		{
			return cur_pos_w;
		}
		uint64_t getWtPos()
		{
			return next_pos.load(std::memory_order_relaxed);
		}
		uint64_t commitNextWtPos()
		{
			next_pos.fetch_add(1, std::memory_order_release);
			cur_pos_w = next_pos.load(std::memory_order_relaxed);
			return cur_pos_w;
		}
		int index()
		{
			return rd_pos_.index();
		}
		uint64_t curMinPosR()
		{
			return rd_pos_.curMinPosR();
		}
		uint64_t getRdPos(int idx)
		{
			return rd_pos_.getRdPos(idx);
		}
		void commitNextRdPos(int idx)
		{
			rd_pos_.commitNextRdPos(idx);
		}
        void initRdPos(int idx)
		{
			rd_pos_.initRdPos(idx, curPosW());
		}
	};

	template <typename T, int RdType = Rd_Once, int Size = 1024, int Mod = Size - 1>
	class SpmcQ
	{
		//! pos
		SeqHandler<RdType> seq_;
		//! 缓存区
		alignas(64) Slot<T> slots_[Size];
	public:
		typedef T ValType;
		SeqHandler<RdType>& seq()
		{
			return seq_;
		}
		bool write(int64_t pos_, T&& val)
		{
			int64_t pos_r_ = seq_.curMinPosR();
			if (pos_ - pos_r_ >= Size)
				return false;
			Slot<T>& slot_ = slots_[pos_ & Mod];
			slot_.update(std::forward<T>(val));
			return true;
		}

		bool read(uint64_t pos_, T& val)
		{
			uint64_t pos_w_ = seq_.curPosW();
			if (pos_ >= pos_w_)
			{
				return false;
			}
			Slot<T>& slot_ = slots_[pos_ & Mod];
			slot_.get(val);
			return true;
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
			std::atomic_thread_fence(std::memory_order_acquire);
			q_.seq().commitNextWtPos();
			return true;
		}
	};

	template <typename Q>
	class CConsumer
	{
		int index_;
		Q& q_;
	public:
		CConsumer(Q& q) : q_(q)
		{
			index_ = q_.seq().index();
            q_.seq().initRdPos(index_);      //当消费加入时 获取最近消费起点  若在应用中消费者在生产者初始化之前进行初始化 则从最原初的0点开始消费
		}

	public:
		uint64_t consumePos()
		{
			return q_.seq().getRdPos(index_);
		}
		bool consume(uint64_t pos_, typename Q::ValType& val)
		{
			if (!q_.read(pos_, val))
				return false;
			q_.seq().commitNextRdPos(index_);
			return true;
		}
	};

}
