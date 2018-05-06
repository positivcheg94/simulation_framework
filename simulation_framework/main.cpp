#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <valarray>

#include <smpp/mmsim.hpp>
#include <smpp/smpp.hpp>
#include <smpp/task.hpp>
#include <smpp/processor.hpp>
#include <smpp/task_processor.hpp>

namespace po = boost::program_options;

template<typename It>
void write_to_stream(std::ostream& stream, It begin, It end, std::string separator = "-")
{
    stream << *begin;
    ++begin;
    while (begin!=end)
    {
        stream << separator << *begin;
        ++begin;
    }
}

int main(int argc, char* argv[])
{
    typedef smpp::SimpleTask                task;
    typedef smpp::Processor                 processor;
    typedef smpp::TaskProcessorWithTransfer	task_processor;

    typedef std::function<std::vector<double>(const size_t, const std::vector<smpp::Processor>& procs, std::vector<smpp::SimpleTask>&&, const smpp::TaskProcessor&)> fsimulator;

    try
    {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help"                 , "produce help message")
            ("sim_log"              , "log simulation data")
            //general simulation params
            ("problem_size"         , po::value<size_t>()->required()                           , "problem size"                                                )
            ("nominal_mips"         , po::value<double>()->default_value(1e10)                  , "nominal mips value"                                          )
            ("mips"                 , po::value<std::vector<double>>()->required()->multitoken(), "cores mips values as multiplication of nominal"              )
            ("task_priority"        , po::value<std::string>()->default_value("min")            , "task scheduling priority"                                    )
            ("proc_priority"        , po::value<std::string>()->default_value("min")            , "processor choosing priority"                                 )
            // task processor
            ("bandwidth"            , po::value<double>()->default_value(1e8)                   , "bandwidth with each processing unit (one value for all)"     )
            ("ping"                 , po::value<double>()->default_value(1e-5)                  , "ping with each processing unit (one value for all)"          )
            // slice params
            ("slices"               , po::value<std::vector<size_t>>()->multitoken()->required(), "slice params (min slice, max slice, step)"                   )
            ("fix_first"            , po::value<size_t>()->default_value(0)                     , "fixed first player strategy"                                 )

            ("randomize_count"      , po::value<size_t>()->default_value(1)                     , "how many times to simulate with shufling"                    )
            ("single_player"        , po::value<bool>()->default_value(false)                   , "make single player simulation"                               )

            ("output"               , po::value<std::string>()->default_value("results.txt")    , "output file"                                                 )
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(vm);

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
        else if (proc_priority == "max")
            proc_comparator = processor::fast_first();
        else
            throw po::validation_error(po::validation_error::invalid_option_value, "proc_priority");

        const double bandwidth	= vm["bandwidth"].as<double>();
        const double ping		= vm["ping"].as<double>();
        std::unique_ptr<task_processor> tp = std::make_unique<task_processor>(bandwidth, ping);

        const std::vector<size_t> slices_arr = vm["slices"].as<std::vector<size_t>>();
        std::vector<size_t> slices;
        if (slices_arr.size() == 3)
        {
            const size_t slice_start    = slices_arr[0];
            const size_t slice_end      = slices_arr[1];
            const size_t slice_step     = slices_arr[2];
            slices.reserve((slice_end - slice_start + 1) / slice_step);
            for (size_t i = slice_start; i < slice_end; i += slice_step)
                slices.push_back(i);
        }
        else
        {
            slices = slices_arr;
        }

        const size_t fix_first      = vm["fix_first"].as<size_t>();

        const size_t	randomize_count = vm["randomize_count"].as<size_t>();
        const bool		do_shuffle		= randomize_count != 0;
        const bool		single_player	= vm["single_player"].as<bool>();

        const bool sim_log = vm.count("sim_log") > 0;
        std::ofstream sim_log_file;

        std::string fname = vm["output"].as<std::string>();
        if (fname == "auto")
        {
            std::stringstream ss;
            ss << "sim_"    << proc_priority << "_" << task_priority;
            ss << "_n_"     << problem_size;
            ss << "_m_"     << std::scientific << std::setprecision(3) << nominal_mips;
            ss << "_bw_"    << std::scientific << std::setprecision(3) << bandwidth;
            ss << "_p_"     << std::scientific << std::setprecision(3) << ping;
            ss << "_rd_"    << randomize_count;
            if (single_player)
                ss << "single";
            ss << ".txt";
            fname = ss.str();
        }
        if (sim_log)
        {
            sim_log_file = std::ofstream(fname + ".log");
            sim_log_file.precision(20);
        }

        std::ofstream file(fname);
        file.precision(20);
        if (!file.is_open())
            throw std::runtime_error("couldn't open file");

        file << "ProblemSize=" << problem_size << "|||NominalMips=" << nominal_mips << "|||Bandwidth=" << bandwidth << "|||Ping=" << ping << std::endl;
        file << "Slices=";
        write_to_stream(file, slices.begin(), slices.end());
        file << std::endl;
        file << "MipsMultipliers=";
        write_to_stream(file, mips.begin(), mips.end());
        file << std::endl;


        if(single_player)
        {
            file << "Slice,Time" << std::endl;
            for (const auto& i : slices)
            {
                file << i << ',';
                auto tasks = smpp::mmsim::create_tasks<task>(problem_size, { i});
                auto result = simulate(procs, proc_comparator, tasks, task_comparator, *tp, 1, false, sim_log);
                if(sim_log)
                {
                    sim_log_file << "Log for slice=" << i << std::endl;
                    std::for_each(result.second.begin(), result.second.end(),[&sim_log_file](auto& val)
                    {
                        sim_log_file << val << std::endl;
                    });
                }
                file << result.first[0];
                file << std::endl;
                file.flush();
            }
        }
        else
        {
            file << "Slice First,Slice Second,Time First,Time Second" << std::endl;
            const std::vector<size_t> v{ fix_first };
            const std::vector<size_t>& f_s = fix_first == 0 ? slices : v;
            for (const auto& i : f_s)
                for (const auto&j : slices)
                {
                    file << i << ',' << j << ',';

                    std::valarray<double> times_array;
                    task_processor::return_type processed_tasks;
                    auto tasks_main = smpp::mmsim::create_tasks<task>(problem_size, { i, j });
                    std::tie(times_array, processed_tasks) = simulate(procs, proc_comparator, tasks_main, task_comparator, *tp, 2, do_shuffle, sim_log);
                    if (sim_log)
                    {
                        sim_log_file << "Log for slice1=" << i << "|slice2=" << j << std::endl;
                        sim_log_file.flush();
                        std::for_each(processed_tasks.begin(), processed_tasks.end(), [&sim_log_file](auto& val)
                        {
                            sim_log_file << val << std::endl;
                        });
                        sim_log_file.flush();
                    }
                    for (size_t times = 1; times < randomize_count; ++times)
                    {
                        auto tasks = smpp::mmsim::create_tasks<task>(problem_size, { i, j });
                        times_array += simulate(procs, proc_comparator, tasks, task_comparator, *tp, 2, do_shuffle).first;
                    }
                    if (randomize_count != 0)
                        times_array /= randomize_count;
                    file << times_array[0] << ',' << times_array[1];
                    file << std::endl;
                    file.flush();
                }
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
