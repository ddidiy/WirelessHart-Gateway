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

/////////////////////////////////////////////////////////////////////////////
/// @file       Cookie.cpp
/// @author     Marius Negreanu
/// @date       Tue Sep  2 16:45:54 EEST 2008
/// @brief      Cookie parser/getter.
/// $Id: Cookie.cpp,v 1.10.18.1 2013/05/15 19:19:17 owlman Exp $
/////////////////////////////////////////////////////////////////////////////


#include "Cookie.h"



#include <stdlib.h>
#include "../Shared/Common.h"

CCookie::~CCookie()
{
	for ( int i=0; m_szCookies && m_szCookies[i];++i )
	{
		free(m_szCookies[i]->name );
		free(m_szCookies[i]->value );
		free(m_szCookies[i]);
	}

	free( m_szCookies );
}


bool CCookie::ReadCookies( const char *cookie_string)
{
	const char *http_cookie, *curpos ;
	char *n0, *n1, *v0, *v1, *cp;
	CCookie::Value *pivot = NULL;
	int count;
	int len;

	if ((curpos = http_cookie = cookie_string) == NULL)
	{
		LOG("%s: HTTP_COOKIE NULL", __FUNCTION__);
		return false;
	}
	count = 0;
	if ((m_szCookies = (CCookie::Value **)malloc (sizeof (CCookie::Value *))) == NULL)
	{
		LOG("Malloc problem");
		return false;
	}
	m_szCookies[0] = NULL;

	while (*curpos)
	{
		for (n0=(char*)curpos; *n0 && *n0==' '; n0++);
		for (n1=n0; *n1 && *n1!=' ' && *n1!='=' && *n1!=';' && *n1!=',';n1++);
		for (v0=n1; *v0 && (*v0==' ' || *v0=='=' || *v0==' ');v0++);
		if (*v0 == '"')
		{
			v1=++v0;
			for (;*v1 && *v1!='"';v1++);
		}
		else
		{
			v1=v0;
			for (;*v1 && *v1!=',' && *v1!=';';v1++);
		}

		if (n0 != n1)
		{
			if (*n0 == '$')
			{
				if (strncasecmp (n0, "$version", 8) && strncasecmp (n0, "$path", 5)
				                && strncasecmp (n0, "$domain", 7))
				{
					curpos = v1+1;
					continue;
				}
			}
			else
				if (pivot && pivot->name)
				{
					count++;
					if ((m_szCookies = (CCookie::Value **)realloc (m_szCookies, sizeof (CCookie::Value *) * (count+1))) == NULL)
					{
						LOG("Realloc problem");
						return false ;
					}
					m_szCookies[count-1] = pivot;
					m_szCookies[count] = pivot = NULL;
				}

			if (!pivot)
			{
				if ((pivot = (CCookie::Value *)malloc (sizeof (CCookie::Value))) == NULL)
					return true;
				memset (pivot, 0, sizeof (CCookie::Value));
				if (m_szCookies && m_szCookies[0] && m_szCookies[0]->version)
					pivot->version = m_szCookies[0]->version;	/* Warning: *Never* free() m_szCookies[n!=0]->version */
			}
			if (*n0 == '$')
			{
				n0++;
				len = sizeof (char) * (v1-v0);
				if ((cp = (char *)malloc (len)) == NULL)
					return true;
				memset (cp, 0, len);
				strncpy (cp, v0, v1-v0);

				if (!strncasecmp (n0, "version", 7))
					pivot->version = cp;
				else if (!strncasecmp (n0, "path", 4))
					pivot->path = cp;
				else if (!strncasecmp (n0, "domain", 6))
					pivot->domain = cp;
				else
					free (cp);
			}
			else
			{
				len = sizeof (char) * (n1-n0+1);
				if ((pivot->name = (char *)malloc (len)) == NULL)
					return true ;
				memset (pivot->name, 0, len);
				strncpy (pivot->name, n0, n1-n0);

				if (*v0 == '"')
					v0++;
				len = sizeof (char) * (v1-v0+1);
				// Ugly fix for next release
				if (! len )
				{
					if ((pivot->value = (char *)malloc (1)) == NULL)
						return true ;
					memset (pivot->value, 0, 1);
					pivot->value[0] = 0 ;
				}
				else if ( len > 0 )
				{
					if ((pivot->value = (char *)malloc (len)) == NULL)
						return true ;
					memset (pivot->value, 0, len);
					strncpy (pivot->value, v0, len-1);
				}
			}
		}
		else
		{
			curpos = n0+1;
		}
		curpos = v1;
		if (*curpos) curpos++;
	}

	count++;
	if ((m_szCookies = (CCookie::Value **)realloc (m_szCookies, sizeof (CCookie::Value *) * (count+1))) == NULL)
	{
		LOG("Realloc problem 2");
		return false;
	}
	m_szCookies[count-1] = pivot;
	m_szCookies[count] = false;

	return true ;
}

CCookie::Value *CCookie::GetCookie (const char *name)
{
	if (!m_szCookies )
		return NULL;

	for (int i=0; m_szCookies[i]; i++)
		if ( m_szCookies[i]->name && m_szCookies[i]->value && !strcmp(name,m_szCookies[i]->name))
		{
			return m_szCookies[i];
		}
	return NULL;
}

char **CCookie::GetCookies()
{
	int i, k;
	char **res = NULL;
	int len;

	if (!m_szCookies)
		return NULL;
	for (i=k=0;m_szCookies[i];i++)
		if (m_szCookies[i]->name && m_szCookies[i]->value)
			k++;
	len = sizeof (char *) * ++k;
	if ((res = (char **)malloc (len)) == NULL)
		return NULL;
	memset (res, 0, len);
	for (i=k=0;m_szCookies[i]; i++)
	{
		if (m_szCookies[i]->name && m_szCookies[i]->value)
		{
			len = strlen (m_szCookies[i]->name) +1;
			if ((res[i] = (char *)malloc (len)) == NULL)
				return NULL;
			memset (res[i], 0, len);
			strcpy (res[i], m_szCookies[i]->name);
		}
	}
	return res;
}
