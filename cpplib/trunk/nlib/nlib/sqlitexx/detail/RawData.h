#ifndef NLIB_SQLITEXX_RAWDATA_H_
#define NLIB_SQLITEXX_RAWDATA_H_

namespace nlib {
namespace sqlitexx {
namespace detail {


class RawData
{
public:
	RawData(char** data, int rows, int cols);
	~RawData();

	const char* RawValue(int row, int col) const;

	int RowsCount() const;

	int ColsCount() const;

private:

	char** data;
	int rowsCount;
	int colsCount;
};

class String
{
public:
	String();
	~String();

	char** operator& ();

	const char* Value() const;

private:
	char *str;
};


} // namespace detail
} // namespace sqlitexx
} // namspace nlib

#endif /*NLIB_SQLITEXX_RAWDATA_H_*/
