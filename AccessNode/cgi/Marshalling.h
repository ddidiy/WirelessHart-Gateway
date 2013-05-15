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

#ifndef _MARSHALLING_H_
#define _MARSHALLING_H_



//JSON_GET_MANDATORY* either initialise the variable or return InvalidParameters.
//	No need to pre-initialise the variable
//TAKE CARE: use predeclared:
//	struct json_object*& cmd.inObj,
//	struct json_object*& cmd.outObj
#define JSON_GET_MANDATORY(_name_, _var_, _getter_) 						\
	{	struct json_object *tmp = json_object_object_get (cmd.inObj, (_name_));	\
		if ( !tmp ) return InvalidParameters( cmd.outObj, _name_ );				\
		_var_ = _getter_( tmp ); 											\
	}

//TAKE CARE: use predeclared:
//	struct json_object*& cmd.inObj,
#define JSON_GET_DEFAULT(_name_,_var_,_getter_,_default_) 					\
	{	struct json_object *tmp = json_object_object_get (cmd.inObj, (_name_));	\
		if ( tmp ) _var_ = _getter_( tmp ); 								\
		else _var_=(_default_); 											\
	}


#define JSON_GET_MANDATORY_INT(_name_,_var_)  JSON_GET_MANDATORY(_name_,_var_,json_object_get_int)
#define JSON_GET_MANDATORY_BOOL(_name_,_var_)  JSON_GET_MANDATORY(_name_,_var_,json_object_get_boolean)
#define JSON_GET_MANDATORY_ARRAY(_name_,_var_) { JSON_GET_MANDATORY(_name_,_var_,json_object_get_array); if ( !_var_ ) return InvalidParameters( cmd.outObj, _name_ ) ;}
// Does not copy string to local buffer. User is not allowed to change it
#define JSON_GET_MANDATORY_STRING_PTR(_name_,_var_)  JSON_GET_MANDATORY(_name_,_var_,json_object_get_string)
// expects a pre-allocated buffer _var_ of at least _size_ bytes
#define JSON_GET_MANDATORY_STRING(_name_, _var_,_size_) 			\
	{	char * _ptr_(NULL);						\
		JSON_GET_MANDATORY((_name_),_ptr_,json_object_get_string)	\
		memmove(_var_, _ptr_, strlen(_ptr_));				\
		_var_[strlen(_ptr_)] = 0;						\
	}

#define JSON_GET_DEFAULT_INT(_name_, _var_,_default_)  JSON_GET_DEFAULT((_name_),_var_,json_object_get_int,(_default_))
#define JSON_GET_DEFAULT_BOOL(_name_, _var_,_default_) JSON_GET_DEFAULT((_name_),_var_,json_object_get_boolean,(_default_))
// Does not copy string to local buffer. User is not allowed to change it
#define JSON_GET_DEFAULT_STRING_PTR(_name_, _var_,_default_) JSON_GET_DEFAULT((_name_),_var_,json_object_get_string,(char*)(_default_))
// expects a pre-allocated buffer _var_ of at least _size_ bytes
#define JSON_GET_DEFAULT_STRING(_name_, _var_,_size_,_default_) 	\
	{	char * _ptr_;JSON_GET_DEFAULT((_name_),_ptr_,json_object_get_string,(char*)(_default_))	\
		memmove(_var_, _ptr_, strlen(_ptr_));			\
		_var_[strlen(_ptr_)] = 0;					\
	}

#define JSON_GET_ORDER_BY(_var_,_default_)									\
	{	char *pOrderBy;														\
		JSON_GET_DEFAULT_STRING_PTR( (char*)"orderBy", pOrderBy, _default_);\
		if ( (_var_ = GetOrderTypeId(pOrderBy)) == ORDER_UNK)				\
			return InvalidParameters( cmd.outObj, "invalid orderBy" ); 			\
	}

#define JSON_GET_ORDER_DIR(_var_,_default_)											\
	{	char *pOrderDir;															\
		JSON_GET_DEFAULT_STRING_PTR( (char*)"orderDirection", pOrderDir, _default_);\
		if ( (_var_ = GetOrderDirectionType(pOrderDir)) == ORDER_DIR_UNK)			\
			return InvalidParameters( cmd.outObj, "invalid orderDirection" ); 			\
	}

#define JSON_RETURN_BOOL(_bRet_,_meth_name_)																	\
	if ( (_bRet_) )	json_object_object_add(cmd.outObj, "result", json_object_new_boolean( (_bRet_) ) );				\
	else			json_object_object_add(cmd.outObj, "error", json_object_new_string( _meth_name_ " failed"));	\
	return (_bRet_) ;

#define JSON_RETURN_STRING(_szRet_,_meth_name_)																		\
	if ( (_szRet_) )	json_object_object_add(cmd.outObj, "result", json_object_new_string( (_szRet_) ) );				\
	else				json_object_object_add(cmd.outObj, "error", json_object_new_string( _meth_name_ " failed"));	\
	return (int)(_szRet_) ;


#define JSON_RETURN_ON_ERR(_ret_,_meth_name_)															\
	{ if ( !(_ret_) )																					\
		{	json_object_object_add(cmd.outObj, "error", json_object_new_string( _meth_name_ " failed"));	\
			return (int)(_ret_) ;																		\
		}																								\
	}



#endif	// _MARSHALLING_H_
