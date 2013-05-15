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

#include <unistd.h>
#include <getopt.h>

#include "../Shared/Common.h"

#include "../Shared/IniParser.h"
#include <string.h>
#include <stdio.h>

#define MAX_STRLEN 1024

#define COMMAND_UNK	0
#define COMMAND_HELP 	1
#define COMMAND_UPDATE 	2
#define COMMAND_SET 	3
#define COMMAND_DELETE  4

struct config_st
{
	int		snCommand;
	int		nMultVars;
	char szSourceFile[FILENAME_MAX];
	char szDestinationFile[FILENAME_MAX];
};

static char short_opts[] = "hc:ms:d:";
static struct option long_opts[] =
{

	{"help",		0, NULL, 'h'},
	{"command",		1, NULL, 'c'},
	{"multivar"	,	0, NULL, 'm'}, 
	{"source",		1, NULL, 's'},
	{"destination",		1, NULL, 'd'},
	{NULL,			0, NULL, 0}
};

const char *g_szHELP="Usage ini_update [-h] [-m] -c command_name"
		"-s source_file -d destination_file\n"
		"-h,  --help                 display this help and exit\n"
		"-m, --multivar              files can contain multiple consecutive variables with same name\n"
		"-c, --command command = [update|set|del] command to be executed:\n"
		"                            update - change variable(s) value from destination file \n"
		"                                     or add a section, variable name and value \n"
		"                                     from source file, if don't exist in destination file\n"
		"                            set    - change all variables values from destination file to\n"
		"                                     corresponding values from source file\n"
		"                            del    - for every (group, variable) source file will be delete\n"
		"                                     from destination file\n"
		"-d, --destination file_name destination file name\n"
		"-s, --source      file_name source file name\n";
		

//#define FILE_INI_UPDATE 

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static void InitConfigParam(config_st &cfg)
{
	memset(&cfg, 0, sizeof(cfg));
	cfg.snCommand = 0;
	cfg.nMultVars = 0;
	strcpy(cfg.szDestinationFile,"");
	strcpy(cfg.szSourceFile,"");
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static bool parseOptions( int argc, char *argv[], struct config_st &cfg)
{
	int opt;
	char *sArgs=new char[MAX_STRLEN];
	
	memset(sArgs, 0, MAX_STRLEN);
		
	do
	{
		opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
		memset(sArgs, 0, MAX_STRLEN);
		
		switch(opt)
		{
			case -1:
				break;
			case 'h':
				cfg.snCommand = COMMAND_HELP;
				break;
			case 'c':
				
				strcpy(sArgs,optarg);
				
				if (strcmp( sArgs,"u")==0 || strcmp( sArgs,"update")==0)
				{
					cfg.snCommand=COMMAND_UPDATE;
				}
				
				if(strcmp( sArgs,"s")==0 || strcmp( sArgs,"set")==0)
				{
					cfg.snCommand=COMMAND_SET;
				}

				if(strcmp( sArgs,"d")==0 || strcmp( sArgs,"del")==0)
				{
					cfg.snCommand=COMMAND_DELETE;
				}
				break;
			
			case 'd':
				strcpy(sArgs,optarg);
				strcpy( cfg.szDestinationFile, sArgs);
				break;
				
			case 's':
				strcpy(sArgs,optarg);
				strcpy( cfg.szSourceFile, sArgs);
				break;
			
			case 'm':
				cfg.nMultVars=1;
			
			default:
				LOG("Try ini_update -h for Help");
				//help(argv[0]);
				return false;
				
		}
		

	} while (opt != -1);
	LOG("Command  value=%d Source file:%s  Destination file:%s", 
		 cfg.snCommand, cfg.szSourceFile, cfg.szDestinationFile);			
				
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static bool CheckConfigParams(config_st &cfg)
{
	LOG("config_st values %d %s %s", cfg.snCommand,cfg.szSourceFile,  cfg.szDestinationFile);
	if (COMMAND_UNK==cfg.snCommand)
	{
		LOG("Incorrect command");
		LOG("Try ini_update -h for Help");
		exit(EXIT_FAILURE);
		return false;
	}

	
	if ( cfg.szDestinationFile[0] == 0)
	{
		LOG("Destination file name is missing");
		return false;
	}
		
	if (cfg.szSourceFile[0]==0)
	{
		LOG("Source file name is missing");
		return false;
	}	
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static void CommandSet(struct config_st &cfg)
{
	LOG("CommandSet");
	CIniParser oIniParserSource;
	CIniParser oIniParserDestination;
	
	if( !oIniParserSource.Load(cfg.szSourceFile, "r" ))
		return;
	
	Creat(cfg.szDestinationFile);
	if( !oIniParserDestination.Load(cfg.szDestinationFile,"r+b" ) )
		return;

	
	char szGroup[MAX_STRLEN];
	char szVarName[MAX_STRLEN];
	char szVarValueDest[MAX_STRLEN];
	char szVarValueSrc[MAX_STRLEN];
	
		
	if (oIniParserSource.FindFirstGroup(szGroup,sizeof(szGroup),false)==false)
		return;
	
	do 
	{		
		int pos = 0;
		char szPrevVarName[MAX_STRLEN];
		szPrevVarName[0]=0;

		if (oIniParserSource.FindFirstVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc))==false)
		{		
			continue;
		}
			
		do 		
		{
			//compute pos based on name
			// prev name == name -> pos++ else pos=0
			if (cfg.nMultVars && strcmp(szPrevVarName,szVarName) == 0)
			{			
				pos++;
			}
			else 
			{	pos = 0;
			}
			
			strcpy(szPrevVarName,szVarName);
			int retCode = oIniParserDestination.GetVar( szGroup, szVarName, szVarValueDest, sizeof(szVarValueDest), pos);
				
			LOG("VAR group=<%s> var_name=<%s> var_value_src=<%s> var_value_dest=<%s> found=%d pos=%d", szGroup, szVarName, szVarValueSrc, szVarValueDest,retCode,pos);	
			//if (retCode == 0 || szVarValueDest[0]==0)
			//{
			retCode = oIniParserDestination.SetVar( szGroup, szVarName, szVarValueSrc, pos, true);				
			LOG("		SET %s", retCode ? "SUCCESS": "FAILED");
			//}			
			
		} while  (oIniParserSource.FindNextVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc) )); 
		LOG("                                                                 ");

	} while(oIniParserSource.FindNextGroup(szGroup, sizeof(szGroup), true));
	

}


//////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////// 

static void CommandUpdate(struct config_st &cfg)
{
	LOG("CommandUpdate");
	CIniParser oIniParserSource;
	CIniParser oIniParserDestination;
	
	if( !oIniParserSource.Load(cfg.szSourceFile, "r" ))
		return;
	
	Creat(cfg.szDestinationFile);
	if( !oIniParserDestination.Load(cfg.szDestinationFile,"r+b" ) )
		return;

	
	char szGroup[MAX_STRLEN];
	char szVarName[MAX_STRLEN];
	char szVarValueDest[MAX_STRLEN];
	char szVarValueSrc[MAX_STRLEN];
	
		
	if (oIniParserSource.FindFirstGroup(szGroup,sizeof(szGroup),false)==false)
		return;
	
	do 
	{		
		int pos = 0;
		char szPrevVarName[MAX_STRLEN];
		szPrevVarName[0]=0;

		if (oIniParserSource.FindFirstVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc))==false)
		{		
			continue;
		}
			
		do 		
		{
			//compute pos based on name
			// prev name == name -> pos++ else pos=0
			if (cfg.nMultVars && strcmp(szPrevVarName,szVarName) == 0)
			{			
				pos++;
			}
			else 
			{	pos = 0;
			}
			
			strcpy(szPrevVarName,szVarName);
			int retCode = oIniParserDestination.GetVar( szGroup, szVarName, szVarValueDest, sizeof(szVarValueDest), pos);
				
			LOG("VAR group=<%s> var_name=<%s> var_value_src=<%s> var_value_dest=<%s> found=%d pos=%d", szGroup, szVarName, szVarValueSrc, szVarValueDest,retCode,pos);	
			if (retCode == 0 || szVarValueDest[0]==0)
			{
				retCode = oIniParserDestination.SetVar( szGroup, szVarName, szVarValueSrc, pos, true);				
				LOG("		UPDATE %s", retCode ? "SUCCESS": "FAILED");
			}			
			
		} while  (oIniParserSource.FindNextVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc) )); 
		LOG("                                                                 ");

	} while(oIniParserSource.FindNextGroup(szGroup, sizeof(szGroup), true));
	
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static void CommandDel(struct config_st &cfg)
{
	LOG("CommandDel");
	CIniParser oIniParserSource;
	CIniParser oIniParserDestination;
	
	if( !oIniParserSource.Load(cfg.szSourceFile, "r" ))
		return;
	
	Creat(cfg.szDestinationFile);
	if( !oIniParserDestination.Load(cfg.szDestinationFile,"r+b" ) )
		return;

	
	char szGroup[MAX_STRLEN];
	char szVarName[MAX_STRLEN];
	char szVarValueSrc[MAX_STRLEN];
	
		
	if (oIniParserSource.FindFirstGroup(szGroup,sizeof(szGroup),false)==false)
		return;
	
	do 
	{		
		int pos = 0;
		char szPrevVarName[MAX_STRLEN];
		szPrevVarName[0]=0;

		if (oIniParserSource.FindFirstVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc))==false)
		{		
			continue;
		}
			
		do 		
		{
			//compute pos based on name
			// prev name == name -> pos++ else pos=0
			if (cfg.nMultVars && strcmp(szPrevVarName,szVarName) == 0)
			{			
				pos++;
			}
			else 
			{	pos = 0;
			}
			
			strcpy(szPrevVarName,szVarName);	
			//if (retCode == 0 || szVarValueDest[0]==0)
			//{
			int retCode = oIniParserDestination.DeleteVar( szGroup, szVarName, 0, pos);				
			LOG("		DEL %s", retCode ? "SUCCESS": "FAILED");
			//}			
			
		} while  (oIniParserSource.FindNextVar(szVarName, sizeof(szVarName), szVarValueSrc, sizeof(szVarValueSrc) )); 
		LOG("                                                                 ");

	} while(oIniParserSource.FindNextGroup(szGroup, sizeof(szGroup), true));
}

int main(int argc, char **argv)
{
	g_stLog.OpenStdout();
	
	config_st cfg;
	
	InitConfigParam(cfg);
	
	parseOptions( argc, argv, cfg);
	LOG("=============================");
	LOG("%d",cfg.snCommand);
	
	if (COMMAND_HELP==cfg.snCommand)
	{
		LOG(g_szHELP);
		exit(EXIT_SUCCESS);
	} 

	
	if (false==CheckConfigParams(cfg))
	{
		LOG("EXIT_FAILURE");
 		exit(EXIT_FAILURE);
	}
	
	
	if (COMMAND_UPDATE==cfg.snCommand)
	{
		CommandUpdate(cfg);
	}
	else
	{
		if(COMMAND_SET==cfg.snCommand)
		{
			CommandSet(cfg);
		}
		else
		{
			if(COMMAND_DELETE==cfg.snCommand)
			{
				CommandDel(cfg);
			}
		}
	}	
	
	return 0;
}


