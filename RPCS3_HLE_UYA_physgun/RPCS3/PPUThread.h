#pragma once
#include "CPUThread.h"
#include "Utilities/BEType.h"


class ppu_thread : public cpu_thread
{
public:
	u64 gpr[32] = {}; // General-Purpose Registers
	f64 fpr[32] = {}; // Floating Point Registers
	v128 vr[32] = {}; // Vector Registers

	alignas(16) bool cr[32] = {}; // Condition Registers (unpacked)

	alignas(16) struct // Floating-Point Status and Control Register (unpacked)
	{
		// TODO
		bool _start[16]{};
		bool fl{}; // FPCC.FL
		bool fg{}; // FPCC.FG
		bool fe{}; // FPCC.FE
		bool fu{}; // FPCC.FU
		bool _end[12]{};
	}
	fpscr;

	u64 lr{}; // Link Register
	u64 ctr{}; // Counter Register
	u32 vrsave{ 0xffffffff }; // VR Save Register
	u32 cia{}; // Current Instruction Address

	// Pack CR bits
	u32 cr_pack() const
	{
		u32 result{};

		for (u32 bit : cr)
		{
			result = (result << 1) | bit;
		}

		return result;
	}

	// Unpack CR bits
	void cr_unpack(u32 value)
	{
		for (bool& b : cr)
		{
			b = (value & 0x1) != 0;
			value >>= 1;
		}
	}

	// Fixed-Point Exception Register (abstract representation)
	struct
	{
		bool so{}; // Summary Overflow
		bool ov{}; // Overflow
		bool ca{}; // Carry
		u8 cnt{};  // 0..6
	}
	xer;

	/*
		Saturation. A sticky status bit indicating that some field in a saturating instruction saturated since the last
		time SAT was cleared. In other words when SAT = '1' it remains set to '1' until it is cleared to '0' by an
		mtvscr instruction.
		1	The vector saturate instruction implicitly sets when saturation has occurred on the results one of
			the vector instructions having saturate in its name:
			Move To VSCR (mtvscr)
			Vector Add Integer with Saturation (vaddubs, vadduhs, vadduws, vaddsbs, vaddshs,
			vaddsws)
			Vector Subtract Integer with Saturation (vsububs, vsubuhs, vsubuws, vsubsbs, vsubshs,
			vsubsws)
			Vector Multiply-Add Integer with Saturation (vmhaddshs, vmhraddshs)
			Vector Multiply-Sum with Saturation (vmsumuhs, vmsumshs, vsumsws)
			Vector Sum-Across with Saturation (vsumsws, vsum2sws, vsum4sbs, vsum4shs,
			vsum4ubs)
			Vector Pack with Saturation (vpkuhus, vpkuwus, vpkshus, vpkswus, vpkshss, vpkswss)
			Vector Convert to Fixed-Point with Saturation (vctuxs, vctsxs)
		0	Indicates no saturation occurred; mtvscr can explicitly clear this bit.
	*/
	bool sat{};

	/*
		Non-Java. A mode control bit that determines whether vector floating-point operations will be performed
		in a Java-IEEE-C9X-compliant mode or a possibly faster non-Java/non-IEEE mode.
		0	The Java-IEEE-C9X-compliant mode is selected. Denormalized values are handled as specified
			by Java, IEEE, and C9X standard.
		1	The non-Java/non-IEEE-compliant mode is selected. If an element in a source vector register
			contains a denormalized value, the value '0' is used instead. If an instruction causes an underflow
			exception, the corresponding element in the target vr is cleared to '0'. In both cases, the '0'
			has the same sign as the denormalized or underflowing value.
	*/
	bool nj = true;

	u32 raddr{ 0 }; // Reservation addr
	u64 rtime{ 0 };
	u64 rdata{ 0 }; // Reservation data

	atomic_t<u32> prio{ 0 }; // Thread priority (0..3071)
	const u32 stack_size; // Stack size
	const u32 stack_addr; // Stack address

	atomic_t<u32> joiner{ ~0u }; // Joining thread (-1 if detached)

	lf_fifo<atomic_t<cmd64>, 127> cmd_queue; // Command queue for asynchronous operations.

	u64 start_time{ 0 }; // Sleep start timepoint
	const char* last_function{}; // Last function name for diagnosis, optimized for speed.

	lf_value<std::string> ppu_name; // Thread name
};