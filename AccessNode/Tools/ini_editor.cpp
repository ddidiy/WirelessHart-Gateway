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

#include "../Shared/IniParser.h"
#include "../Shared/AnPaths.h"

// This ini_editor can also be run on a x86 Ubuntu Linux MiniPC which runs the ISA100 System Manager
// and Monitoring Host Application, to assist their reconfiguration: it is called by ubuntu-cfg.sh
// Compile it like this:
// $ cd AccessNode
// $ make clean
// $ cd Tools
// $ make host=i386 link=static ini_editor
// $ g++  -g -o ini_editor IniEditor.cpp ../Shared/libshared.a -DMINIPC_EDITOR -static
#ifdef MINIPC_EDITOR
#undef INI_FILE
#define INI_FILE "/usr/local/NISA/SystemManager/config.ini"
#endif

CIniParser g_Iniparser;

const char short_opts[] = "f:s:v:p:rw:da"; //file:name, section:name, variable:name,p:position, read, write:value,delete, raw

/*
Usage: ini_editor [-f <filename>] [-s <section>=NULL] -v <variable> [-p var_pos=-1] {-r | -w <value>| -d} [-a] 

-s <section> not present --> call ini_config.SetVar or GetVar with group=NULL

if -p specified --> call ini_config.SetVar or GetVar with position parameter equal with position
if -d is present --> delete variable
*/

int main( int argc, char* argv[])
{
	int opt;
	char filename[PATH_MAX] = INI_FILE;

	char szSectionInput[MAX_LINE_LEN] = {0,};
	char* section = NULL;

	bool read_req = false;
	bool raw = false;
	bool del_req = false;
	int var_pos = -1;
	static char variables[10][MAX_LINE_LEN];
	int var_count = 0;
	static char values[10][MAX_LINE_LEN];
	int val_count = 0, retCode=0;
	do{
		opt= getopt( argc, argv, short_opts);
		switch( opt){
			case 'f':
				strncpy( filename, optarg, sizeof(filename) );
				filename[ sizeof(filename)-1 ] = 0;
				break;
			case 's':
				strncpy( szSectionInput, optarg, MAX_LINE_LEN-1);
				section = szSectionInput;
				break;
			case 'v':
				strncpy( variables[var_count], optarg, MAX_LINE_LEN-1 );
				++var_count;
				break;
			case 'p':
				var_pos = atoi(optarg);
				break;
			case 'r':
				read_req = true;
				break;
			case 'w':
				strncpy( values[val_count], optarg, MAX_LINE_LEN-1 );
				++val_count;
				break;
			case 'd':
				del_req = true;
				break;
			case 'a':
				raw = true;
				break;
			case -1:
				if( (read_req || val_count || del_req) && var_count)
				{
					break;
				}

			default:
				printf("Usage: %s [-f <filename>] [-s <section>=NULL] -v <variable> [-p var_pos=-1] {-r | -w <value>| -d} [-a]\n", argv[0]);
				return 1;
		}
	}while( opt != -1);

	if (!g_Iniparser.Load(filename, "r+b", true) )
	{
		return 1;
	}
	if(var_pos == -1)//se executa pt toate varibilele introduse
	{
		int i, pos;
		if (val_count)
		{
			pos = 0;
			for( i=0; i<val_count && i<var_count; ++i){
				if ( i && !strcmp( variables[i-1], variables[i] )){
						++pos;
				}else{
						pos=0;
				}
				if(!raw){
					retCode = !g_Iniparser.SetVar( section, variables[i], values[i], pos, true);
				}else{
					retCode = !g_Iniparser.SetVarRawString( section, variables[i], values[i], pos, true);
				}
			}
		}
		else if (read_req)
		{
			char value[MAX_LINE_LEN];
			pos = 0;
			for( i=0; i<var_count; ++i){
				if ( i && !strcmp( variables[i-1], variables[i] )){
					++pos;
				}else{
					pos=0;
				}
				if(!raw){
					retCode = !g_Iniparser.GetVar( section, variables[i], value, MAX_LINE_LEN, pos);
				}else{
					retCode = !g_Iniparser.GetVarRawString( section, variables[i], value, MAX_LINE_LEN, pos);
				}
				printf("%s\n", value);
			}
		}
		else if(del_req)
		{
			pos = 0;
			for( i=0; i<var_count; ++i){
				if ( i && !strcmp( variables[i-1], variables[i] )){
					++pos;
				}else{
					pos = 0;
				}
				retCode = !g_Iniparser.DeleteVar(section, variables[i], 0, pos);
			}
		}
		return 0;
	}
	
	// position specified --> only one var will be affected

	if(val_count==1)
	{
		//test and log error for val_count>1

		if(!raw)
		{
			retCode = !g_Iniparser.SetVar( section, variables[0], values[0], var_pos, true);
		}
		else
		{
			retCode = !g_Iniparser.SetVarRawString( section, variables[0], values[0], var_pos, true);
		}
	}
	else if(read_req)
	{
		char value[MAX_LINE_LEN];
		if	(!raw)
		{
			retCode = !g_Iniparser.GetVar( section, variables[0], value, MAX_LINE_LEN, var_pos);
		}
		else
		{
			retCode = !g_Iniparser.GetVarRawString( section, variables[0], value, MAX_LINE_LEN, var_pos);
		}
		printf("%s\n", value);
	}
	else if(del_req)
	{
		retCode = !g_Iniparser.DeleteVar(section, variables[var_pos], 0, var_pos);
	}
	

	g_Iniparser.Release();
	return retCode;
}
