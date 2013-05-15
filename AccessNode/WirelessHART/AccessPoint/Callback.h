/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef CALLBACK_H
#define CALLBACK_H

#include "ReferenceList.h"


/***
 * This code was originally written based on the techniques
 * described in http://www.codeproject.com/cpp/TTLFunction.asp
 * It was subsequently rewritten to follow the architecture
 * outlined in "Modern C++ Design" by Andrei Alexandrescu in
 * chapter 5, "Generalized Functors".
 *
 * This code uses:
 *   - default template parameters to saves users from having to
 *     specify empty parameters when the number of parameters
 *     is smaller than the maximum supported number
 *   - the pimpl idiom: the Callback class is passed around by
 *     value and delegates the crux of the work to its pimpl
 *     pointer.
 *   - two pimpl implementations which derive from CallbackImpl
 *     FunctorCallbackImpl can be used with any functor-type
 *     while MemPtrCallbackImpl can be used with pointers to
 *     member functions.
 *   - a reference list implementation to implement the Callback's
 *     value semantics.
 *
 * This code most notably departs from the alexandrescu
 * implementation in that it does not use type lists to specify
 * and pass around the types of the callback arguments.
 * Of course, it also does not use copy-destruction semantics
 * and relies on a reference list rather than auto_ptr to hold
 * the pointer.
 */
class empty {};

//////////////////////////////////////////////////////////////////////////////
/// declare the CallbackImpl class
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1, typename T2, typename T3, typename T4>
class CallbackImpl;


//////////////////////////////////////////////////////////////////////////////
/// define CallbackImpl for 0 params
//////////////////////////////////////////////////////////////////////////////
template <typename R>
class CallbackImpl<R,empty,empty,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (void) = 0;
};


//////////////////////////////////////////////////////////////////////////////
/// define CallbackImpl for 1 params
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1>
class CallbackImpl<R,T1,empty,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1) = 0;
};


//////////////////////////////////////////////////////////////////////////////
/// define CallbackImpl for 2 params
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1, typename T2>
class CallbackImpl<R,T1,T2,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2) = 0;
};


//////////////////////////////////////////////////////////////////////////////
/// define CallbackImpl for 3 params
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1, typename T2, typename T3>
class CallbackImpl<R,T1,T2,T3,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2, T3) = 0;
};


//////////////////////////////////////////////////////////////////////////////
/// define CallbackImpl for 4 params
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1, typename T2, typename T3, typename T4>
class CallbackImpl {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2, T3, T4) = 0;
};


//////////////////////////////////////////////////////////////////////////////
/// an impl for Functors:
//////////////////////////////////////////////////////////////////////////////
template <typename T, typename R, typename T1, typename T2, typename T3, typename T4>
class FunctorCallbackImpl : public CallbackImpl<R,T1,T2,T3,T4> {
public:
	FunctorCallbackImpl (T const &functor)
		: m_functor (functor) {}
	virtual ~FunctorCallbackImpl () {}
	R operator() (void) {
		return m_functor ();
	}
	R operator() (T1 a1) {
		return m_functor (a1);
	}
	R operator() (T1 a1,T2 a2) {
		return m_functor (a1,a2);
	}
	R operator() (T1 a1,T2 a2,T3 a3) {
		return m_functor (a1,a2,a3);
	}
	R operator() (T1 a1,T2 a2,T3 a3,T4 a4) {
		return m_functor (a1,a2,a3,a4);
	}
private:
	T m_functor;
};


//////////////////////////////////////////////////////////////////////////////
/// an impl for pointer to member functions
//////////////////////////////////////////////////////////////////////////////
template <typename OBJ_PTR, typename MEM_PTR, typename R, typename T1, typename T2, typename T3, typename T4>
class MemPtrCallbackImpl : public CallbackImpl<R,T1,T2,T3,T4> {
public:
	MemPtrCallbackImpl (OBJ_PTR const&obj_ptr, MEM_PTR mem_ptr)
		: m_obj_ptr (obj_ptr), m_mem_ptr (mem_ptr) {}
	virtual ~MemPtrCallbackImpl () {}
	R operator() (void) {
		return ((*m_obj_ptr).*m_mem_ptr) ();
	}
	R operator() (T1 a1) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1);
	}
	R operator() (T1 a1,T2 a2) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2);
	}
	R operator() (T1 a1,T2 a2,T3 a3) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2,a3);
	}
	R operator() (T1 a1,T2 a2,T3 a3,T4 a4) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2,a3,a4);
	}
private:
	OBJ_PTR const m_obj_ptr;
	MEM_PTR m_mem_ptr;
};


//////////////////////////////////////////////////////////////////////////////
/// declare and define Callback class
//////////////////////////////////////////////////////////////////////////////
template<typename R,
	 typename T1 = empty, typename T2 = empty,
	 typename T3 = empty, typename T4 = empty>
class Callback {
public:
	template <typename FUNCTOR>
	Callback (FUNCTOR const &functor)
		: m_impl (new FunctorCallbackImpl<FUNCTOR,R,T1,T2,T3,T4> (functor))
	{}

	template <typename OBJ_PTR, typename MEM_PTR>
	Callback (OBJ_PTR const &obj_ptr, MEM_PTR mem_ptr)
		: m_impl (new MemPtrCallbackImpl<OBJ_PTR,MEM_PTR,R,T1,T2,T3,T4> (obj_ptr, mem_ptr))
	{}

	Callback (ReferenceList<CallbackImpl<R,T1,T2,T3,T4> *> const &impl)
		: m_impl (impl)
	{}

	bool is_null (void) {
		return (m_impl.get () == 0)?true:false;
	}

	Callback () : m_impl () {}
	R operator() (void) {
		return (*(m_impl.get ())) ();
	}
	R operator() (T1 a1) {
		return (*(m_impl.get ())) (a1);
	}
	R operator() (T1 a1, T2 a2) {
		return (*(m_impl).get ()) (a1,a2);
	}
	R operator() (T1 a1, T2 a2, T3 a3) {
		return (*(m_impl).get ()) (a1,a2,a3);
	}
	R operator() (T1 a1, T2 a2, T3 a3, T4 a4) {
		return (*(m_impl).get ()) (a1,a2,a3,a4);
	}
private:
	ReferenceList<CallbackImpl<R,T1,T2,T3,T4>*> m_impl;
};

//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
template <typename OBJ, typename R>
Callback<R> make_callback (R (OBJ::*mem_ptr) (), OBJ *const obj_ptr) {
	return Callback<R> (obj_ptr, mem_ptr);
}
template <typename OBJ, typename R, typename T1>
Callback<R,T1> make_callback (R (OBJ::*mem_ptr) (T1), OBJ *const obj_ptr) {
	return Callback<R,T1> (obj_ptr, mem_ptr);
}
template <typename OBJ, typename R, typename T1, typename T2>
Callback<R,T1,T2> make_callback (R (OBJ::*mem_ptr) (T1,T2), OBJ *const obj_ptr) {
	return Callback<R,T1,T2> (obj_ptr, mem_ptr);
}
template <typename OBJ, typename R, typename T1,typename T2, typename T3>
Callback<R,T1,T2,T3> make_callback (R (OBJ::*mem_ptr) (T1,T2,T3), OBJ *const obj_ptr) {
	return Callback<R,T1,T2,T3> (obj_ptr, mem_ptr);
}
template <typename OBJ, typename R, typename T1, typename T2, typename T3, typename T4>
Callback<R,T1,T2,T3,T4> make_callback (R (OBJ::*mem_ptr) (T1,T2,T3,T4), OBJ *const obj_ptr) {
	return Callback<R,T1,T2,T3,T4> (obj_ptr, mem_ptr);
}

template <typename R>
Callback<R> make_callback (R (*fn_ptr) ()) {
	return Callback<R> (fn_ptr);
}
template <typename R, typename T1>
Callback<R,T1> make_callback (R (*fn_ptr) (T1)) {
	return Callback<R,T1> (fn_ptr);
}
template <typename R, typename T1, typename T2>
Callback<R,T1,T2> make_callback (R (*fn_ptr) (T1,T2)) {
	return Callback<R,T1,T2> (fn_ptr);
}
template <typename R, typename T1, typename T2,typename T3>
Callback<R,T1,T2,T3> make_callback (R (*fn_ptr) (T1,T2,T3)) {
	return Callback<R,T1,T2,T3> (fn_ptr);
}
template <typename R, typename T1, typename T2,typename T3,typename T4>
Callback<R,T1,T2,T3,T4> make_callback (R (*fn_ptr) (T1,T2,T3,T4)) {
	return Callback<R,T1,T2,T3,T4> (fn_ptr);
}


#endif /* CALLBACK_H */
