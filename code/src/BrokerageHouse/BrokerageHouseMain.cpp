/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * executable that opens the Brokerage House to business
 * 25 July 2006
 */

#include "BrokerageHouse.h"
#include "DBT5Consts.h"
#include "global_consts.h"

// Establish defaults for command line option
int iListenPort = iBrokerageHousePort;

char szHost[iMaxHostname + 1] = "";
char szDBName[iMaxDBName + 1] = "";
char szDBPort[iMaxPort + 1] = "";
char szUser[25] = "root";
char szSocket[256] = DB_SOCKET;
char szPass[25] = "";
char outputDirectory[iMaxPath + 1] = ".";

#ifdef NO_MEE_FOR_TRADERESULT
char szFileLoc[iMaxPath + 1] = ".";
int iActiveCustomerCount = 1000;
int iConfiguredCustomerCount = 1000;
char szBHaddr[iMaxHostname + 1] = "localhost"; // Brokerage House address
int iUsers = 1;
#endif

// shows program usage
void usage()
{
	cout << "Usage: BrokerageHouseMain [options]" << endl << endl;
 	cout << "   Option      Default    Description" << endl;
 	cout << "   =========   =========  ===============" << endl;
	cout << "   -d string              Database name" << endl;
 	cout << "   -h string   localhost  Database server" << endl;
 	printf("   -l integer  %-9d  Socket listen port\n", iListenPort);
 	cout << "   -o string   .          Output directory" << endl;
 	cout << "   -p integer             Database port" << endl;
	cout << endl;
}

// Parse command line
void parse_command_line(int argc, char *argv[])
{
	int arg;
	char *sp;
	char *vp;

	// Scan the command line arguments
	for (arg = 1; arg < argc; ++arg) {

		// Look for a switch
		sp = argv[arg];
		if (*sp == '-') {
			++sp;
		}
		*sp = (char) tolower(*sp);

		/*
		 *  Find the switch's argument.  It is either immediately after the
		 *  switch or in the next argv
		 */
		vp = sp + 1;
		// Allow for switched that don't have any parameters.
		// Need to check that the next argument is in fact a parameter
		// and not the next switch that starts with '-'.
		//
		if ((*vp == 0) && ((arg + 1) < argc) && (argv[arg + 1][0] != '-')) {
			vp = argv[++arg];
		}

		// Parse the switch
		switch (*sp) {
		case 'd': // Database name.
			strncpy(szDBName, vp, iMaxDBName);
			szDBName[iMaxDBName] = '\0';
			break;
		case 'h': // Database host name.
			strncpy(szHost, vp, iMaxHostname);
			szHost[iMaxHostname] = '\0';
			break;
		case 'o': // output directory
			strncpy(outputDirectory, vp, iMaxPath);
			outputDirectory[iMaxPath] = '\0';
			break;
		case 'p': // Postmaster port
			strncpy(szDBPort, vp, iMaxPort);
			szDBPort[iMaxPort] = '\0';
			break;
		case 'l':
			iListenPort = atoi(vp);
			break;
#ifdef NO_MEE_FOR_TRADERESULT
		case 'i':
			strncpy(szFileLoc, vp, iMaxPath);
			break;
		case 't':
			iConfiguredCustomerCount = atol(vp);
			break;
		case 'c':
			iActiveCustomerCount = atol(vp);
			break;
		case 'e':
			iUsers = atol(vp);
			break;
#endif
		case 'u':
			strcpy(szUser, vp);
			break;
		case 's':
			strcpy(szSocket, vp);
			break;
		case 'a':
			strcpy(szPass, vp);
			break;
		default:
			usage();
			cout << endl << "Error: Unrecognized option: " << sp << endl;
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	cout << "dbt5 - Brokerage House" << endl;
	cout << "Listening on port: " << iListenPort << endl << endl;

	// Parse command line
	parse_command_line(argc, argv);

	// Let the user know what settings will be used.
	cout << "Using the following database settings:" << endl;
	cout << "Host: " << szHost << endl;
	cout << "Database port: " << szDBPort << endl;
	cout << "Database name: " << szDBName << endl;

#ifdef DB_PGSQL
	CBrokerageHouse	BrokerageHouse(szHost, szDBName, szDBPort, iListenPort,
			outputDirectory);
#else
#ifdef NO_MEE_FOR_TRADERESULT
	CBrokerageHouse	BrokerageHouse(szDBName, szHost, szUser, szPass, szDBPort, szSocket,
									iListenPort, outputDirectory, szFileLoc, szBHaddr, iActiveCustomerCount, iConfiguredCustomerCount, iUsers);
#else
	CBrokerageHouse	BrokerageHouse(szDBName, szHost, szUser, szPass, szDBPort, szSocket,
									iListenPort, outputDirectory);
#endif
#endif
	cout << "Brokerage House opened for business, waiting traders..." << endl;
	try {
		BrokerageHouse.startListener();
	} catch (CBaseErr *pErr) {
		cout << "Error " << pErr->ErrorNum() << ": " << pErr->ErrorText();
		if (pErr->ErrorLoc()) {
			cout << " at " << pErr->ErrorLoc();
		}
		cout << endl;
		return 1;
	} catch (std::bad_alloc err) {
		// operator new will throw std::bad_alloc exception if there is not
		// sufficient memory for the request.
		cout << "*** Out of memory ***" << endl;
		return 2;
	} catch (const exception &e) {
		cout << e.what() << endl;
		return 3;
	}

	pthread_exit(NULL);

	cout << "Brokerage House closed for business" << endl;
	return(0);
}
