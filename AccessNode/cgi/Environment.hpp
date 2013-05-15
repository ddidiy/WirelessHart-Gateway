#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_
/////////////////////////////////////////////////////////////////////////////
/// @file       Environment.hpp
/// @author     Marius Negreanu
/// @date       Tue Sep  2 16:45:54 EEST 2008
/// @brief      Import/convert Environment variables.
/// $Id: Environment.hpp,v 1.14 2011/03/02 09:12:00 marcel Exp $
/////////////////////////////////////////////////////////////////////////////


enum REQUEST_METHOD {
	REQ_MET_POST,
	REQ_MET_GET,
	REQ_MET_UNKN,
	REQ_MET_UNSET
} ;

struct ContentType {
	enum {
		Unknown=1
	};
	struct Multipart {
		enum {
			FormData=2
		};
	};
	struct Text {
		enum {
			Plain=3
		};
	} ;
} ;

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

#include <map>


/////////////////////////////////////////////////////////////////////////////
/// @class CEnvironment Import/convert Environment variables..
/////////////////////////////////////////////////////////////////////////////
class CEnvironment {
public:
	CEnvironment()
	{}
	~CEnvironment()
	{
		std::map<const char*, const char*, ltstr>::iterator it ;
		for (it=m_oEnvMap.begin();it!=m_oEnvMap.end(); ++it)
		{
			free( (void*)(it->first) );
			free( (void*)(it->second) );
		}
		m_oEnvMap.clear() ;
	}
public:

	void Put( const char* varName, const char* varValue )
	{
		const char* name  = strdup(varName) ;
		const char* value = strdup(varValue) ;

		//FLOG("[%s]=[%s]", name, value );
		m_oEnvMap[name] = value ;
	}
	const char* Get(const char* varName)
	{
		if ( m_oEnvMap.find(varName) == m_oEnvMap.end()) return NULL ;
		const char* varValue = m_oEnvMap[varName];
		return varValue ;
	}
	int ContentType(void)
	{
		int eContentType ;
		const char* met= Get("CONTENT_TYPE");
		if ( !met )
			return ContentType::Unknown ;

		if ( strstr(met,"multipart/form-data") )
			eContentType = ContentType::Multipart::FormData ;

		if ( strstr(met,"text/plain") )
			eContentType = ContentType::Text::Plain ;

		return eContentType ;
	}

	REQUEST_METHOD RequestMethod(void)
	{
		REQUEST_METHOD eReqMet=REQ_MET_UNKN;

		const char* met= Get("REQUEST_METHOD");

		if ( !met )
			return eReqMet ;

		if ( !strcmp(met,"POST") )
			eReqMet = REQ_MET_POST ;
		else if ( !strcmp(met,"GET") )
			eReqMet = REQ_MET_GET ;

		return eReqMet ;
	}

	ssize_t ContentLength(void)
	{
		ssize_t nCLen ;
		const char *len = Get("CONTENT_LENGTH");
		char *endptr;
		if ( !len )
			return (nCLen=0);
		errno = 0;
		nCLen = strtol(len, &endptr, 10);
		if ( (errno == ERANGE && (nCLen == LONG_MAX || nCLen == LONG_MIN))
				|| (errno != 0 && nCLen == 0)
				|| endptr == len)
		{
			return (nCLen=0) ;
		}
		return nCLen ;
	}
	inline const char* HttpCookie(void)
	{
		const char* szHttpCookie = Get("HTTP_COOKIE");
		//FLOG("[%s]", szHttpCookie );
		return szHttpCookie ;
	}
private:
	std::map<const char*, const char*,ltstr > m_oEnvMap ;
} ;

#endif
