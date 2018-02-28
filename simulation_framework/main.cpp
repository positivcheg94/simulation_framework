#include <list>
#include <vector>
#include <iostream>


#include <smpp/mmsim.hpp>
#include <smpp/smpp.hpp>
#include <smpp/task.hpp>
#include <smpp/processor.hpp>
#include <smpp/task_processor.hpp>


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

	auto tasks = smpp::mmsim::create_tasks<task>(5000, { 50,50 });
	smpp::TaskProcessorWithTicker tp(0.001);
	//smpp::TaskProcessor tp;
	auto res = smpp::simulate<processor,task, task::small_first, processor::slow_first>(mips, std::move(tasks), tp);

	std::cout << res.back().expected_completition_time << std::endl;
	return 0;
}
