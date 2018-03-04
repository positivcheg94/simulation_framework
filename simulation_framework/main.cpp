#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>

#include <boost/algorithm/string.hpp>    
#include <boost/program_options.hpp>
#include <valarray>
namespace po = boost::program_options;

#include <smpp/mmsim.hpp>
#include <smpp/smpp.hpp>
#include <smpp/task.hpp>
#include <smpp/processor.hpp>
#include <smpp/task_processor.hpp>


int main(int argc, char* argv[])
{
	typedef smpp::SimpleTask				task;
	typedef smpp::Processor					processor;
	typedef smpp::TaskProcessorWithTransfer	task_processor;

	typedef std::function<std::vector<double>(const size_t, const std::vector<smpp::Processor>& procs, std::vector<smpp::SimpleTask>&&, const smpp::TaskProcessor&)> fsimulator;

	try
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			//general simulation params
			("problem_size"			, po::value<size_t>()->required()							, "problem size"												)
			("nominal_mips"			, po::value<double>()->default_value(1e10)					, "nominal mips value"											)
			("mips"					, po::value<std::vector<double>>()->required()->multitoken(), "cores mips values as multiplication of nominal"				)
			("task_priority"		, po::value<std::string>()->default_value("min")			, "task scheduling priority"									)
			("proc_priority"		, po::value<std::string>()->default_value("min")			, "processor choosing priority"									)
			// task processor
			("bandwidth"			, po::value<double>()->default_value(1e10)					, "bandwidth with each processing unit (one value for all)"		)
			("ping"					, po::value<double>()->default_value(1e-5)					, "ping with each processing unit (one value for all)"			)
			// slice params
			("slices"				, po::value<std::vector<size_t>>()->multitoken()->required(), "slice params (min slice, max slice, step)"					)

			("randomize_count"		, po::value<size_t>()->default_value(1)						, "how many times to simulate with shufling"					)

			("output"				, po::value<std::string>()->default_value("results.txt")	, "output file"													)
			;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if(vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}

		const size_t problem_size = vm["problem_size"].as<size_t>();
		const double nominal_mips = vm["nominal_mips"].as<double>();
		std::vector<double> mips = vm["mips"].as<std::vector<double>>();
		std::vector<processor> procs;
		procs.reserve(mips.size());
		std::for_each(mips.begin(), mips.end(),
			[nominal_mips, &procs](double val)
		{
			procs.emplace_back(nominal_mips*val);
		});

		auto task_priority = vm["task_priority"].as<std::string>();
		std::transform(task_priority.begin(), task_priority.end(), task_priority.begin(), ::tolower);
		task::comparator task_comparator;
		if(task_priority == "min")
			task_comparator = task::small_first();
		else if (task_priority == "max")
			task_comparator = task::large_first();
		else
			throw po::validation_error(po::validation_error::invalid_option_value, "task_priority");

		auto proc_priority = vm["proc_priority"].as<std::string>();
		std::transform(proc_priority.begin(), proc_priority.end(), proc_priority.begin(), ::tolower);
		processor::comparator proc_comparator;
		if (proc_priority == "min")
			proc_comparator = processor::slow_first();
		else if (task_priority == "max")
			proc_comparator = processor::fast_first();
		else
			throw po::validation_error(po::validation_error::invalid_option_value, "proc_priority");

		const double bandwidth	= vm["bandwidth"].as<double>();
		const double ping		= vm["ping"].as<double>();
		std::unique_ptr<task_processor> tp = std::make_unique<task_processor>(bandwidth, ping);

		const std::vector<size_t> slices = vm["slices"].as<std::vector<size_t>>();
		if(slices.size() != 3)
			throw po::validation_error(po::validation_error::invalid_option_value, "slices");
		const size_t slice_start	= slices[0];
		const size_t slice_end		= slices[1];
		const size_t slice_step		= slices[2];

		const size_t	randomize_count = vm["randomize_count"].as<size_t>();
		const bool		do_shuffle		= randomize_count == 0;

		const std::string fname = vm["output"].as<std::string>();

		std::ofstream file(fname);
		file.precision(20);
		if (!file.is_open())
			throw std::runtime_error("couldn't open file");
		file << "Slice First,Slice Second,Time First, Time Second" << std::endl;

		for(size_t i = slice_start; i < slice_end; i+=slice_step)
			for (size_t j = slice_start; j < slice_end; j += slice_step)
			{
				file << i << ',' << j << ',';
				auto tasks = smpp::mmsim::create_tasks<task>(problem_size, { i, j });
				std::valarray<double> result = simulate(procs, proc_comparator, std::move(tasks), task_comparator, *tp, 2, do_shuffle);
				for (size_t times = 1; times < randomize_count; ++times)
				{
					tasks	= smpp::mmsim::create_tasks<task>(problem_size, { i, j });
					result	+= simulate(procs, proc_comparator, std::move(tasks), task_comparator, *tp, 2, do_shuffle);
				}
				if (randomize_count != 0)
					result /= randomize_count;
				file << result[0] << ',' << result[1] << std::endl;
				file.flush();
			}
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Exception " << ex.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Exception of unknown type" << std::endl;
	}
	return 0;
}
