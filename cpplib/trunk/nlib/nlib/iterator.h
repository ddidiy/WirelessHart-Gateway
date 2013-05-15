#ifndef NLIB_ITERATOR_H
#define NLIB_ITERATOR_H

#include <iterator>


namespace nlib {

/**
 *  @brief  Turns assignment into extraction.
 *
 *  These are input iterators, constructed from a container-of-T.
 *  Assigning to a T the iterator removes it to the container using
 *  pop_front.
 *
 *  Container::pop_front should return a pointer to the next T in the container.
 *
 *  Tip:  Using the front_extractor function to create these iterators can
 *  save typing.
 */
template<typename _Container>
class front_extract_iterator :
	public std::iterator< std::input_iterator_tag, typename _Container::reference, void, void, void>
{
public:
	/// A nested typedef for the type of whatever container you used.
	typedef _Container container_type;
	typedef typename _Container::reference iterator_type;

protected:
	container_type* container;

public:
	/// The only way to create this %iterator is with a container.
	explicit front_extract_iterator(container_type& __x) :
		container(&__x)
	{
	}

	/**
	 *  @param  value  An instance of whatever type
	 *                 container_type::const_reference is; presumably a
	 *                 reference-to-const T for container<T>.
	 *  @return  This %iterator, for chained operations.
	 *
	 *  This kind of %iterator doesn't really have a "position" in the
	 *  container (you can think of the position as being permanently at
	 *  the end, if you like).  Assigning a value to the %iterator will
	 *  always append the value to the end of the container.
	 */
	front_extract_iterator&
	operator=(typename _Container::const_reference __value)
	{
		return *this;
	}

	/// Pops a value from the container. When a value is read, the iterator moves to the next value.
	iterator_type
	operator*()
	{
		iterator_type temp = container->front();
		container->pop_front();
		return temp;
	}

	// Simply returns *this.  (This %iterator does not "move".)
	front_extract_iterator&
	operator++()
	{

		return *this;
	}

	// Simply returns *this.  (This %iterator does not "move".)
	front_extract_iterator operator++(int)
	{
		return *this;
	}
};

/**
 *  @param  x  A container of arbitrary type.
 *  @return  An instance of back_insert_iterator working on @p x.
 *
 *  This wrapper function helps in creating front_extract_iterator instances.
 *  Typing the name of the %iterator requires knowing the precise full
 *  type of the container, which can be tedious and impedes generic
 *  programming.  Using this function lets you take advantage of automatic
 *  template parameter deduction, making the compiler match the correct
 *  types for you.
 */
template<typename _Container>
inline front_extract_iterator<_Container> front_extracter(_Container& __x)
{
	return front_extract_iterator<_Container> (__x);
}

} //namespace nlib

#endif /*NLIB_ITERATOR_H*/
