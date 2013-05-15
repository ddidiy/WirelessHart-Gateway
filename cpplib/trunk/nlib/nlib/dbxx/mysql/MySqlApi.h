/*
 * MySqlApi.h
 *
 *  Created on: Nov 25, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_DBXX_MYSQLAPI_H_
#define NLIB_DBXX_MYSQLAPI_H_

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__WIN__)
#	define STDCALL __stdcall
#else
#	define STDCALL
#endif

#ifdef	__cplusplus
extern "C" {
#endif

//forward decl
struct st_mysql;
struct st_mysql_res;

typedef char **MYSQL_ROW;
typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;
typedef unsigned long long my_ulonglong;
typedef char my_bool;

enum mysql_option 
{
  MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_COMPRESS, MYSQL_OPT_NAMED_PIPE,
  MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP,
  MYSQL_SET_CHARSET_DIR, MYSQL_SET_CHARSET_NAME, MYSQL_OPT_LOCAL_INFILE,
  MYSQL_OPT_PROTOCOL, MYSQL_SHARED_MEMORY_BASE_NAME, MYSQL_OPT_READ_TIMEOUT,
  MYSQL_OPT_WRITE_TIMEOUT, MYSQL_OPT_USE_RESULT,
  MYSQL_OPT_USE_REMOTE_CONNECTION, MYSQL_OPT_USE_EMBEDDED_CONNECTION,
  MYSQL_OPT_GUESS_CONNECTION, MYSQL_SET_CLIENT_IP, MYSQL_SECURE_AUTH,
  MYSQL_REPORT_DATA_TRUNCATION, MYSQL_OPT_RECONNECT,
  MYSQL_OPT_SSL_VERIFY_SERVER_CERT
};

extern void STDCALL mysql_server_end(void);
extern MYSQL * STDCALL mysql_init(MYSQL *mysql);
extern int STDCALL mysql_options(MYSQL *mysql, enum mysql_option option, const char *arg);
extern MYSQL * STDCALL mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd,
  const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag);

extern void STDCALL mysql_close(MYSQL *sock);
extern unsigned int STDCALL mysql_errno(MYSQL *mysql);
extern const char * STDCALL mysql_error(MYSQL *mysql);

extern int		STDCALL mysql_query(MYSQL *mysql, const char *q);
extern MYSQL_RES *     STDCALL mysql_store_result(MYSQL *mysql);
extern my_ulonglong STDCALL mysql_insert_id(MYSQL *mysql);

extern void		STDCALL mysql_free_result(MYSQL_RES *result);
extern MYSQL_ROW	STDCALL mysql_fetch_row(MYSQL_RES *result);
extern my_ulonglong STDCALL mysql_num_rows(MYSQL_RES *res);
extern unsigned int STDCALL mysql_num_fields(MYSQL_RES *res);
extern int         STDCALL mysql_next_result(MYSQL *mysql);
extern MYSQL_RES * STDCALL mysql_use_result(MYSQL *mysql);


#ifdef	__cplusplus
}
#endif

//#include <mysql/mysql.h>

#endif /* NLIB_DBXX_MYSQLAPI_H_ */
