#ifndef _ANY_H_
#define _ANY_H_

/**
 *
 * Valued Conversion class. See Kevlin Henney[1]
 * [1] http://www.two-sdg.demon.co.uk/curbralan/papers/ValuedConversions.pdf
 *
 * int main()
 * {
 *	std::vector<any> any_vec ;
 *	const char * value="some string\n";
 *	int a = 5 ;
 *	any_vec.push_back( value ) ;
 *	any_vec.push_back( a ) ;
 *	std::cout << any_cast<const char*>(any_vec[0]);   ;
 *	std::cout << any_cast<int>(any_vec[1]);   ;
 *	 return 0 ;
 * }
 */

#include <typeinfo>
#include <algorithm>

class any
{
public:
	any()
		: content(0)
	{
	}
	template<typename value_type>
		any(const value_type &value)
		: content(new holder<value_type>(value))
		{
		}
	any(const any &other)
		: content(other.content ? other.content->clone() : 0)
		{
		}
	~any()
	{
		delete content;
	}
public: // modifiers
	any& swap(any /*&*/rhs)
	{
		std::swap(content, rhs.content);
		return *this;
	}
	any& operator=(const any &rhs)
	{
		any(rhs).swap(*this) ;
		return *this;
	}
	template<typename value_type>
		any &operator=(const value_type &rhs)
		{
			return swap(any(rhs));
		}
public: // queries
	bool empty() const
	{
		return !content ;
	}
	const std::type_info& type_info() const
	{
		return content
			? content->type_info()
			: typeid(void) ;
	}

	operator const void *() const
	{
		return content;
	}
	template<typename value_type>
		bool copy_to(value_type &value) const
		{
			const value_type *copyable =to_ptr<value_type>();
			if(copyable)
				value = *copyable;
			return copyable;
		}
	template<typename value_type>
		const value_type *to_ptr() const
		{
			return type_info() == typeid(value_type)
				? &static_cast<holder<value_type> *>(content)->held
				: 0;
		}
private:
	class placeholder
	{
		public:
			virtual ~placeholder()
			{
			}
			virtual const std::type_info & type_info() const = 0 ;
			virtual placeholder* clone() const = 0 ;
	};
	template < typename value_type>
		class holder : public placeholder
	{
		public:
			holder(const value_type& value )
				: held(value)
			{
			}
		public:
			virtual const std::type_info& type_info() const
			{
				return typeid(value_type);
			}
			virtual placeholder* clone() const
			{
				return new holder(held) ;
			}
			const value_type held ;
		private: // intentionally left unimplemented
			holder & operator=(const holder &);
	};
	placeholder * content ;
private:
	template<typename value_type>
		friend value_type* any_cast(any *operand) ;
	template<typename value_type>
		friend value_type* any_cast(const any *operand) ;
	template<typename value_type>
		friend value_type any_cast(const any &operand) ;
};

	template<typename value_type>
inline value_type* any_cast(any *operand)
{
	return operand && operand->type_info() == typeid(value_type)
		? &static_cast<any::holder<value_type> *>(operand->content)->held
		: 0 ;
}
	template<typename value_type>
inline value_type* any_cast(const any *operand)
{
	return any_cast<value_type>(const_cast<any*>(operand));
}
	template<typename value_type>
inline value_type any_cast(const any &operand)
{
	const value_type *result =
		operand.to_ptr<value_type>();
	return result ? *result : throw std::bad_cast();
}

#endif	/*_ANY_H_*/
