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

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <cctype>
#include <cerrno>
#include <algorithm>

#include "../Shared/Common.h"
#include "../Shared/Utils.h"
#include "Cgi.h"
#include "../Shared/h.h"

#include <sys/socket.h>
#include <netinet/in.h>


#define BUFSIZE 128
#define NULLFREE(a) if (a) { free(a); a=NULL;}


static ssize_t peek(int fd, void *buf, size_t count)
{
	int  n=0;

	sockaddr_in cliaddr ;
	socklen_t len = sizeof(cliaddr );

	n = recvfrom( fd, buf, count, MSG_DONTWAIT|MSG_TRUNC|MSG_PEEK, (struct sockaddr *)&cliaddr, &len);

	if ( -1 == n )
	{
		if ( errno == EAGAIN )
		{
			return 0 ;
		}
		FPERR("peek");
	}
	else if ( 0 == n )
	{
		if ( errno==EINTR)
		{
			FPWARN("peek");
		}
		sleep(1) ; // wait a while
	}
	return n ;
}




int fputc_s(int c, FILE *stream)
{
	int rv = fputc (c, stream) ;
	if ( EOF == rv )
	{
		ERR("Unable to fputc(%02X) in tmpfile: %s", c, strerror(errno));
		log2flash("CGI: ERROR: Unable to fputc(%02X) in tmpfile: %s", c, strerror(errno));
	}
	return rv ;
}

int fclose_s(FILE *fp)
{
	int rv = fclose(fp) ;
	if ( EOF == rv )
	{
		LOG_ERR("fclose failed");
		log2flash("CGI: fclose failed:[%s]", strerror(errno) ) ;
	}
	return rv ;
}

int fcopy_s(FILE* dest, int src, size_t size, int&contentLength )
{
	char rdBuf[8*1024];
	int rb = read( src, rdBuf, size );
	if ( rb <= 0 )
	{
		LOG_ERR("smth went wrong again2");
		return -1 ;
	}
	for ( int wb=0; wb < rb; )
	{
		wb += fwrite( rdBuf+wb, sizeof(char), rb-wb, dest);
	}
	contentLength -= rb ;
	if ( contentLength <= 0 )
	{
		LOG_HEX("BUFFER", (const unsigned char*)rdBuf, rb );
	}
	return rb ;
}

char *EscapeSgml (char *string)
{
	char *cp, *np;
	char *buf;
	size_t len;

	for (cp=string, len=0; *cp; cp++)
	{
		switch (*cp)
		{
		case '&':
			len+=5;
			break;
		case '<':
		case '>':
			len+=4;
			break;
		default:
			len++;
			break;
		}
	}

	if (len == strlen(string))
	{
		return strdup(string);
	}

	if ((buf = (char *)malloc(len+1)) == NULL)
	{
		return NULL;
	}

	for (cp=string, np=buf; *cp; cp++)
	{
		switch (*cp)
		{
		case '&':
			*np++ = '&';
			*np++ = 'a';
			*np++ = 'm';
			*np++ = 'p';
			*np++ = ';';
			break;
		case '<':
			*np++ = '&';
			*np++ = 'l';
			*np++ = 't';
			*np++ = ';';
			break;
		case '>':
			*np++ = '&';
			*np++ = 'g';
			*np++ = 't';
			*np++ = ';';
			break;
		default:
			*np++ = *cp;
			break;
		}
	}
	*np = '\0';

	return buf;
}
char *UnescapeSgml (char *text)
{
	char *cp, *xp;

	for (cp=text,xp=text; *cp; cp++)
	{
		if (*cp == '%')
		{
			if ( isxdigit(*(cp+1)) && isxdigit(*(cp+2)) )
			{
				if (islower(*(cp+1)))
					*(cp+1) = toupper(*(cp+1));
				if (islower(*(cp+2)))
					*(cp+2) = toupper(*(cp+2));
				*(xp) = (*(cp+1) >= 'A' ? *(cp+1) - 'A' + 10 : *(cp+1) - '0' ) * 16
				        + (*(cp+2) >= 'A' ? *(cp+2) - 'A' + 10 : *(cp+2) - '0');
				xp++;
				cp+=2;
			}
		}
		else
		{
			*(xp++) = *cp;
		}
	}
	memset(xp, 0, cp-xp);
	return text;
}

/////////////////////////////////////////////////////////////////
// public members
/////////////////////////////////////////////////////////////////
void CGI::FreeList (char **list)
{
	int i;

	for (i=0; list[i] != NULL; i++)
		free (list[i]);
	free (list);
}


/*  cgiInit()
 *
 *  Read from io.input if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
bool CGI::Init( )
{
	//if ( res && !res->vars && !res->files)
	if ( ! ReadVariables () )
	{
		return false;
	}
	return true;
}


void CGI::Free ()
{
	int i;

	if (vars)
	{
		for (i=0;vars[i]; i++)
		{
			if (vars[i]->name)
				free (vars[i]->name);
			if (vars[i]->value)
				free (vars[i]->value);
			free (vars[i]);
		}
		free (vars);
	}

	if (files)
	{
		for (i=0;files[i]; i++)
		{
			if (files[i]->name)
				free (files[i]->name);
			if (files[i]->type)
				free (files[i]->type);
			if (files[i]->filename)
				free (files[i]->filename);
			if (files[i]->tmpfile)
			{
				unlink (files[i]->tmpfile);
				free (files[i]->tmpfile);
			}
			free (files[i]);
		}
		free (files);
	}
}






char *CGI::GetValue ( const char *name)
{
	int i;

	if ( !vars)
	{
		return NULL;
	}
	for (i=0;vars[i]; i++)
		if (!strcmp(name,vars[i]->name))
		{
			if (strlen(vars[i]->value) > 0)
			{
				return vars[i]->value;
			}
			else
			{
				return NULL;
			}
		}
	return NULL;
}

char **CGI::GetVariables ()
{
	int i;
	char **res = NULL;
	int len;

	if (!vars)
		return NULL;

	for (i=0;vars[i]; i++);
	len = sizeof (char *) * ++i;
	if ((res = (char **)malloc (len)) == NULL)
	{
		return NULL;
	}
	memset (res, 0, len);
	for (i=0;vars[i]; i++)
	{
		len = strlen (vars[i]->name) +1;
		if ((res[i] = (char *)malloc (len)) == NULL)
		{
			return NULL;
		}
		memset (res[i], 0, len);
		strcpy (res[i], vars[i]->name);
	}
	return res;
}



/* cgiGetFiles
 *
 * Returns a list of names of all files.
 */

char **CGI::GetFiles ()
{
	int i;
	char **res = NULL;
	int len;

	if (! files)
		return NULL;

	for (i=0;files[i]; i++);

	len = sizeof (char *) * ++i;
	if ((res = (char **)malloc (len)) == NULL)
		return NULL;
	memset (res, 0, len);
	for (i=0;files[i]; i++)
	{
		len = strlen (files[i]->name) +1;
		if ((res[i] = (char *)malloc (len)) == NULL)
			return NULL;
		memset (res[i], 0, len);
		strcpy (res[i], files[i]->name);
	}
	return res;
}


/* cgiGetFile
 *
 * Return data structure for CGI file variable
 */
s_file *CGI::GetFile (const char *name)
{
	int i;

	if (! files)
		return NULL;

	for (i=0;files[i]; i++)
		if (!strcmp(name,files[i]->name))
		{
			return files[i];
		}
	return NULL;
}


int searchodoa(const char* rdBuf, size_t size)
{
	const char *orig=rdBuf ;
	while ( size > 0 )
	{
		const char *c = (const char*)memchr(rdBuf, 0x0D, size) ;
		if ( ! c ) return -1 ;
		if ( c && *(c+1)==0x0A ) return (c-orig) ;
		else
		{
			size -= (c-rdBuf)-1 ;
			rdBuf = c+1 ;
		}
	}
	return -1 ;
}


/////////////////////////////////////////////////////////////////
// protected members
/////////////////////////////////////////////////////////////////
/* cgiReadFile()
 *
 * Read and save a file from a multipart request
 * another case to look for, when 0x0D is seen as the last
 * character in the buffer, then the whole buffer should be
 * commited, and retry the search
 */
char *CGI::ReadFile (char *boundary, int& contentLength)
{
	size_t boundaryLen;
	char tmpl[]= NIVIS_TMP"cgi_file_upload_XXXXXX";
	FILE *tmpfile;
	int fd;

	boundaryLen = strlen(boundary);
	FLOG("BOUNDARY[%s]", boundary);

	int startContentLength = contentLength ;

	if ( -1 == (fd = mkstemp (tmpl)) )
	{
		log2flash("CGI: cgiReadFile failed: Unable to mkstemp");
		return NULL;
	}

	if ((tmpfile = fdopen (fd, "w")) == NULL)
	{
		unlink (tmpl);
		log2flash("CGI: cgiReadFile failed: Unable to fdopen tmpfile");
		return NULL;
	}

	FLOG("contentLength[%d]", contentLength);
	log2flash("CGI: cgiReadFile expecting %u bytes", contentLength);
	while (contentLength > 0 && !feof (io.input))
	{
		size_t boundaryOffset =0 ;
		char rdBuf[8*1024];
		struct timeval timeout={5,0};
		fd_set readfds, errfds;
		FD_ZERO(&readfds);
		FD_ZERO(&errfds);

		FD_SET( fileno(io.input), &readfds );
		FD_SET( fileno(io.input), &errfds );
		int rv=select(fileno(io.input)+1, &readfds, NULL, &errfds, &timeout);

		if ( -1 == rv || FD_ISSET(fileno(io.input), &errfds) )
		{
			FPERR("select failed");
			break ;
		}

		if ( 0== rv || ! FD_ISSET( fileno(io.input), &readfds) )
		{
			LOG("FILE is not readable");
			continue;
		}

		ssize_t ssize = peek( fileno(io.input), rdBuf, sizeof(rdBuf) );
		if ( ssize <= 0 ) break ;
		size_t size = (size_t)ssize;

		LOG("Size:%i contentLength:%i", size, contentLength);

		if ( boundaryOffset )
		{
			if ( ! strncmp( rdBuf, boundary+boundaryOffset, _Min( strlen(boundary)-boundaryOffset, size)) )
			{
				boundaryOffset+=_Min( strlen(boundary)-boundaryOffset, size);
				if ( boundaryOffset==boundaryLen)
				{
					LOG("Rest of boundary found[%s] -> exit", rdBuf );
					boundaryOffset = 0 ;
					break;
				}
				else
				{
					LOG("Still looking for boundary");
					continue ;
				}
			}
			else
			{
				LOG("Rest of partial match not found -> commit all data. (rb:%i contentLength:%i)", size, contentLength);
				fcopy_s(tmpfile, fileno(io.input), size, contentLength);
				boundaryOffset = 0 ;
			}
		}



		int boundaryStart = searchodoa(rdBuf, size);
		if ( boundaryStart <0 )
		{
			fcopy_s(tmpfile, fileno(io.input), size, contentLength);
			LOG("No 0x0D/0x0A seen -> commit all data (rb:%i contentLength:%i)", size, contentLength );
			continue;
		}
		else
		{
			if ( ! strncmp( rdBuf+boundaryStart+2, boundary, _Min(size,boundaryLen)) )
			{
				// read from socket until boundary begins
				// write the read data to disk so we can use
				// rdBuf to reconstruct the boundary
				boundaryOffset = _Min(size,boundaryLen)  ;
				if ( boundaryOffset >= boundaryLen )
				{
					size_t plus=0, finalsz=size;
					if ( ! strncmp("--\x0d\x0a",rdBuf+boundaryStart+2+boundaryLen,4) )
					{
						LOG("Final boundary found -> commit all data and exit (contentLength:%i)", contentLength );
						plus=4;
						finalsz=sizeof(rdBuf);
					}
					ssize_t rb = read( fileno(io.input), rdBuf, finalsz );
					if ( rb <=0 )
					{
						LOG_ERR("Smth went wrong");
						break ;
					}
					for ( size_t wb=0; wb < rb-boundaryLen-2-plus/*0d0a*/; )
					{
						wb += fwrite( rdBuf+wb, sizeof(char), (rb-boundaryLen-2-plus)-wb, tmpfile);
					}
					contentLength -= rb ;
					LOG("Boundary found comit data until boundaryStart (rb:%i contentLength:%i)", rb, contentLength);
					break ;
				}
				else
				{
					// needs testing, hard to hit branch
					LOG("Partial match -> need more data. comit data before boundaryStart (contentLength:%i)", contentLength );
					//consume until boundary begins
					fcopy_s( tmpfile, fileno(io.input), boundaryStart-1, contentLength) ;
					continue ;
				}
			}
			else
			{
				fcopy_s(tmpfile, fileno(io.input), size, contentLength);
				LOG("Reached end of buffer-> commit all data. (rb:%i contentLength:%i)", size, contentLength);
				continue;
			}
		}
	}

	fclose_s (tmpfile);

	FLOG("cgiReadFile[%u]B tmpFile:[%s] contentLengthLeft[%d]B", startContentLength-contentLength, tmpl, contentLength);
	//log2flash("CGI: cgiReadFile[%u]B tmpFile[%s] contentLengthLeft[%d]", startContentLength-contentLength, tmpl, contentLength);
	if (contentLength)
	{
		ERR("cgiReadFile incomplete dl -> unlink(tmpFile:[%s])", tmpl);
		log2flash("ERR:cgiReadFile incomplete dl -> unlink(tmpFile:[%s])", tmpl);
		unlink(tmpl);
		return NULL;
	}

	return strdup (tmpl);
}


static int readable(FILE*in, time_t timeout)
{
	struct timeval tv ={timeout, 0} ;
	fd_set ifds,efds;
	if (NULL==in) return false ;
	FD_ZERO(&ifds);
	FD_ZERO(&efds);
	FD_SET(fileno(in),&ifds);
	FD_SET(fileno(in),&efds);
	if ( -1 == select(fileno(in)+1,&ifds,NULL,&efds,&tv ) )
	{
		LOG("select failed");
		return false ;
	}
	if (FD_ISSET(fileno(in),&ifds) )return true ;
	return false ;
}
/* cgiReadMultipart()
 *
 * Decode multipart/form-data
 */
#define MULTIPART_DELTA 5
bool CGI::ReadMultipart (char *boundary, int contentLength)
{
	char *line(0),*cp(0), *xp(0), *formName(0), *contentType(0);
	char *fileName(0), *tmpFile(0) ;
	bool inHeader = false ;
	s_var **result = NULL;
	s_var **tmp;
	int numresults = 0, current = 0;
	s_file **tmp_files = NULL;
	s_file **tmpf;
	s_file *file;
	int index = 0;
	size_t len;
	size_t lineSz(0), rb(0) ;

	FLOG("BOUNDARY[%s]", boundary);
	if ( ! readable(io.input,5) ) return false;

	while ( true )
	{
		if ( contentLength<=0 || feof(io.input) ) break ;
		if ( ! readable(io.input,5) )
		{
			vars = NULL;
			files = NULL ;
			free (formName);
			NULLFREE (contentType) ;
			return false;
		}
		if( (rb=getline( &line, &lineSz, io.input)) <= 0 || ferror(io.input) ) break;
		if ( rb <0 )
		{
			ERR("getline failed");
			break ;
		}
		FLOG("RB:%i LINE:[%s]", rb, line);
		contentLength-=rb ;

		if (!strncmp (line, boundary, strlen(boundary)))
		{
			inHeader = true ;
		}
		else if (!strncasecmp (line, "Content-Disposition: form-data; ", 32))
		{
			if (!formName)
			{
				if ( NULL == (cp = strstr (line, "name=\"")) )
					continue;
				cp += 6;

				if ( NULL == (xp = strchr (cp, '\"')) )
					continue;

				formName = strndup (cp, xp-cp);
				UnescapeSgml (formName);

				if ( NULL ==(cp = strstr (line, "filename=\"")) )
					continue;

				cp += 10;
				if ( NULL == (xp = strchr (cp, '\"'))  )
					continue;

				fileName = strndup (cp, xp-cp);
				UnescapeSgml (fileName);
			}
		}
		else if (!strncasecmp (line, "Content-Type: ", 14))
		{
			if (!contentType)
			{
				cp = line + 14;
				contentType = strdup (cp);
			}
		}
		else if (inHeader)
		{
			if (line[0] !=0x0D || line[1]!= 0x0A) continue ;

			inHeader = false ;
			if ( !fileName ) continue ;
			inHeader = true ;
			tmpFile = ReadFile (boundary, contentLength);

			if (!tmpFile)
			{
				free (formName);
				free (fileName);
				NULLFREE (contentType) ;
				formName = fileName = contentType = NULL;
				continue ;
			}

			if (!strlen (fileName))
			{
				unlink (tmpFile);
				free (tmpFile);
				free (formName);
				free (fileName);
				NULLFREE (contentType) ;
				formName = fileName = contentType = NULL;
				continue ;
			}
			if ( NULL == (file = (s_file *)malloc (sizeof (s_file))) )
			{
				unlink (tmpFile);
				free (tmpFile);
				free (formName);
				free (fileName);
				NULLFREE (contentType) ;
				formName = fileName = contentType = NULL;
				continue;
			}

			if ( NULL == (cp = rindex (fileName, '/'))
			                &&   NULL == (cp = rindex (fileName, '\\'))
			   )
				file->filename = fileName;
			else
			{
				file->filename = strdup (++cp);
				free (fileName);
			}
			file->name	= formName;
			file->type	= contentType;
			file->tmpfile	= tmpFile;
			formName = contentType = fileName = NULL;

			if (!tmp_files)
			{
				if ((tmp_files = (s_file **)malloc(2*sizeof (s_file *))) == NULL)
				{
					unlink (tmpFile);
					free (tmpFile);
					free (formName);
					formName = NULL;
					NULLFREE(contentType);
					free (file->filename);
					free (file);
					continue;
				}
				memset (tmp_files, 0, 2*sizeof (s_file *));
				index = 0;
			}
			else
			{
				for (index=0; tmp_files[index]; index++);
				if ((tmpf = (s_file **)realloc(tmp_files, (index+2)*sizeof (s_file *))) == NULL)
				{
					unlink (tmpFile);
					free (tmpFile);
					free (formName);
					NULLFREE(contentType);
					free (file->filename);
					free (file);
					formName = contentType = fileName = NULL;
					continue;
				}
				tmp_files = tmpf;
				memset (tmp_files + index, 0, 2*sizeof (s_file *));
			}
			tmp_files[index] = file;
		}
		else // inHeader==false
		{
			if (formName)
			{

				/* try to find out if there's already such a variable */
				for (index=0; index<current && strcmp (result[index]->name,formName); index++);

				if (index == current)
				{
					if (!result)
					{
						len = MULTIPART_DELTA * sizeof (s_var *);
						if ((result = (s_var **)malloc (len)) == NULL)
						{
							free (formName);
							NULLFREE (contentType) ;
							return false;
						}
						numresults = MULTIPART_DELTA;
						memset (result, 0, len);
						current = 0;
					}
					else
					{
						if (current+2 > numresults)
						{
							len = (numresults + MULTIPART_DELTA) * sizeof(s_var *);
							if ((tmp = (s_var **)realloc (result, len)) == NULL)
							{
								for (index=0; result[index]; index++)
									free (result[index]);
								free (result);
								free (formName);
								NULLFREE (contentType) ;
								return false;
							}
							result = tmp;
							memset (result + numresults, 0, len - numresults*sizeof(s_var *));
							numresults += MULTIPART_DELTA;
						}
					}
					if ((result[current] = (s_var *)malloc(sizeof(s_var))) == NULL)
					{
						for (index=0; result[index]; index++)
							free (result[index]);
						free (result);
						free (formName);
						NULLFREE (contentType) ;
						return false;
					}
					current++;
					result[index]->name = formName;
					formName = NULL;
					result[index]->value = strdup (line);
					UnescapeSgml (result[index]->value);
					NULLFREE (contentType) ;
				}
				else
				{
					free (formName);
					formName = NULL;
					if ((cp = (char *)realloc (result[index]->value, strlen(result[index]->value)+strlen(line)+2)) != NULL)
					{
						strcat(cp, "\n");
						strcat(cp, line);
						result[index]->value = cp;
						NULLFREE (contentType) ;
					}
				}
			}
			else
			{
				if (index > 0)
				{
					xp = strdup (line);
					UnescapeSgml (xp);

					if ((formName = (char *)malloc (strlen(result[index]->value)+strlen(xp)+3)) == NULL)
					{
						for (index=0; result[index]; index++)
							free (result[index]);
						free (result);
						free (xp);
						return false;
					}
					sprintf (formName, "%s\r\n%s", result[index]->value, xp);
					free (result[index]->value);
					result[index]->value = formName;
					formName = NULL;
					free (xp);
				}
			}
		}
	}
	if ( line ) free ( line );

	vars = result;
	files = tmp_files;
	return true;
}
/*  cgiReadVariables()
 *
 *  Read from io.input if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
bool CGI::ReadVariables ( )
{
	char *line = NULL;
	int numargs;
	char *cp,*sptr;
	const char*esp ;
	s_var **result;
	int i, k, len;


	cp = (char*)io.env.Get("CONTENT_TYPE");
	if (cp && strstr(cp, "multipart/form-data") && strstr(cp, "boundary="))
	{
		cp = strstr(cp, "boundary=") + strlen ("boundary=") - 2;
		*cp = *(cp+1) = '-';
		return ReadMultipart ( cp, io.env.ContentLength() );
	}

	/*if ((res = (s_cgi *)malloc (sizeof (s_cgi))) == NULL)
	{
		return NULL;
	}*/

	if ( io.env.RequestMethod()==REQ_MET_POST )
	{
		if ( !io.env.ContentLength()
		                ||  NULL == (line=(char*)malloc(io.env.ContentLength()+2)) )
			return false;
		if (fgets( line, io.env.ContentLength() +1, io.input) == NULL)
			return false;
	}
	else if ( io.env.RequestMethod() == REQ_MET_GET )
	{
		esp = io.env.Get("QUERY_STRING");
		if (esp && strlen(esp))
		{
			if ((line = (char *)malloc (strlen(esp)+2)) == NULL)
				return false;
			sprintf (line, "%s", esp);
		}
		else
		{
			return false;
		}
	}
	else
	{
		printf ("Content-Type: text/html\r\n\r\nSome text\r\n");
		return false;
	}

	/*
	 *  From now on all cgi variables are stored in the variable line
	 *  and look like  foo=bar&foobar=barfoo&foofoo=
	 */


	for (cp=line; *cp; cp++)
		if (*cp == '+')
			*cp = ' ';

	if (strlen(line))
	{
		for (numargs=1,cp=line; *cp; cp++)
			if (*cp == '&' || *cp == ';' ) numargs++;
	}
	else
		numargs = 0;

	len = (numargs+1) * sizeof(s_var *);
	if ((result = (s_var **)malloc (len)) == NULL)
		return false;
	memset (result, 0, len);

	cp = line;
	i=0;
	while (*cp)
	{
		char *ip=NULL ;
		if ((ip = (char *)strchr(cp, '&')) != NULL)
		{
			*ip = '\0';
		}
		else if ((ip = (char *)strchr(cp, ';')) != NULL)
		{
			*ip = '\0';
		}
		else
			ip = cp + strlen(cp);

		if ((esp=(char *)strchr(cp, '=')) == NULL)
		{
			cp = ++ip;
			continue;
		}

		if (!strlen(esp))
		{
			cp = ++ip;
			continue;
		}

		if (i<numargs)
		{
			char *name;
			char *value;

			if ((name = (char *)malloc((esp-cp+1) * sizeof (char))) == NULL)
				return false;
			strncpy(name, cp, esp-cp);
			name[esp-cp] = '\0';
			UnescapeSgml (name);

			cp = (char*)++esp;

			if ((value = (char *)malloc((ip-esp+1) * sizeof (char))) == NULL)
			{
				free (name);
				return false;
			}
			strncpy(value, cp, ip-esp);
			value[ip-esp] = '\0';
			UnescapeSgml (value);

			/* try to find out if there's already such a variable */
			for (k=0; k<i && strcmp (result[k]->name, name); k++);

			if (k == i)  	/* No such variable yet */
			{
				if ((result[i] = (s_var *)malloc(sizeof(s_var))) == NULL)
					return false;
				result[i]->name = name;
				result[i]->value = value;
				i++;
			}
			else  	/* There is already such a name, suppose a mutiple field */
			{
				free (name);
				len = (strlen(result[k]->value)+strlen(value)+2) * sizeof (char);
				if ((sptr = (char *)malloc(len)) == NULL)
				{
					free (value);
					return false;
				}
				memset (sptr, 0, len);
				sprintf (sptr, "%s\n%s", result[k]->value, value);
				free (result[k]->value);
				free (value);
				result[k]->value = sptr;
			}
		}
		cp = ++ip;
	}

	vars = result;
	files = NULL;

	return true;
}
