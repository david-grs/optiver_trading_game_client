#pragma once

#include <chrono>
#include <thread>
#include <type_traits>

class TSCClock
{
public:
	static void Initialise();

	static uint64_t Now();
	static std::chrono::nanoseconds FromCycles(uint64_t);

	template <class DurationT>
	static uint64_t ToCycles(DurationT);

private:
	static double& GetFrequencyGHz()
	{
		static double TSCFreqGHz = .0;
		return TSCFreqGHz;
	}
};

namespace detail
{

inline uint64_t rdtscp()
{
	uint64_t rax, rcx, rdx;
	__asm__ __volatile__("rdtscp" : "=a"(rax), "=d"(rdx), "=c"(rcx));
	return (rdx << 32) + rax;
}

inline void cpuid()
{
	uint64_t rax, rbx, rcx, rdx;
	__asm__ __volatile__("cpuid" : "=a"(rax), "=b"(rbx), "=d"(rdx), "=c"(rcx));
}

inline uint64_t rdtscp(int& chip, int& core)
{
	uint64_t rax, rcx, rdx;
	__asm__ __volatile__("rdtscp" : "=a"(rax), "=d"(rdx), "=c"(rcx));
	chip = static_cast<int>((rcx & 0xFFF000) >> 12);
	core = static_cast<int>(rcx & 0xFFF);
	return (rdx << 32) + rax;
}

}

inline uint64_t TSCClock::Now()
{
	return detail::rdtscp();
}

inline std::chrono::nanoseconds TSCClock::FromCycles(uint64_t cycles)
{
	const double nanoseconds{static_cast<double>(cycles) / GetFrequencyGHz()};
	return std::chrono::nanoseconds(static_cast<uint64_t>(nanoseconds));
}

template <class DurationT>
inline uint64_t TSCClock::ToCycles(DurationT duration)
{
	const double nanoseconds{static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count())};
	return static_cast<uint64_t>(nanoseconds * GetFrequencyGHz());
}


inline void TSCClock::Initialise()
{
	double& tscFreq = GetFrequencyGHz();
	if (tscFreq != .0)
	{
		return;
	}

	using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
									 std::chrono::high_resolution_clock,
									 std::chrono::steady_clock>;

	int chip, core, chip2, core2;

	auto start = Clock::now();

	detail::cpuid();
	uint64_t rdtsc_start = detail::rdtscp(chip, core);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	uint64_t rdtsc_end = detail::rdtscp(chip2, core2);
	detail::cpuid();

	auto end = Clock::now();

	if (core != core2 || chip != chip2)
		throw std::runtime_error("tsc_clock: process needs to be pin to a specific core");

	auto duration_s = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	uint64_t cycles = rdtsc_end - rdtsc_start;

	tscFreq = static_cast<double>(cycles) / static_cast<double>(duration_s.count());
}


