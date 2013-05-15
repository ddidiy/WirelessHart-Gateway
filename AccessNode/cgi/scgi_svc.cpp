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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>	//getprotoent
#include <unistd.h>	//read
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>	//F_GETFL
#include <sys/select.h>	//fd_set,select,FD_ZERO
#include <sys/un.h>	// AF_UNIX sockadd_un
#include <sys/wait.h>
#include <pthread.h>
#include <cctype>	//isspace

#include "../Shared/Common.h"
#include "../Shared/Utils.h"
#include "../Shared/h.h"
#include "../Shared/FileLock.h"
#include "../Shared/app.h" // HandlerFATA-L

#include <pthread.h>
#include <signal.h>
#include <vector>

#include "JsonRPC.h"
#include "Cgi.h"


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t g_logMutex = PTHREAD_MUTEX_INITIALIZER;;

enum LogLevel
  {
    LL_DEBUG = 4,
    LL_INFO = 3,
    LL_WARN = 2,
    LL_ERROR = 1
  };


void mhlog(enum LogLevel level, const std::ostream& message)
{
  return;
}



#if 0
void CLog::WriteMsg(const char*p_sMsg,...)
{
	pthread_mutex_lock(&g_logMutex);
	if(( p_sMsg==( char*) 0)){
		WriteMsg("WARNING! You called the logging facility using the old style! Check log.h and then your code!");
		pthread_mutex_unlock(&g_logMutex);
		return;
	}
	va_list lstArg;
	va_start( lstArg, p_sMsg);
	writeMsg(false, p_sMsg, lstArg);
	va_end( lstArg );
	pthread_mutex_unlock(&g_logMutex);
}
#endif

/// @defgroup LibUnmarshaller Unmarshalling Library
/// @{ @}

#define _UPLOAD_TMP	NIVIS_TMP"upgrade_web/"
#define UPGRADE_LOCK	NIVIS_TMP"fw_upgrade.lock"

#define MAX_CONNECTIONS	15
#define SCGI_PID_FILE	NIVIS_TMP"scgi_svc.pid"

enum { AWAITING_REQUEST=1, PROCESSING, ALL_DONE };

struct conn_t
{
	pthread_t	tid ;
	pid_t		pid ;
	int		state;
	int		sd;
	bool		callTop ;
	IO		io;
	CJsonRPC*	jsonRPC ;
	char*		req[16] ; // 16requests is more than enough
	conn_t()
		: tid(0)
		, pid(0)
		, state(AWAITING_REQUEST)
		, sd(-1)
		, callTop(false)
		, jsonRPC(0)
		{
			memset(req,0,sizeof(req));
		}

	void close( void )
	{
		shutdown(sd, SHUT_RDWR );
		fclose(io.input);

		delete jsonRPC;
		jsonRPC=0;// make close re-entrant
		for ( int i=0; req[i]; ++i)
		{
			free(req[i]);
		}
		memset(req,0,sizeof(req));
		state = ALL_DONE;	//pending delete, no other operation is allowed
		if ( tid && ! pthread_kill(tid, 0) ) pthread_kill(tid, SIGKILL);
	}
} ;

conn_t*		vector[MAX_CONNECTIONS]={NULL,};
int g_nMaxFd ;

static int	connNewUnix( const char * path) ;
static void	connSetNonBlocking(int sk) ;
static void	connPoll(int srv_sk, fd_set *ifds, fd_set *ofds) ;

static int	connAccept( int srv_sk ) ;
static conn_t*	connAlloc( int cli_sk );
static int	connInsert( int cli_sk ) ;
static void	connDispatch( fd_set *ifds, fd_set *ofds) ;
//static conn_t* connFree( conn_t* cli_cn ) ;
//static void	connPrint();
static int	connProcess(conn_t *cli_cn) ;
static void	*HandleCall(void* c) ;

static ssize_t	readSize( int sk) ;
static bool	readComma( int sk) ;
static void	exportEnv(char*read_bf, size_t len, CEnvironment&env );
static int	handleMultipartForm(conn_t*cli_cn) ;
static int	handleTextPlain(conn_t*cli_cn) ;

int g_timeToQuit=0;

void handle_sigterm(int)
{
	LOG("TimeTOQuit");
	g_timeToQuit=1;
}

void handle_sigchld( int )
{
	const int oerrno = errno;
	pid_t pid;
	int status;

	/* Set up handler again. */
	(void) signal( SIGCHLD, handle_sigchld );

	/* Reap defunct children until there aren't any more. */
	for (;;)
	{
		pid = waitpid( (pid_t) -1, &status, WNOHANG );
		if ( (int) pid == 0 )		/* none left */
			break;
		if ( (int) pid < 0 )
		{
			if ( errno == EINTR || errno == EAGAIN )
				continue;
			/* ECHILD shouldn't happen with the WNOHANG option,
			 ** but with some kernels it does anyway.  Ignore it.
			 */
			if ( errno != ECHILD )
			{
				FPERR();
			}
			break;
		}
	}
	/* Restore previous errno. */
	errno = oerrno;
}

int main()
{
	struct sigaction sact;
	sigemptyset(&sact.sa_mask);
	sigset_t new_set;

	g_stLog.Open(NIVIS_TMP"scgi_svc.log");
	LOG("Starting SCGI SVC (version=%s) ...", VERSION);

	systemf_to( 500, "rm -rf "__SESSION_DIR"/*");

	signal(SIGPIPE, SIG_IGN );
	signal(SIGTERM, handle_sigterm );
	signal(SIGINT,  handle_sigterm );
	signal(SIGCHLD, handle_sigchld );

	// restart
	// killall scgi_svc
	// sleep
	// if ( pid ) killall -9 scgi_svc
	// execv(./scgi_svc)
	sigemptyset(&new_set);
	sigaddset(&new_set, SIGSEGV);
	sigaddset(&new_set, SIGABRT);

	sact.sa_flags = SA_NODEFER ;
	sact.sa_handler = HandlerFATAL ;
	if ( 0 != ( sigaction( SIGSEGV, &sact, NULL )
		  | sigaction( SIGABRT, &sact, NULL )
		  )
	   )
	{
		ERR("unable to install signals");
		return EXIT_FAILURE ;
	}

	if ( 0!= pthread_sigmask(SIG_UNBLOCK, &new_set, NULL) )
	{
		ERR("Unable to unblock signals");
		return EXIT_FAILURE;
	}

	int srv_sk = connNewUnix(NIVIS_FIRMWARE"www/wwwroot/rpc.cgi");
	if ( -1 == srv_sk ) return EXIT_FAILURE ;


	while ( !g_timeToQuit )
	{
		fd_set ifds, ofds;
		double dMin1, dMin5, dMin15, dTotal ;
		TouchPidFile(SCGI_PID_FILE);
		if (GetProcessLoad (dMin1, dMin5, dMin15, dTotal, GET_PROCESS_LOAD_AT_ONE_MIN))
			LOG("ProcessLoad: 1/5/15 min: %.2f/%.2f/%.2f total: %.2f", dMin1, dMin5, dMin15, dTotal );

		connPoll( srv_sk, &ifds, &ofds );
		if ( FD_ISSET(srv_sk,&ifds) )
		{
			int cli_sk=-1;
			if ( -1 == (cli_sk=connAccept(srv_sk)) )
			{
				continue ;
			}
			connInsert( cli_sk ) ;
			continue ;
		}
		connDispatch( &ifds, &ofds) ;
	}
	LOG("Exit");

	return EXIT_SUCCESS ;
}

/**
 *
 * @param path
 * @return
 */
static int connNewUnix(const char*path)
{
	int srv_sk, on=1, rv ;
	unlink(path);
	srv_sk = socket( AF_UNIX, SOCK_STREAM, 0 ) ;
	if ( -1 == srv_sk )
	{
		PERR("socket(%s)", path);
		return -1 ;
	}

	rv = setsockopt( srv_sk, SOL_SOCKET, SO_REUSEADDR, &on, (socklen_t)sizeof(on) ) ;
	if ( -1 == rv )
	{
		PERR("setsockopt(SO_REUSEADDR,%s)", path);
		close(srv_sk);
		return -1 ;
	}
	connSetNonBlocking(srv_sk);

	struct sockaddr_un  srv_name={0,};
	srv_name.sun_family	= AF_UNIX ;
	strncpy(srv_name.sun_path, path, sizeof(srv_name.sun_path) );
	srv_name.sun_path[sizeof(srv_name.sun_path)-1] = 0 ;

	rv = bind( srv_sk, (struct sockaddr*)&srv_name, (socklen_t)sizeof(srv_name) );
	if ( -1 == rv )
	{
		PERR("bind(%s)", path);
		close(srv_sk);
		return -1 ;
	}

	rv = listen( srv_sk, 5 );
	if ( -1 == rv )
	{
		PERR("listen(%s)", path);
		close(srv_sk);
		return -1 ;
	}
	chmod( path,0777) ;
	return srv_sk ;
}
/**
 *
 * @param sk
 */
static void connSetNonBlocking(int sk)
{
	int opts;

	opts = fcntl(sk,F_GETFL);
	if (opts < 0) {
		PERR("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sk,F_SETFL,opts) < 0) {
		PERR("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	return;
}
/**
 *
 * @param srv_sk
 * @param ifds
 * @param ofds
 */
static void connPoll(int srv_sk, fd_set *ifds, fd_set *ofds)
{
	FD_ZERO(ifds) ;
	FD_ZERO(ofds) ;

	FD_SET( srv_sk, ifds );

	g_nMaxFd = -1 ;
	pthread_mutex_lock(&mutex);
	for ( ssize_t i=0; i<MAX_CONNECTIONS;++i)
	{
		if (vector[i] && vector[i]->state==AWAITING_REQUEST )
		{
			FD_SET( vector[i]->sd, ifds );
			FD_SET( vector[i]->sd, ofds );
			g_nMaxFd = _Max(vector[i]->sd,g_nMaxFd);
		}
	}
	pthread_mutex_unlock(&mutex);
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	g_nMaxFd = _Max(g_nMaxFd,srv_sk);
	if ( -1 == select(g_nMaxFd+1, ifds, ofds, NULL, &tv) )
	{
		PERR("select()") ;
	}
}
/**
 *
 * @param srv_sk
 * @return
 */
static int connAccept( int srv_sk )
{
	int cli_sk ;
	struct sockaddr_in cli_name={0,};

	socklen_t cli_len=sizeof(cli_name);

	while ( -1 == (cli_sk=accept(srv_sk,(struct sockaddr*)&cli_name,&cli_len)) )
	{
		if ( errno != EINTR ) {
			FLOG("EINTR: Unable to accept") ;
			return -1 ;
		}
	}
	return cli_sk ;
}
/**
 *
 * @param cli_sk
 * @return
 */
static conn_t* connAlloc( int cli_sk )
{
	conn_t *c = new conn_t ;

	c->sd = cli_sk;
	c->io.input = c->io.output = fdopen(cli_sk,"r+");
	setvbuf(c->io.input,(char*)NULL, _IONBF, 0);

	//connPrint() ;
	return c;
}
/**
 *
 * @param cli_sk
 * @return
 */
static int connInsert(int cli_sk)
{
	size_t i;

	pthread_mutex_lock(&mutex);
	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (vector[i] == NULL)
		{
			if ((vector[i] = connAlloc(cli_sk)) == NULL)
				break ;

			pthread_mutex_unlock(&mutex);
			return 0;
		}
	}
	LOG("WARNING connInsert: no slots available");
	pthread_mutex_unlock(&mutex);
	close(cli_sk);
	return -1;
}
/**
 * @param ifds
 * @param ofds
 */
static void connDispatch( fd_set *ifds, fd_set *ofds )
{
	pthread_mutex_lock(&mutex);
	for ( ssize_t i=0; i<MAX_CONNECTIONS; ++i)
	{
		if ( ! vector[i] ) continue ;

		switch (vector[i]->state)
		{
		case AWAITING_REQUEST:
			if (FD_ISSET(vector[i]->sd,ifds) && FD_ISSET(vector[i]->sd,ofds) )
			{
				if ( ! connProcess(vector[i]) )
				{
					vector[i]->close();
					delete vector[i] ;
					vector[i]=NULL;
				}
			}
			break;
		case ALL_DONE:
			delete vector[i];
			vector[i]=NULL ;
		}
	}
	pthread_mutex_unlock(&mutex);
}
/**
 * @param cli_cn
 * @return
 */
/*static conn_t* connFree( conn_t* cli_cn )
{
	FLOG() ;
	return NULL ;
}*/
/**
 *
 */
/*static void connPrint()
{
	for ( ssize_t i=0; i<MAX_CONNECTIONS; ++i)
	{
		if ( ! vector[i] ) continue ;
		conn_t* cn = vector[i] ;
		FLOG("[%i] tid:%u state:%i sd:%i sh:%i", i, cn->tid, cn->state, cn->sd, cn->io.input);
	}
}*/
static int readScgiHeader(conn_t* cli_cn, char*&read_bf, ssize_t&len)
{
	len = readSize(cli_cn->sd);
	if ( len < 0 ) { FLOG("Invalid SCGI size.") ;return false ; }

	read_bf= new char [len+1] ;
	int rv = read( cli_cn->sd, read_bf, len ) ;
	if ( rv < len )
	{
		ERR("I've read(%i) less than expected(%i)", rv, len);
		delete [] read_bf ;
		return false ;
	}
	if ( ! readComma(cli_cn->sd) )
	{
		delete [] read_bf ;
		return false ;
	}
	return true ;

}

/**
 * @brief Process an incomming request.
 * @todo split/refactor
 * @param cli_cn
 * @return
 */
static int connProcess(conn_t *cli_cn)
{
	ssize_t len ;
	char *read_bf(0) ;

	if ( ! readScgiHeader(cli_cn,read_bf, len) ) return false ;

	cli_cn->jsonRPC =new CJsonRPC(cli_cn->io) ;
	exportEnv(read_bf, len, cli_cn->jsonRPC->io.env);
	delete [] read_bf ;
	/**************************/
	if ( cli_cn->jsonRPC->io.env.RequestMethod() != REQ_MET_POST
	||   cli_cn->jsonRPC->io.env.ContentLength() == 0 )
	{
		cli_cn->jsonRPC->JsonError( -32400 );
		return false ;
	}
	int contentType = cli_cn->jsonRPC->io.env.ContentType() ;

	if ( contentType == ContentType::Multipart::FormData )
	{
		if ( ! handleMultipartForm(cli_cn) )  { return false ;}
	}
	else if ( contentType == ContentType::Text::Plain )
	{
		if ( ! handleTextPlain(cli_cn) ) { return false ; }
	}
	else
	{
		cli_cn->jsonRPC->JsonError( -32701 );
		return false ;
	}

	cli_cn->state = PROCESSING ;

	int rv = pthread_create(&(cli_cn->tid), (pthread_attr_t*)NULL, HandleCall, (void*) cli_cn ) ;
	if ( rv )
	{
		cli_cn->jsonRPC->JsonError(-32400);
		return false ;
	}
	pthread_detach(cli_cn->tid);
	return true ;
}
/**
 *
 * @param c
 */
static void *HandleCall(void* c)
{

	//sigemptyset( &new_set );
	//sigaddset( &new_set, SIGSEGV );


	conn_t *cli_cn= (conn_t*)c ;
	if ( ! *cli_cn->req )
	{
		FLOG("No request to serve.");
		cli_cn->close();
		return NULL ;
	}
	cli_cn->pid = getpid() ;

	for ( size_t i=0; cli_cn->req[i] && i<sizeof(cli_cn->req) ; ++i)
	{
		if ( ! cli_cn->jsonRPC->HandleCall( cli_cn->req[i], cli_cn->callTop) )
			break ;
	}
	pthread_mutex_lock(&mutex) ;
	cli_cn->tid = 0 ;
	cli_cn->close();
	pthread_mutex_unlock(&mutex) ;
	pthread_exit(0) ;
}
/**
 *
 * @param sk
 * @return
 */
static ssize_t readSize(int sk)
{
	char read_buf[16]={0,};
	for ( unsigned off=0; off<10; ++off)
	{
		int rv= read(sk, (void*)(read_buf+off), 1 );
		if ( rv<= 0 ) return rv ;
		if ( read_buf[off]== ':' ) break ;
	}
	unsigned long len;
	sscanf( read_buf, "%9lu", &len );
	return len ;
}
/**
 *
 * @param sk
 * @return
 */
static bool readComma( int sk)
{
	char comma;
	int rv= read( sk, (void*)&comma, 1 );
	if ( rv<=0
	||  comma != ',' )
	{
		ERR("readComma failed");
		return false ;
	}
	return true ;
}
/**
 * @brief Export Environment variables given as netStrings.
 * @param read_bf
 * @param len
 */
static void exportEnv(char*read_bf, size_t len, CEnvironment&env )
{
	char*end=read_bf+len;
	do
	{
		char * var = read_bf ;
		read_bf += strlen(var) +1;
		char * val = read_bf ;
		read_bf += strlen(val) + 1 ;
		env.Put(var,val);
	}
	while (read_bf<end);
}
/**
 *
 * @param cli_cn
 * @return
 */
static int handleMultipartForm(conn_t*cli_cn)
{
	FLOG("multipart/formdata");
	char **vars, *tmp ;

	CGI cgi(cli_cn->jsonRPC->io) ;

	if ( ! cgi.Init( ) )
	{
		ERR("Unable to InitCGI");
		return false ;
	}

	CFileLock oLock(UPGRADE_LOCK);
	int nSecLeft = oLock.ForceOldLock(5*60); //use 5minutes without upload time and 15minutes with upload time
	// try to obtain the lock
	if (nSecLeft)
	{
		char szMsg[1024];
		sprintf(szMsg,"FW upgrade in progress please try again in %d minutes", nSecLeft/60 + 1);

		LOG(szMsg);

		cli_cn->jsonRPC->JsonError(szMsg,true) ;
		return true ;
	}

	//Remove _UPLOAD_TMP first
	if ( ! access(_UPLOAD_TMP, R_OK|W_OK) )
	{
		log2flash("CGI: dir "_UPLOAD_TMP" was not previously deleted");
		if ( system("rm -rf "_UPLOAD_TMP"* &>/dev/null" ) == -1 )
		{
			cli_cn->jsonRPC->JsonError("Unable remove temporary dir", true) ;
		};
	}

	// Recreate _UPLOAD_TMP
	if ( access(_UPLOAD_TMP, R_OK|W_OK) && mkdir(_UPLOAD_TMP , 0777) )
	{
		FLOG("Unable to make dir");
		cli_cn->jsonRPC->JsonError("Unable create temporary dir", true) ;
		exit(EXIT_FAILURE);
	}

	/***************************************************************/
	// get the file first then act uppon it
	vars = cgi.GetFiles ();
	if (vars)
	{
		s_file *file;
		for (int i=0; vars[i] != NULL; i++)
		{
			file = cgi.GetFile (vars[i]);
			if ( !file )
			{
				FLOG("Unable to get file");
				continue ;
			}

			tmp = EscapeSgml (file->filename);
			free (tmp);
			if (file->type)
			{
				tmp = EscapeSgml (file->type);
				free (tmp);
			}

			char dstfile[PATH_MAX]={0,};
			snprintf(dstfile, PATH_MAX, _UPLOAD_TMP"%s", file->filename );
			FLOG("FILE:[%s]", dstfile);

			if (rename(file->tmpfile, dstfile))
			{
				cli_cn->jsonRPC->JsonError("Unable to rename file", true);
				return false ;
			}
			log2flash("CGI: %s uploaded [%s]", cli_cn->jsonRPC->io.env.Get("REMOTE_ADDR"), dstfile ) ;
		}
		cgi.FreeList (vars);
	}
	else
	{
		LOG("No Files") ;
	}
	/***************************************************************/
	/***************************************************************/
	vars = cgi.GetVariables ();
	if (vars)
	{
		memset(cli_cn->req, 0, sizeof(cli_cn->req) ) ;
		for (int i=0,k=0; vars[i] != NULL; i++)
		{
			if ( strncmp( "call", vars[i], 4) )
				continue ;

			char *val = cgi.GetValue (vars[i]);
			if ( !val )
			{
				WARN("Unable to get variable");
				continue ;
			}
			while ( val && isspace(*val) ) val++ ;
			if ( '\0' == *val ) continue ;

			char *req = strdup(val) ;
			if ( ! req ) continue ;

			FLOG("var[%s] req:[%s]", vars[i], val) ;
			cli_cn->req[k++] = req;
		}
		cgi.FreeList (vars);
	}else
	{
		FLOG("No variables");
	}
	cgi.Free ();
	if (unlink(UPGRADE_LOCK)) LOG_ERR("upload unlink(%s)",UPGRADE_LOCK);
	oLock.Unlock();
	cli_cn->callTop = true ;
	return true ;
}
/**
 *
 * @param cli_cn
 * @return
 */
static int handleTextPlain(conn_t*cli_cn)
{
	//FLOG("text/plain");
	char *req = (char*)calloc( sizeof(char), cli_cn->jsonRPC->io.env.ContentLength() +8 );
	if ( !req )
	{
		cli_cn->jsonRPC->JsonError("alloc");
		return false;
	}

	for ( int br=0; br< cli_cn->jsonRPC->io.env.ContentLength(); )
	{
		int rv = read( cli_cn->sd, req+br, cli_cn->jsonRPC->io.env.ContentLength() - br);
		if ( (rv<0 && errno!=EAGAIN) || rv==0)
		{
			PERR("read rv:%d ContentLenght:%d",rv, cli_cn->jsonRPC->io.env.ContentLength() );
			free(req);
			return false ;
		}
		br +=rv ;
	}
	memset(cli_cn->req, 0, sizeof(cli_cn->req) ) ;
	cli_cn->req[0] = req;
	cli_cn->callTop = false ;
	return true ;
}
