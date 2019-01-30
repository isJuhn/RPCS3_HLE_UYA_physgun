#pragma once
#include "../RPCS3/PPUThread.h"
#include "../RPCS3/Utilities/geometry.h"
#include <vector>

#define RETURN_TO(x) ppu.lr = x
#define RETURN_OFFSET(x) ppu.lr = ppu.cia + x

namespace vm
{
	using addr_t = u32;

	template<typename T>
	class ptrb
	{
	public:
		ptrb(addr_t addr) : m_addr{ addr }
		{
		}

		template<typename AT>
		ptrb(ptrb<AT>& right) : m_addr{ right.m_addr }
		{
		}

		void set(addr_t addr)
		{
			m_addr = addr;
		}

		u32 addr()
		{
			return m_addr;
		}

		operator bool() const
		{
			return m_addr;
		}

		operator u64() const
		{
			return m_addr;
		}

		T& operator*()
		{
			return *reinterpret_cast<T*>(m_addr + api::g_base_addr);
		}

		T* operator->()
		{
			return reinterpret_cast<T*>(m_addr + api::g_base_addr);
		}

	private:
		addr_t m_addr;
	};

	template<>
	class ptrb<void>
	{
	public:
		ptrb() : m_addr{}
		{
		}

		ptrb(addr_t addr) : m_addr{ addr }
		{
		}

		u32 addr()
		{
			return m_addr;
		}

		void set(addr_t addr)
		{
			m_addr = addr;
		}

		operator bool() const
		{
			return m_addr;
		}

		operator u64() const
		{
			return m_addr;
		}

	private:
		u32 m_addr;
	};

	template<u32 g_count, u32 f_count, typename T, typename... Targs>
	struct call_helper
	{
		static void call(ppu_thread& ppu, u32 addr, T t, Targs... args)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				ppu.fpr[f_count] = static_cast<f64>(t);
				call_helper<g_count, f_count + 1, Targs...>::call(ppu, addr, args...);
			}
			else
			{
				ppu.gpr[g_count + 3] = static_cast<u64>(t);
				call_helper<g_count + 1, f_count, Targs...>::call(ppu, addr, args...);
			}
		}
	};

	template<u32 g_count, u32 f_count, typename T>
	struct call_helper<g_count, f_count, T>
	{
		static void call(ppu_thread& ppu, u32 addr, T t)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				ppu.fpr[f_count] = static_cast<f64>(t);
			}
			else
			{
				ppu.gpr[g_count + 3] = static_cast<u64>(t);
			}
			api::do_call(ppu, addr);
		}
	};

	template<typename RT, typename... Args>
	class ptrb<RT(Args...)>
	{
	public:
		ptrb(addr_t addr) : m_addr{ addr }
		{
		}

		u32 addr()
		{
			return m_addr;
		}
		
		RT operator()(ppu_thread& ppu, Args... args)
		{
			call_helper<0, 0, Args...>::call(ppu, m_addr, args...);
			if constexpr (std::is_floating_point<RT>::value)
			{
				return static_cast<RT>(ppu.fpr[0]);
			}
			return static_cast<RT>(ppu.gpr[3]);
		}

		operator bool() const
		{
			return m_addr;
		}

		operator u64() const
		{
			return m_addr;
		}

	private:
		addr_t m_addr;
	};

	template<typename... Args>
	class ptrb<void(Args...)>
	{
	public:
		ptrb(addr_t addr) : m_addr{ addr }
		{
		}

		u32 addr()
		{
			return m_addr;
		}

		void operator()(ppu_thread& ppu, Args... args)
		{
			call_helper<0, 0, Args...>::call(ppu, m_addr, args...);
		}

		operator bool() const
		{
			return m_addr;
		}

		operator u64() const
		{
			return m_addr;
		}

	private:
		addr_t m_addr;
	};

	template<typename T>
	using ptr = ptrb<to_be_t<T>>;
}

namespace api
{
	u8* g_base_addr;
	std::string hash;
	std::function<void(u32, be_t<u32>)> write32;
	std::function<const be_t<u32>&(u32)> read32;
	std::function<u32(ppu_thread&, u32, u32)> stack_alloc;
	std::function<void(ppu_thread&, u32, u32)> stack_dealloc;
	std::function<void(ppu_thread&, u32)> do_call;
	std::function<void(const std::string& text, int x, int y, int font_size, color4f color, const std::string& font)> draw_text;
	std::function<void(int x, int y, int w, int h, color4f col)> draw_square;

	struct func_end
	{
		static const bool end = true;
	};

	template<unsigned int i, auto T, auto... Args>
	struct func
	{
		using type = decltype(T);
		using Tail = func<i + 1, Args...>;
		static const bool end = false;
		static const unsigned int index = i;
	};

	template<unsigned int i, auto T>
	struct func<i, T>
	{
		using type = decltype(T);
		using Tail = func_end;
		static const bool end = false;
		static const unsigned int index = i;
	};

	std::vector<void*> api_table{ &g_base_addr, &hash, &write32, &read32, &stack_alloc, &stack_dealloc, &do_call, &draw_text, &draw_square };
	using api_table_t = func <0, &g_base_addr, &hash, &write32, &read32, &stack_alloc, &stack_dealloc, &do_call, &draw_text, &draw_square>;

	template<typename table_t, bool end = table_t::end>
	struct reg_api
	{
		static void register_table(const std::vector<void*>& table)
		{
			using type = typename table_t::type;
			constexpr int index = table_t::index;
			*static_cast<type>(api_table[index]) = *static_cast<type>(table[index]);
			reg_api<typename table_t::Tail>::register_table(table);
		}
	};

	template<typename table_t>
	struct reg_api<table_t, true>
	{
		static void register_table(const std::vector<void*>& table)
		{
			return;
		}
	};
}
