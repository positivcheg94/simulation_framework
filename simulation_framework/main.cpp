#include <list>
#include <vector>
#include <iostream>


#include <smpp/mmsim.hpp>
#include <smpp/smpp.hpp>
#include <smpp/task.hpp>
#include <smpp/processor.hpp>


int main()
{
	typedef smpp::SimpleTask task;
	typedef smpp::Processor processor;
	const double nominal_mips = 10000000000.0;
	std::vector<processor> mips;
	mips.emplace_back(nominal_mips);
	mips.emplace_back(nominal_mips);
	mips.emplace_back(nominal_mips);
	mips.emplace_back(nominal_mips);

	auto tasks = smpp::mmsim::create_tasks<task>(5000, { 2,2 });
	auto res = smpp::simulate<processor,task, task::small_first, processor::slow_first>(mips, std::move(tasks));

	std::cout << res.begin()->expected_completition_time << std::endl;
	return 0;
}