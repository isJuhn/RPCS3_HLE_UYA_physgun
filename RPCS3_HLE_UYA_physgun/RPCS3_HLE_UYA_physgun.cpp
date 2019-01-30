#include "stdafx.h"
#include <functional>
#include "RPCS3/PPUThread.h"
#include "RPCS3_HLE/HLE_utils.h"
#include "vec4.h"
#include <sstream>

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

struct
{
	u32 func_spawn_ray;
	u32 func_coll_line;
	u32 u32_cant_shoot;
	u32 ptr_controller_data;
	u32 ptr_looking_pos;
	u32 ptr_ratchet_moby;
	u32 ptr_collision_pos;
	u32 ptr_collision_moby;
	u32 return_address;
} offsets{};

extern "C" void __declspec(dllexport) __cdecl physgun_tick(ppu_thread& ppu)
{
	vm::ptr<spawn_ray_func> spawn_ray{ offsets.func_spawn_ray };
	vm::ptr<CollLine> coll_line{ offsets.func_coll_line };
	vm::ptr<Vec4> vec_0{ api::stack_alloc(ppu, sizeof(Vec4), alignof(Vec4)) };

	vm::ptr<Vec4> start_pos{static_cast<u32>(ppu.gpr[28])};

	if (physgun_data.physics > 0)
	{
		--physgun_data.physics;
	}

	if (!api::read32(offsets.u32_cant_shoot))
	{
		if (api::read32(api::read32(offsets.ptr_controller_data) + 0xA0) & 0x28 && !physgun_data.physics)
		{
			if (!physgun_data.selected_moby)
			{
				vm::ptr<Vec4> end_pos{ offsets.ptr_looking_pos };
				*end_pos += end_pos->sub_vec4(*start_pos);
				vm::ptr<void> ratchet_moby{ api::read32(offsets.ptr_ratchet_moby) };
				coll_line(ppu, start_pos, end_pos, 0x20, ratchet_moby, 0);
				end_pos.set(offsets.ptr_collision_pos);
				vm::ptr<void> sel_moby{ api::read32(offsets.ptr_collision_moby) };
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
		vm::ptr<void> ratchet_moby{ api::read32(offsets.ptr_ratchet_moby) };
		physgun_data.ratchet_pos = *vm::ptr<Vec4>{ratchet_moby.addr() + 0x10};
		physgun_data.ray_norm_vector = vm::ptr<Vec4>{ offsets.ptr_looking_pos }->sub_vec4(physgun_data.ratchet_pos).norm();
		*vm::ptr<Vec4>{physgun_data.selected_moby.addr() + 0x10} = physgun_data.ratchet_pos.add_vec4(physgun_data.ray_norm_vector.mul_i(physgun_data.ray_length)).add_vec4(physgun_data.moby_offset);
		vm::ptr<Vec4> end_pos{ api::stack_alloc(ppu, sizeof(Vec4), alignof(Vec4)) };
		*end_pos = physgun_data.ratchet_pos.add_vec4(physgun_data.ray_norm_vector.mul_i(physgun_data.ray_length));
		spawn_ray(ppu, start_pos, vec_0, end_pos, 1, 0.0f);
		api::stack_dealloc(ppu, end_pos.addr(), sizeof(Vec4));

		api::draw_square(20, 105, 150, 25, color4f(0.f, 0.f, 0.f, 0.6f));
		std::ostringstream ostr;
		ostr << "Moby ID: 0x" << std::hex << (api::read32(physgun_data.selected_moby.addr() + 0xAA) >> 16);
		api::draw_text(ostr.str(), 20, 100, 16, color4f(1.f, 1.f, 1.f, 1.f), "Arial");
	}
	api::stack_dealloc(ppu, vec_0.addr(), sizeof(Vec4));
	RETURN_TO(offsets.return_address);
}

extern "C" void __declspec(dllexport) __cdecl init_dll(const std::vector<void*>& table)
{
	api::reg_api<api::api_table_t>::register_table(table);

	if (api::hash == "PPU-2b534405de98e28b1368947c8f14d9a386bc53de")
	{
		// BCES01503
		offsets.func_spawn_ray = 0xBE6FF0;
		offsets.return_address = 0x5A1734;
	}
	else if (api::hash == "PPU-25754a689f16c96ab071a738b8e46e4e2e12b327")
	{
		// NPEA00387
		offsets.func_spawn_ray = 0xBE6FE8;
		offsets.return_address = 0x5A12F4;
	}
	offsets.func_coll_line = 0xBD5790;
	offsets.u32_cant_shoot = 0xEE9334;
	offsets.ptr_controller_data = 0xC1D574;
	offsets.ptr_looking_pos = 0xDA3E40;
	offsets.ptr_ratchet_moby = 0xDA4DB0;
	offsets.ptr_collision_pos = 0xEE97A0;
	offsets.ptr_collision_moby = 0xEE9798;
}
