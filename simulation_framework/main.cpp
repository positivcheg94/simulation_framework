#include <list>
#include <vector>
#include <iostream>


#include <smpp/mmsim.hpp>
#include <smpp/smpp.hpp>
#include <smpp/task.hpp>


int main()
{
	const double nominal_mips = 10000000000.0;
	const std::vector<double> MIPS = { nominal_mips, nominal_mips, nominal_mips, nominal_mips };

	auto tasks = smpp::mmsim::create_tasks<smpp::SimpleTask>(5000, { 2,2 });
	auto res = smpp::simulate<smpp::SimpleTask, std::less<smpp::SimpleTask>, std::less<>, true>(MIPS, std::move(tasks));
	std::cout << res.begin()->expected_completition_time << std::endl;
	return 0;
}