/*
 * compiler_boosttest_formatter.h
 *
 *  Created on: Nov 21, 2008
 *      Author: nicu.dascalu
 */

#ifndef COMPILER_BOOSTTEST_FORMATTER_H_
#define COMPILER_BOOSTTEST_FORMATTER_H_

// FIXME [nicu.dascalu] - find another way to check if unit test has been ncluded
// should be included first #include <boost/test/unit_test.hpp>

namespace boost {
namespace unit_test {
namespace nlib {

/**
 * GCC complier style formatter, used for eclipse error navigation.
 */
class GCCOutputFormatter: public boost::unit_test::unit_test_log_formatter
{
public:
	GCCOutputFormatter()
	{
		ident = 0;
	}
private:
	void log_start(std::ostream& out, counter_t test_cases_amount);
	void log_finish(std::ostream& out);
	void log_build_info(std::ostream& out);
	void test_unit_start(std::ostream& out, test_unit const& tu);
	void test_unit_finish(std::ostream& out, test_unit const& tu, unsigned long elapsed);
	void test_unit_skipped(std::ostream& out, test_unit const&);
	void log_exception(std::ostream& out, log_checkpoint_data const&, const_string explanation);
	void log_entry_start(std::ostream& out, log_entry_data const&, log_entry_types let);
	void log_entry_value(std::ostream& out, const_string value);
	void log_entry_finish(std::ostream& out);

private:
	void print_prefix(std::ostream& output, const_string file, std::size_t line, const std::string& category);
	void print_ident(std::ostream& output);
	int ident;
};

inline
void GCCOutputFormatter::log_start(std::ostream& output, counter_t test_cases_amount)
{
	if (test_cases_amount > 0)
		output << "Running " << test_cases_amount << " test " << (test_cases_amount > 1
		  ? "cases"
		  : "case") << "...\n";
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_finish(std::ostream& ostr)
{
	ostr.flush();
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_build_info(std::ostream& output)
{
	output << "Platform: " << BOOST_PLATFORM << '\n' << "Compiler: " << BOOST_COMPILER << '\n' << "STL     : "
	    << BOOST_STDLIB << '\n' << "Boost   : " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
	    << BOOST_VERSION % 100 << std::endl;
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::test_unit_start(std::ostream& output, test_unit const& tu)
{
	print_ident(output);
	output << tu.p_name << " entering...";
	ident++;
	output << std::endl;
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::test_unit_finish(std::ostream& output, test_unit const& tu, unsigned long elapsed)
{
	ident--;
	print_ident(output);
	output << tu.p_name << " left.";

	if (elapsed > 0)
	{
		output << "; testing time: ";
		if (elapsed % 1000 == 0)
			output << elapsed / 1000 << "ms";
		else
			output << elapsed << "mks";
	}

	output << std::endl;
	output.flush();
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::test_unit_skipped(std::ostream& output, test_unit const& tu)
{
	print_ident(output);
	output << "Test " << tu.p_type_name << " \"" << tu.p_name << "\"" << "is skipped" << std::endl;
	output.flush();
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_exception(std::ostream& output, log_checkpoint_data const& checkpoint_data,
  const_string explanation)
{
	print_ident(output);
//	print_prefix(output, "[Boost.Test]", 0, "error");
//	output << '\n';
	print_prefix(output, checkpoint_data.m_file_name, checkpoint_data.m_line_num, "error");
	output << " Exception in \"" << framework::current_test_case().p_name << "\"= ";

	if (!explanation.is_empty()){
	    std::string msg(explanation.begin(), explanation.size());
        std::replace(msg.begin(), msg.end(), ':', '_');
		output << msg;
	}else {
		output << "uncaught exception, system error or abort requested";
	}
//	output << " in \"" << framework::current_test_case().p_name;

	if (!checkpoint_data.m_file_name.is_empty())
	{
//				output << '\n';
//				output << "File:" << checkpoint_data.m_file_name << '\n';
//				output << "Line:" << checkpoint_data.m_line_num << '\n';
//				output << "Exception:" << explanation << '\n';
//				print_prefix(output, checkpoint_data.m_file_name, checkpoint_data.m_line_num, false);
//				output << "last checkpoint";
		if (!checkpoint_data.m_message.empty())
			output << "= " << checkpoint_data.m_message;
	}

	output << std::endl;
	output.flush();
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_entry_start(std::ostream& output, log_entry_data const& entry_data, log_entry_types let)
{
	print_ident(output);

	switch (let)
	{
	case BOOST_UTL_ET_INFO:
		print_prefix(output, entry_data.m_file_name, entry_data.m_line_num, "info");
		break;
	case BOOST_UTL_ET_MESSAGE:
		break;
	case BOOST_UTL_ET_WARNING:
		print_prefix(output, entry_data.m_file_name, entry_data.m_line_num, "warning");
		output << " in \"" << framework::current_test_case().p_name << "\": ";
		break;
	case BOOST_UTL_ET_ERROR:
		print_prefix(output, entry_data.m_file_name, entry_data.m_line_num, "error");
		output << " in \"" << framework::current_test_case().p_name << "\": ";
		break;
	case BOOST_UTL_ET_FATAL_ERROR:
		print_prefix(output, entry_data.m_file_name, entry_data.m_line_num, "error");
		output << " 'fatal error' in \"" << framework::current_test_case().p_name << "\": ";
		break;
	}
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_entry_value(std::ostream& output, const_string value)
{
	output << value;
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::log_entry_finish(std::ostream& output)
{
	output << std::endl;
	output.flush();
}

//____________________________________________________________________________//

inline
void GCCOutputFormatter::print_prefix(std::ostream& output, const_string file, std::size_t line,
  const std::string& category)
{
	output << file << ':' << line << ":" << category << ": [Boost.Test]";
}

inline
void GCCOutputFormatter::print_ident(std::ostream& output)
{
	for (int i = 0; i < ident; i++)
		output << "  ";
}

} //namsapce nlib
} //namspace test
} //namspace boost


#endif /* COMPILER_BOOSTTEST_FORMATTER_H_ */
