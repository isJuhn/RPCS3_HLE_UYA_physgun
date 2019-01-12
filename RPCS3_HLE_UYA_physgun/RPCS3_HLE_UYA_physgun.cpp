#include "stdafx.h"
#include <functional>
#include "RPCS3/PPUThread.h"
#include "RPCS3_HLE/HLE_utils.h"
#include "vec4.h"

std::function<void(u32)> printint;

using spawn_ray_func = void(vm::ptr<Vec4> arg1, vm::ptr<Vec4> arg2, vm::ptr<Vec4> arg3, u32 arg4, f64 argf1);
using CollLine = b8(vm::ptr<Vec4> vec_from, vm::ptr<Vec4> vec_to, u32, vm::ptr<void> moby, u32);

struct physgun_mod_data_t
{
	u8 physics;
	vm::ptr<void> selected_moby;
	Vec4 ray_norm_vector;
	f32 ray_length;
	Vec4 ratchet_pos;
	Vec4 moby_offset;
} physgun_data{};

extern "C" void __declspec(dllexport) __cdecl physgun_tick(ppu_thread& ppu)
{
	vm::ptr<spawn_ray_func> spawn_ray{ vm::addr_t{ 0xBE6FF0 } };
	vm::ptr<CollLine> coll_line{ vm::addr_t{ 0xBD5790 } };
	vm::ptr<Vec4> vec_0{ api::stack_alloc(ppu, sizeof(Vec4), alignof(Vec4)) };

	vm::ptr<Vec4> start_pos{static_cast<u32>(ppu.gpr[28])};

	if (physgun_data.physics > 0)
	{
		--physgun_data.physics;
	}

	if (!api::read32(0xEE9334))
	{
		if (api::read32(api::read32(0xC1D574) + 0xA0) & 0x28 && !physgun_data.physics)
		{
			if (!physgun_data.selected_moby)
			{
				vm::ptr<Vec4> end_pos{ vm::addr_t{0xDA3E40} };
				*end_pos += end_pos->sub_vec4(*start_pos);
				vm::ptr<void> ratchet_moby{ vm::addr_t{ api::read32(0xDA4DB0) } };
				coll_line(ppu, start_pos, end_pos, 0x20, ratchet_moby, 0);
				end_pos.set(0xEE97A0);
				vm::ptr<void> sel_moby{ vm::addr_t{ api::read32(0xEE9798) } };
				if (sel_moby)
				{
					physgun_data.physics = 5;
					physgun_data.selected_moby = sel_moby;
					physgun_data.moby_offset = vm::ptr<Vec4>{ sel_moby.addr() + 0x10 }->sub_vec4(*end_pos);
					Vec4 ray_vector = end_pos->sub_vec4(*vm::ptr<Vec4>{ratchet_moby.addr() + 0x10});
					physgun_data.ray_norm_vector = ray_vector.div_i(ray_vector.len());
					physgun_data.ray_length = ray_vector.len();
					physgun_data.ratchet_pos = *vm::ptr<Vec4>{ratchet_moby.addr() + 0x10};
				}
			}
			else
			{
				physgun_data.physics = 5;
				physgun_data.selected_moby.set(0x0);
			}
		}
	}
	
	if (physgun_data.selected_moby)
	{
		vm::ptr<void> ratchet_moby{ api::read32(0xDA4DB0) };
		physgun_data.ratchet_pos = *vm::ptr<Vec4>{ratchet_moby.addr() + 0x10};
		physgun_data.ray_norm_vector = vm::ptr<Vec4>{ 0xDA3E40 }->sub_vec4(physgun_data.ratchet_pos).norm();
		*vm::ptr<Vec4>{physgun_data.selected_moby.addr() + 0x10} = physgun_data.ratchet_pos.add_vec4(physgun_data.ray_norm_vector.mul_i(physgun_data.ray_length)).add_vec4(physgun_data.moby_offset);
		vm::ptr<Vec4> end_pos{ api::stack_alloc(ppu, sizeof(Vec4), alignof(Vec4)) };
		*end_pos = physgun_data.ratchet_pos.add_vec4(physgun_data.ray_norm_vector.mul_i(physgun_data.ray_length));
		spawn_ray(ppu, start_pos, vec_0, end_pos, 1, 0.0f);
		api::stack_dealloc(ppu, end_pos.addr(), sizeof(Vec4));
	}
	api::stack_dealloc(ppu, vec_0.addr(), sizeof(Vec4));
	RETURN_TO(0x5A1734);
}

extern "C" void __declspec(dllexport) __cdecl init_dll(const std::vector<void*>& table)
{
	api::reg_api<api::api_table_t>::register_table(table);
}
