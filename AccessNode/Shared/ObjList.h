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

/***************************************************************************
                          ObjList.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    email                : cristian.giurgea@nivis.com
 ***************************************************************************/
// ObjList.h: interface for the ObjList class.

//lint -library 

//////////////////////////////////////////////////////////////////////

// for the NULL pointer constant

#if !defined(AFX_OBJLIST_H__137C1D9B_EAD6_4B7B_A21D_88C69E91EE4F__INCLUDED_)
#define AFX_OBJLIST_H__137C1D9B_EAD6_4B7B_A21D_88C69E91EE4F__INCLUDED_

#include <time.h>

/// @addtogroup libshared
/// @{

//typedef struct{}* OBJ_POSITION;

//#if !defined(SAFE_DELETE)
//#	define SAFE_DELETE(x)	if (x) {delete x; x = NULL;}
//#endif
template<typename T>
class elem{
public:
    elem();
    ~elem();
    T obj;
    elem<T>* next;
    elem<T>* prev;
};

template<typename T>
elem<T>::elem()
{
	next = (elem<T>*)NULL;
	prev = (elem<T>*)NULL;
}

template<typename T>
elem<T>::~elem()
{
}

//typedef struct{}* OBJ_POSITION;
typedef void* OBJ_POSITION;

template<typename T> class ObjList 
{
	public:
		ObjList();
		virtual ~ObjList();

		void AddGenericHead();
		void AddGenericTail();

//		virtual OBJ_POSITION AddHead( T );
//
//		WARNING: don't EVER put virtual where we did not.  
//
		OBJ_POSITION AddHead( T );
		OBJ_POSITION AddTail( T );
		void RemoveAll();
virtual void RemoveAt( OBJ_POSITION& );
		
		T& GetHead();
    	T& GetTail();
		OBJ_POSITION GetHeadPosition();
		OBJ_POSITION GetTailPosition();
		T& GetNext( OBJ_POSITION& );
		T& GetPrev( OBJ_POSITION& );
		T& GetAt( OBJ_POSITION );
        bool SetAt( OBJ_POSITION pos, T& newElem );
		OBJ_POSITION InsertAfter( OBJ_POSITION pos, T& newElem );
		OBJ_POSITION InsertBefore( OBJ_POSITION pos, T& newElem );

		int GetCount();

		void RemoveHead();
		void RemoveTail();

		void RemoveElement( T& tElem );
		void RemoveElement( T& tElem, int (*fcomp) ( T&, T& ) );
		//methods added by marcel
		//an iterator.
		T& operator []( OBJ_POSITION& p_pos ){ return GetAt(p_pos); }
		OBJ_POSITION GetNextPosition( OBJ_POSITION& p_pos );
        OBJ_POSITION RemoveAndGetNextPosition( OBJ_POSITION& p_pos);
		
	//data members
		int count;
		elem<T>* headNode;
		elem<T>* tailNode;
};

template<typename T> 
ObjList<T>::ObjList()
{
    headNode = (elem<T>*)NULL;
    tailNode = (elem<T>*)NULL;
    count = 0;
}

template<typename T> 
ObjList<T>::~ObjList()
{
    RemoveAll();
}

template<typename T> 
OBJ_POSITION ObjList<T>::AddHead( T head )
{
	elem<T>* addedHead = new elem<T>;

	if( count == 0 )
	{
		tailNode = headNode = addedHead;
		headNode->prev = tailNode->next = (elem<T>*)NULL;
		headNode->obj = head;
	}
	else
	{
		addedHead->next = headNode;
		addedHead->prev = (elem<T>*)NULL;
		addedHead->obj = head;
		headNode->prev = addedHead;
		headNode = addedHead;
	}
	count++;
	return (OBJ_POSITION)headNode;
}

template<typename T> OBJ_POSITION ObjList<T>::AddTail( T tail )
{
	if( count != 0 )
	{
		elem<T>* addedTail = new elem<T>;
		addedTail->prev = tailNode;
		addedTail->next = (elem<T>*)NULL;
		addedTail->obj = tail;
		tailNode->next = addedTail;
    tailNode = addedTail;
    count ++;
	}
	else
		AddHead( tail );

	return (OBJ_POSITION)tailNode;
}

 

template<typename T> void ObjList<T>::RemoveAll()
{
	OBJ_POSITION pos = GetHeadPosition();

	/*while( pos )
	{
		temp = pos;
		GetNext( pos );
		RemoveAt( temp );
	}*/

  if ( !pos )
    return;

  elem< T >* pobj = ( elem< T >* )pos;

  do
  {
    elem< T >* pnobj = pobj->next;

    delete pobj;

    pobj = pnobj;
  }
  while( pobj );

  headNode = tailNode = (elem<T>*)NULL;

	count = 0;
}

 

template<typename T> void ObjList<T>::RemoveAt( OBJ_POSITION& pos )
{
	if( count == 0 || pos == NULL )
  		return;
  	
	elem<T>* remObj = (elem<T>*) pos;
	
	if( ( remObj->next == NULL ) && ( remObj->prev == NULL) ) //count == 1
	{
		headNode = tailNode = (elem<T>*)NULL;
		pos = NULL;
	}
	else
		if( ( remObj->prev != NULL ) && ( remObj->next != NULL ) )
		{
			remObj->prev->next = remObj->next;
			remObj->next->prev = remObj->prev;
			pos = ( OBJ_POSITION )remObj->next;
		}
		else
			if( remObj->prev == NULL )
			{
				headNode = remObj->next;
				headNode->prev = (elem<T>*)NULL;
				pos = ( OBJ_POSITION )remObj->next;
			}
			else
				if( remObj->next == NULL )
				{
					tailNode = remObj->prev;
					tailNode->next = (elem<T>*)NULL;
					pos = NULL;
				}

    delete remObj;
    remObj = (elem<T>*)NULL;
	count--;
}


template<typename T> int ObjList<T>::GetCount()
{
	return count;
}

template<typename T> OBJ_POSITION ObjList<T>::GetHeadPosition()
{
    OBJ_POSITION pos = (OBJ_POSITION)headNode;
    return pos;
}

template<typename T> OBJ_POSITION ObjList<T>::GetTailPosition()
{
    OBJ_POSITION pos = (OBJ_POSITION)tailNode;
    return pos;
}

template< class T >
T& ObjList<T>::GetNext( OBJ_POSITION& pos )
{
	T& retObj = ((elem<T>*)pos)->obj;

	if( (elem<T>*)pos != tailNode )
		pos = (OBJ_POSITION)(((elem<T>*)pos)->next);
	else
		pos = NULL;

	return retObj;
}

template<typename T>
OBJ_POSITION ObjList<T>::GetNextPosition( OBJ_POSITION& pos )
{
	if( pos )
	{
		if( (elem<T>*)pos != tailNode )
			pos = (OBJ_POSITION)(((elem<T>*)pos)->next);
		else
			pos = NULL;
	}
	return pos;
}


template< class T >
T& ObjList<T>::GetPrev( OBJ_POSITION& pos )
{
	T& retObj = ((elem<T>*)pos)->obj;

	if( (elem<T>*)pos != headNode )
		pos = (OBJ_POSITION)(((elem<T>*)pos)->prev);
	else
		pos = NULL;

	return retObj;
}

template<typename T>
T& ObjList<T>::GetAt( OBJ_POSITION pos )
{
	return ((elem<T>*)pos)->obj;
}

template<typename T> bool ObjList<T>::SetAt( OBJ_POSITION pos,  T& newElem )
{
	if( !pos )
	    return false;
	((elem<T>*)pos)->obj = newElem;
	return true;
}

template<typename T> 
OBJ_POSITION ObjList<T>::InsertAfter( OBJ_POSITION pos,  T& newElem )
{
	if( !pos || !((elem<T>*)pos)->next )
	{	return AddTail(newElem);		
	}

	elem<T>* added = new elem<T>;
	added->prev = ((elem<T>*)pos);
	added->next = ((elem<T>*)pos)->next;
	added->obj = newElem;

	((elem<T>*)pos)->next = added;
	added->next->prev = added;
    count ++;

	return (OBJ_POSITION)added;
}

template<typename T> 
OBJ_POSITION ObjList<T>::InsertBefore( OBJ_POSITION pos,  T& newElem )
{
	if( !pos || !((elem<T>*)pos)->prev )
	{	return AddHead(newElem);
	}

	elem<T>* added = new elem<T>;
	added->prev = ((elem<T>*)pos)->prev;
	added->next = ((elem<T>*)pos);
	added->obj = newElem;

	((elem<T>*)pos)->prev = added;
	added->prev->next = added;
    count ++;

	return (OBJ_POSITION)added;
}

template<typename T> T& ObjList<T>::GetHead()
{
	return headNode->obj;
}

template<typename T> T& ObjList<T>::GetTail()
{
	return tailNode->obj;
}

template<class T> 
void ObjList<T>::RemoveHead()
{
	OBJ_POSITION pos = GetHeadPosition();
	RemoveAt( pos );
}

template<class T> 
void ObjList<T>::RemoveTail()
{
	OBJ_POSITION pos = GetTailPosition();
	RemoveAt( pos );
}

template <class T>
inline OBJ_POSITION FindElement( T& vT, ObjList<T>& lsT )
{
	OBJ_POSITION pos = lsT.GetHeadPosition();
	while(pos)
		if (  vT == lsT.GetAt(pos) )
			return pos;
		else
			lsT.GetNext( pos );
		return NULL;
}


template <class T>
inline OBJ_POSITION FindElement( T& vT, ObjList<T>& lsT, int ( *fcomp ) ( T& , T& ) )
{
	OBJ_POSITION pos = lsT.GetHeadPosition();
	while(pos)
		if ( !fcomp( vT, lsT.GetAt(pos) ))
			return pos;
		else
			lsT.GetNext( pos );
		return NULL;
}



template <class T>
inline OBJ_POSITION FindElement( T& vT, ObjList<T>& lsT, int ( *fcomp ) ( T , T ) )
{
	OBJ_POSITION pos = lsT.GetHeadPosition();
	while(pos)
		if ( !fcomp( vT, lsT.GetAt(pos) ))
			return pos;
		else
			lsT.GetNext( pos );
	return NULL;
}


template <class T, class tObject>
inline OBJ_POSITION FindElementObj( tObject& tobj, ObjList<T>& lsT, int (*fcomp) ( T, tObject ) )
{
	OBJ_POSITION pos = lsT.GetHeadPosition();
	while(pos)
		if ( !fcomp( lsT.GetAt(pos), tobj ))
			return pos;
		else
			lsT.GetNext( pos );
		
		return NULL;
}

template< class T> 
void ObjList<T>::AddGenericHead()
{
	elem<T>* paddedHead = new elem<T>;
		
	if( count == 0 )
	{
		tailNode = headNode = paddedHead;

		headNode->prev = tailNode->next = (elem<T>*)NULL;
	}
	else
	{
		paddedHead->next = headNode;
		paddedHead->prev = (elem<T>*)NULL;

		headNode->prev = paddedHead;
		headNode = paddedHead;
	}

	count++;
}

template< class T > 
void ObjList<T>::AddGenericTail()
{
	if( count != 0 )
	{
		elem< T >* paddedTail = new elem< T >;

		tailNode->next = paddedTail;
		paddedTail->prev = tailNode;

		tailNode = paddedTail;

		count ++;
	}
	else
	{
		AddGenericHead();
	}
}

template<class T> 
void ObjList<T>::RemoveElement( T& tElem )
{
	OBJ_POSITION posRemove = FindElement<T>( tElem, *this );
	if ( posRemove )
		RemoveAt( posRemove );
}

template<class T> 
void ObjList<T>::RemoveElement( T& tElem, int (*fcomp) ( T&, T& ) )
{
	OBJ_POSITION posRemove = FindElement<T>( tElem, *this, fcomp );
	if ( posRemove )
		RemoveAt( posRemove );
}

template<class T, class tObject> 
void RemoveElementObj( tObject& tobj, ObjList<T>& lsT, int (*fcomp) ( T&, tObject& ) )
{
	OBJ_POSITION posRemove = FindElement<T>( tobj, lsT, fcomp );
	if ( posRemove )
		lsT.RemoveAt( posRemove );
}

#if defined( __AFXTEMPL_H__ ) 

	template <class T>
	inline POSITION FindElement( T& vT, CList<T, T&>& lsT )
	{
		POSITION pos = lsT.GetHeadPosition();
		while(pos)
			if (  vT == lsT.GetAt(pos) )
				return pos;
			else
				lsT.GetNext( pos );
			return NULL;
	}

	template <class T, class tObject>
	inline POSITION FindElementObj( tObject& tobj, CList<T, T&>& lsT, int (*fcomp) ( T&, tObject& ) )
	{
		POSITION pos = lsT.GetHeadPosition();
		while(pos)
			if ( !fcomp( lsT.GetAt(pos), tobj ))
				return pos;
			else
				lsT.GetNext( pos );

		return NULL;
	}

	template <class T>
	inline POSITION FindElement( T& vT, CList<T, T&>& lsT, int ( *fcomp ) ( T& , T& ) )
	{
		POSITION pos = lsT.GetHeadPosition();
		while(pos)
			if ( !fcomp( vT, lsT.GetAt(pos) ))
				return pos;
			else
				lsT.GetNext( pos );

		return NULL;
	}
	
	template<class T> 
		void RemoveElement( T& tElem, CList<T, T&>& lsT )
	{
		POSITION posRemove = FindElement<T>( tElem, lsT );
		if ( posRemove )

			lsT.RemoveAt( posRemove );
	}
	
	template<class T> 
		void RemoveElement( T& tElem, CList<T, T&>& lsT, int ( *fcomp ) ( T& , T&  ) )
	{
		POSITION posRemove = FindElement<T>( tElem, lsT, fcomp );
		if ( posRemove )
			lsT.RemoveAt( posRemove );
	}
	
	template <class T, class tObject>
		void RemoveElementObj( tObject& tobj, CList<T, T&>& lsT, int (*fcomp) ( T&, tObject& ) )
	{
		POSITION posRemove = FindElementObj<T>( tobj, lsT, fcomp );
		if ( posRemove )
			lsT.RemoveAt( posRemove );
	}

#endif //defined( __AFXTEMPL_H__ ) 

/// @}
#endif // !defined(AFX_OBJLIST_H__137C1D9B_EAD6_4B7B_A21D_88C69E91EE4F__INCLUDED_)

