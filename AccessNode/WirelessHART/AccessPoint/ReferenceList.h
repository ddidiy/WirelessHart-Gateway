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

#ifndef REFERENCE_LIST_H
#define REFERENCE_LIST_H

/* This is a reference list implementation. The technique underlying
 * this code was first described in 1995 by Risto Lankinen on Usenet
 * but I have never been able to find his original posting. Instead,
 * this code is based on the description of the technique found in
 * "Modern C++ design" by Andrei Alexandrescu in chapter 7.
 */


template <typename OBJ_PTR>
class ReferenceList;

template <typename OBJ_PTR>
class ReferenceList {
public:
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	ReferenceList ()
		:  m_obj_ptr (),
		   m_prev (this),
		   m_next (this)
	{}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	ReferenceList (ReferenceList &o)
		: m_obj_ptr (),
		  m_prev (this),
		  m_next (this)
	{
		insert_self_in_other (o);
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	ReferenceList (ReferenceList const&o)
		: m_obj_ptr (),
		  m_prev (this),
		  m_next (this)
	{
		insert_self_in_other (o);
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	ReferenceList (OBJ_PTR const &obj_ptr)
		: m_obj_ptr (obj_ptr),
		  m_prev (this),
		  m_next (this)
	{}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	~ReferenceList () {
		remove_from_list ();
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	ReferenceList & operator= (ReferenceList const&o) {
		remove_from_list ();
		insert_self_in_other (o);
		return *this;
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	OBJ_PTR operator-> () {
		return m_obj_ptr;
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	void set (OBJ_PTR obj_ptr) {
		remove_from_list ();
		m_obj_ptr = obj_ptr;
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	OBJ_PTR get (void) {
		// explicit conversion to raw pointer type.
		return m_obj_ptr;
	}
private:
	void insert_self_in_other (ReferenceList const&o) {
		m_prev = &o;
		m_next = o.m_next;
		m_next->m_prev = this;
		o.m_next = this;
		m_obj_ptr = o.m_obj_ptr;
	}
	void remove_from_list (void) {
		if (m_prev == this) {
			//assert (m_next == this);
			delete m_obj_ptr;
			m_obj_ptr = OBJ_PTR ();
		}
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;
	}
	OBJ_PTR m_obj_ptr;
	mutable ReferenceList const*m_prev;
	mutable ReferenceList const*m_next;
};

#endif /* REFERENCE_LIST_H */
