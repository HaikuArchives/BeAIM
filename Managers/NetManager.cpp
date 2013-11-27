#include <Application.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Say.h"
#include "Globals.h"
#include "NetManager.h"
#include "MiscStuff.h"

#ifndef BEAIM_BONE
#	include <net/socket.h>
#else
#	include <arpa/inet.h>
#endif

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

//=====================================================
// constants for SOCKS5 support

#define SOCKS5_VER					5
#define SOCKS5_CMD_CONNECT			1
#define SOCKS5_CMD_BIND				2
#define SOCKS5_CMD_ASSOCIATE			3
#define SOCKS5_ADDR_IPV4				1
#define SOCKS5_ADDR_DOMAIN			3
#define SOCKS5_ADDR_IPV6				4
#define SOCKS5_AUTH_NONE				0
#define SOCKS5_AUTH_GSSAPI			1
#define SOCKS5_AUTH_USER_PASS		2

//=====================================================

Netlet::Netlet() {
	nid.nid = -1;
	nid.type = -1;
	status = NS_NOTCONNECTED;
	socketID = -1;
	conThread = -1;
	owner = NULL;
	isDying = false;
	dataSent = false;
}

//-----------------------------------------------------

Netlet::~Netlet() {
}

//-----------------------------------------------------

int32 Netlet::ConnectThreadEntry( void* arg ) {
	Netlet* tcp = (Netlet*)arg;
	return tcp->ConnectThread();
}

//-----------------------------------------------------

int32 Netlet::ConnectThread() {

	printf( "connect thread started\n" );

	bool prelim = true;
	bool errorSent = false;
	BMessage* resMessage = NULL;
	BString rThreadName;
	BString conHost;
	int32 conPort;
	bool authenticate = false;
	BString proxyAuthUser;
	BString proxyAuthPass;
	netProxyMode proxyMode;

	lock.Lock();
	socketID = socket( AF_INET, SOCK_STREAM, 0 );
	
	// get the host/port of where we're really supposed to connect,
	// based on the current proxy settings
	owner->Lock();
	proxyMode = owner->proxyMode;
	if( proxyMode == NPM_NO_PROXY ) {
		conHost = host;
		conPort = port;
	} else {
		conHost = owner->proxyHost;
		conPort = owner->proxyPort;
		authenticate = owner->authenticate;
		proxyAuthUser = owner->proxyAuthUser;
		proxyAuthPass = owner->proxyAuthPass;
	}
	owner->Unlock();	
	lock.Unlock();
	
	printf( "initial connection: %s:%ld\n", conHost.String(), conPort );

	struct sockaddr_in sa;
	struct hostent *he;
	in_addr ia;	
	int s = sizeof(struct sockaddr_in);
	
	if( !conHost.Length() ) {
		lock.Lock();
		conThread = -1;
		status = NS_NOTCONNECTED;
		lock.Unlock();
		return 0;
	}

	ia.s_addr = INADDR_ANY;
	ia.s_addr = inet_addr( conHost.String() );
	if( ia.s_addr == INADDR_ANY || ia.s_addr == (unsigned long) -1 )
	{
		he = gethostbyname( conHost.String() );
		if(he != 0)
			ia.s_addr = *(int *)he->h_addr_list[0];
		else
		{
			if( lock.Lock() && !isDying ) {
				if( proxyMode == NPM_NO_PROXY )
					resMessage = new BMessage(NETLET_COULD_NOT_CONNECT);
				else
					resMessage = new BMessage(NETLET_COULD_NOT_CONNECT_TO_PROXY);
				AddToMessage( resMessage, nid );
				resMessage->AddBool( "netonly", true );
				owner->PostMessage( resMessage );
				lock.Unlock();
			}
			prelim = false;
			errorSent = true;
			lock.Lock();
			status = NS_NOTCONNECTED;
			conThread = -1;
			lock.Unlock();
			return 0;
		}
	}

	sa.sin_family = AF_INET;
	sa.sin_addr = ia;
	sa.sin_port = htons( conPort );

	if( prelim ) {
		if( connect( socketID, (struct sockaddr *) &sa, s ) < 0 )
			prelim = false;
 		else
			prelim = true;
	}

	// The Story So Far:
	//   At this point, we're connected, either to the target server or to a proxy server of
	//   some sort. If we're using in proxy mode (either HTTPS or SOCKS, doesn't matter)
	//   then we need to (possibly) authenticate ourselves and then attempt to make the proxy
	//   connect to the server we wanted in the first place. But both HTTPS and SOCKS5 have a
	//   few things that need to happen in between...
	
	// Attempt to connect via SOCKS5, and bail if the attempt fails
	if( prelim && proxyMode == NPM_SOCKS5_PROXY )
		if( !SOCKS5Connect() )
			return 0;
			
	// Attempt to connect via SOCKS5, and bail if the attempt fails
	else if( prelim && proxyMode == NPM_HTTPS_PROXY )
		if( !HTTPSConnect() )
			return 0;

 	if( prelim && ( getpeername( socketID, (struct sockaddr *) &sa, &s ) < 0 ) )
		prelim = false;

	if( prelim ) {
		rThreadName = host;
		rThreadName.Append("_read");
		lock.Lock();
		status = NS_CONNECTED;
		readThread = spawn_thread( ReadThreadEntry, rThreadName.String(), B_LOW_PRIORITY, this );
		if( readThread <= 0 ) {
			closesocket(socketID);
			status = NS_NOTCONNECTED;
			conThread = -1;
			readThread = -1;
			lock.Unlock();
			return 0;
		}
		resume_thread( readThread );
		if( !isDying ) {
			resMessage = new BMessage(NETLET_CONNECTED);
			AddToMessage( resMessage, nid );
			resMessage->AddBool( "netonly", true );
			owner->PostMessage( resMessage );
		}
		lock.Unlock();
	}
	else {
		closesocket(socketID);
		if( !errorSent ) {
			lock.Lock();
			if( !isDying ) {
				if( proxyMode == NPM_NO_PROXY )
					resMessage = new BMessage(NETLET_COULD_NOT_CONNECT);
				else
					resMessage = new BMessage(NETLET_COULD_NOT_CONNECT_TO_PROXY);
				AddToMessage( resMessage, nid );
				resMessage->AddBool( "netonly", true );
				owner->PostMessage( resMessage );
			}
			readThread = -1;
			status = NS_NOTCONNECTED;
			lock.Unlock();
		}
	}
	
	lock.Lock();
	conThread = -1;
	lock.Unlock();

	return 0;
}

//-----------------------------------------------------
//  SOCKS code stolen from various places, some of which I don't even remember,
//  and also figured out by reverse engineering and all that. Hope it works,
//  and that I'm not breaking any licenses by using it or anything.

bool Netlet::SOCKS5Connect() {

	char Buf[1024];
	Buf[0] = SOCKS5_VER;
	Buf[1] = 2; // methods
	Buf[2] = SOCKS5_AUTH_NONE;
	Buf[3] = SOCKS5_AUTH_USER_PASS;
	
	// authentication info
	BString proxyAuthUser = owner->proxyAuthUser;
	BString proxyAuthPass = owner->proxyAuthPass;
	bool proxyAuthenticate = owner->authenticate;
	bool success = false;
	int32 msgCode = NETLET_COULD_NOT_CONNECT;
	BString errString;
			
	// No idea how to implement this.
	// AuthReq[3] = SOCKS5_AUTH_GSSAPI;

	printf("[SOCKS5] Connected, Requesting authentication type.\n");
	send( socketID, (void*)Buf, 4, 0 );
	if( recv(socketID, (void*)Buf, 2, 0) == 2 )
	{
		if (Buf[0] == SOCKS5_VER)
		{
			bool Authenticated = false;
			switch( Buf[1] )
			{
				case SOCKS5_AUTH_NONE:
				{
					Authenticated = true;
					printf("[SOCKS5] No authentication needed.\n");
					break;
				}
				case SOCKS5_AUTH_USER_PASS:
				{
					if( proxyAuthenticate )
					{
						printf("[SOCKS5] User/Pass authentication needed.\n");
						char *b = Buf;
						*b++ = 1; // ver of sub-negotiation ??
								
						int NameLen = proxyAuthUser.Length();
						*b++ = (char)NameLen;
						strcpy(b, proxyAuthUser.String());
						b += NameLen;
	
						int PassLen = proxyAuthPass.Length();
						*b++ = (char)PassLen;
						strcpy(b, proxyAuthPass.String());
						b += PassLen;
	
						send( socketID, (void*)Buf, 3 + NameLen + PassLen, 0 );
						if( recv(socketID, (void*)Buf, 2, 0) == 2 )
						{
							Authenticated = bool(Buf[0] == 5 && Buf[1] == 0);
						}
					}

					if( !Authenticated )
					{
						printf("[SOCKS5] User/Pass authentication failed.\n");
						msgCode = NETLET_PROXY_BAD_AUTH;
					}
					break;
				}
			}

			if (Authenticated)
			{
				printf("[SOCKS5] Authentication successful.\n");

				int HostPort = htons(port);

				// Header
				char *b = Buf;
				*b++ = SOCKS5_VER;
				*b++ = SOCKS5_CMD_CONNECT;
				*b++ = 0; // reserved

				long IpAddr = inet_addr(host.String());
				if (IpAddr != -1)
				{
					// Ip
					*b++ = SOCKS5_ADDR_IPV4;
					memcpy(b, &IpAddr, 4);
					b += 4;
				}
				else
				{
					// Domain Name
					*b++ = SOCKS5_ADDR_DOMAIN;
					int Len = host.Length();
					*b++ = Len;
					strcpy(b, host.String());
					b += Len;
				}

				// Port
				memcpy(b, &HostPort, 2);
				b += 2;

				send( socketID, (void*)Buf, (int)b - (int)Buf, 0 );
				if( recv(socketID, (void*)Buf, 10, 0) == 10 )
				{
					if (Buf[0] == SOCKS5_VER)
					{
						switch (Buf[1])
						{
							case 0:
								// yay! connected!
								success = true;
								break;
							case 1:
								errString = "SOCKS5: General SOCKS server failure!";
								break;
							case 2:
								errString = "SOCKS5: Connection not allowed by ruleset.";
								break;
							case 3:
								errString = "SOCKS5: Network unreachable.";
								break;
							case 4:
								errString = "SOCKS5: Host unreachable.";
								break;
							case 5:
								errString = "SOCKS5: Connection refused.";
								break;
							case 6:
								errString = "SOCKS5: TTL expired.";
								break;
							case 7:
								errString = "SOCKS5: command not supported.";
								break;
							case 8:
								errString = "SOCKS5: Address type not supported.";
								break;
							default:
								errString = "SOCKS5: Unknown SOCKS server failure";
								break;
						}
					}
					else
					{
						errString = "SOCKS5: Wrong socks version.";
					}
				}
				else
				{
					errString = "SOCKS5: Connection request read failed.";
				}
			}
			else
			{
				errString = "SOCKS5: Not authenticated.";
			}
		}
		else
		{
			errString = "SOCKS5: Wrong socks version.";
		}
	}
	else
	{
		errString = "SOCKS5: Authentication type read failed.";
	}
	
	// if we couldn't connect to the final server for whatever reason, send out the failure message
	if( !success ) {	
		BMessage* resMessage = new BMessage(msgCode);
		AddToMessage( resMessage, nid );
		if( errString.Length() )
			resMessage->AddString( "errorstring", errString.String() );
		resMessage->AddBool( "netonly", true );
		lock.Lock();
		if( !isDying )
			owner->PostMessage( resMessage );
		else delete resMessage;
		readThread = -1;
		status = NS_NOTCONNECTED;
		lock.Unlock();
		return false;
	}
}

//-----------------------------------------------------

bool Netlet::HTTPSConnect() {

	// authentication info
	BString proxyAuthUser = owner->proxyAuthUser;
	BString proxyAuthPass = owner->proxyAuthPass;
	bool proxyAuthenticate = owner->authenticate;
	
	DataContainer sendStuff, recvStuff;
	const int datSize = 2048;
	BString userPassString, response;
	BMessage* resMessage = NULL;
	char cdata[datSize];
	ssize_t recvSize;
	timeval tv;
	fd_set fds;
	bool again;
	bool prelim = true;
		
	printf( "HTTPS proxy! yay!\n" );

	// build the connect request string and send to the HTTPS server
	sendStuff << "CONNECT ";
	sendStuff << (char*)host.String() << ":" << port << " ";
	sendStuff << "HTTP/1.0" << char(0xD) << char(0xA);
	sendStuff << "User-agent: ";
	sendStuff << "BeAIM/" << (char*)BeAIMVersion(false).String();
	sendStuff << char(0xD) << char(0xA);
		
	// HTTPS authentication involves tacking on some more fields, including
	// the name:pass information (this has to be Base64 encoded)
	if( proxyAuthenticate ) {
		userPassString = proxyAuthUser;
		userPassString.Append(":");
		userPassString.Append( proxyAuthPass );
		userPassString = Base64Encode(userPassString);
		sendStuff << "Authorization: Basic " << userPassString.String();
		sendStuff << char(0xD) << char(0xA);
		sendStuff << "Proxy-authorization: Basic " << userPassString.String();
		sendStuff << char(0xD) << char(0xA);
	}
	sendStuff << char(0xD) << char(0xA);
	send( socketID, sendStuff.c_ptr(), sendStuff.length(), 0 );

	// now we wait for the HTTPS server to respond one way or the other... either
	// we were connected, the authentication failed, or something else went wrong.
	// grab all the data from the server's response, to be analyzed in just a bit...
	recvSize = recv(socketID, (void*)cdata, datSize, 0);
	do {
		again = false;
		recvStuff << DataContainer(cdata,recvSize);
		FD_ZERO(&fds);
		FD_SET(socketID, &fds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		select(32, &fds, NULL, NULL, &tv);

		if( FD_ISSET(socketID, &fds) ) {
			recvSize = recv(socketID, (void*)cdata, datSize, 0);
			again = true;			
		}
	} while( recvSize > 0 && again );

	// we should have a server response at this point, or something else has gone wrong...
	// grab the first 50 characters of the response and we'll find out.
	if( recvStuff.length() > 50 )
		recvStuff[50] = char('\0');
	else
		recvStuff << char('\0');
	response = BString( recvStuff.c_ptr() );
	prelim = false;
		
	// yank the HTTP status code
	if( response.FindFirst(" ") != B_ERROR ) {
		response.Remove(0, 1+response.FindFirst(" "));
		response.Truncate(3);

		// 200 means we connected OK, anything else is a Bad Thing
		if( response == "200" )
			prelim = true;
				
		// probably an authorization failure
		else if( response == "403" || response == "407" || response == "401" )
			resMessage = new BMessage(NETLET_PROXY_BAD_AUTH);

		// the proxy couldn't connect to the requested server
		else if( response == "503" || response == "504" )
			resMessage = new BMessage(NETLET_COULD_NOT_CONNECT);
	}
		
	// if we couldn't connect to the final server for whatever reason, send out the failure message
	if( !prelim ) {	
		AddToMessage( resMessage, nid );
		resMessage->AddBool( "netonly", true );
		lock.Lock();
		if( !isDying )
			owner->PostMessage( resMessage );
		else delete resMessage;
		readThread = -1;
		status = NS_NOTCONNECTED;
		lock.Unlock();
		return false;
	}
	
	// must have gone OK... declare victory
	return true;
}

//-----------------------------------------------------

int32 Netlet::ReadThreadEntry( void* arg ) {
	Netlet* tcp = (Netlet*)arg;
	return tcp->ReadThread();
}

//-----------------------------------------------------

int32 Netlet::ReadThread() {

	const int datSize = 2048;
	char cdata[datSize];
	timeval tv;
	fd_set fds;
	bool again;
	bool done = false;
	ssize_t recvSize;
	int selRes;
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	while( !done ) {

		// wait for data, read it, and send it out, repeat ad nauseum
		while( (recvSize = recv(socketID, (void*)cdata, datSize, 0)) > 0 ) {
		
			// loop through and grab all the data on the socket
			do {
				again = false;
				lock.Lock();
				data << DataContainer(cdata,recvSize);
				lock.Unlock();
	
				// do a select on the socket to see if there's any more data
				// waiting to be read and stuck on to the current packet
				FD_ZERO(&fds);
				FD_SET(socketID, &fds);
				selRes = select(32, &fds, NULL, NULL, &tv);
				
				// if there's any data waiting, try and grab it
				if( FD_ISSET(socketID, &fds) ) {
					recvSize = recv(socketID, (void*)cdata, datSize, 0);
					again = true;			
				}

			} while( recvSize > 0 && again );
			BMessage* msg = new BMessage(DATA_RECEIVED);
			AddToMessage( msg, nid );
			msg->AddBool( "netonly", true );
			owner->PostMessage( msg );
		}

		// evil hack to keep the Netlet (in pre-BONE BeOS environments)
		// from thinking it's disconnected, just because another thread sent
		// data on this socket, which caused this thread to unblock. Yech.
		lock.Lock();
		if( dataSent )
			done = dataSent = false;
		else
			done = true;
		lock.Unlock();
	}

	// we're disconnected now, for real this time.
	lock.Lock();
	if( !isDying ) {
		BMessage* msg = new BMessage(NETLET_DISCONNECTED);
		AddToMessage( msg, nid );
		owner->PostMessage( msg );
	}
	readThread = -1;
	lock.Unlock();
	return 0;
}

//-----------------------------------------------------

void Netlet::GrabContents( DataContainer& ret, int howmuch, bool clear ) {

	lock.Lock();
	if( howmuch == -1 || howmuch > data.length() ) {
		ret = data;
		if( clear )
			data = DataContainer();
	}
	else {
		ret = data.getrange(0,howmuch);
		if( clear )
			data = data.getrange(howmuch,-1);
	}
	lock.Unlock();
}

//-----------------------------------------------------

bool Netlet::IsDataWaiting() {

	bool ret = false;
	lock.Lock();
	if( data.length() )
		ret = true;
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

void Netlet::Send( DataContainer what ) {

	lock.Lock();
	send( socketID, what.c_ptr(), what.length(), 0 );
	dataSent = true;
	lock.Unlock();
}

//-----------------------------------------------------

void Netlet::AddToMessage( BMessage* msg, nlID nid ) {
	msg->AddInt32( "netlet_nid", nid.nid );
	msg->AddInt32( "netlet_type", nid.type );
}

//=====================================================

NetworkManager::NetworkManager()
			  : BLooper("NetworkManager")
{
	netletIDPool = 0;
	proxyMode = NPM_NO_PROXY;
	Run();
}

//-----------------------------------------------------

NetworkManager::~NetworkManager() {

}

//-----------------------------------------------------

nlID NetworkManager::MakeNetlet( int type ) {

	nlID netID;
	lock.Lock();

	netID.nid = netletIDPool++;
	netID.type = type;
	
	Netlet* newNet = new Netlet;
	newNet->owner = this;
	newNet->nid = netID;
	netlets.Add( newNet );

	lock.Unlock();
	return netID;
}

//-----------------------------------------------------

void NetworkManager::Connect( nlID nid, BString host, int port ) {

	lock.Lock();
	Netlet* temp = NULL;
	int nindex = FindNetlet(nid);

	// start the netlet connecting...
	if( nindex != -1 && netlets[nindex]->status == NS_NOTCONNECTED ) {
		temp = netlets[nindex];
		temp->host = host;
		temp->port = port;
		temp->status = NS_CONNECTING;
		temp->conThread = spawn_thread( temp->ConnectThreadEntry, "netlet_connect", B_LOW_PRIORITY, temp );
		if( temp->conThread <= 0 ) {
			lock.Unlock();
			return;
		}
		resume_thread( temp->conThread );
	}

	lock.Unlock();
}

//-----------------------------------------------------

void NetworkManager::Read( nlID nid, DataContainer& dtemp, int howmuch ) {

	lock.Lock();
	int nindex = FindNetlet(nid);
	if( nindex == -1 ) {
		lock.Unlock();
		return;
	}
	netlets[nindex]->GrabContents( dtemp, howmuch );
	lock.Unlock();
}

//-----------------------------------------------------

void NetworkManager::Send( nlID nid, DataContainer dtemp ) {

	lock.Lock();
	int nindex = FindNetlet(nid);
	if( nindex == -1 ) {
		lock.Unlock();
		return;
	}
	netlets[nindex]->Send(dtemp);
	lock.Unlock();
}

//-----------------------------------------------------

void NetworkManager::Disconnect( nlID nid ) {

	printf( "NetworkManager::Disconnect: %d\n", nid.nid );
	
	// don't try to disconnect an invalid netlet
	if( nid.nid == -1 )
		return;
	
	lock.Lock();
	RemoveNetlet(nid);
	lock.Unlock();
}

//-----------------------------------------------------

void NetworkManager::RemoveNetlet( nlID nid ) {

	printf( "NetworkManager::RemoveNetlet: %d\n", nid.nid );

	Netlet* templet;
	lock.Lock();
	int nindex = FindNetlet(nid);
	if( nindex == -1 ) {
		lock.Unlock();
		return;
	}
	templet = netlets[nindex];
	netlets.Delete(nindex);
	ShutdownNetlet( templet );
	lock.Unlock();
}

//-----------------------------------------------------

void NetworkManager::MessageReceived( BMessage* msg ) {

	nlID netletID;
	netletID.nid = msg->FindInt32("netlet_nid");
	netletID.type = msg->FindInt32("netlet_type");

	switch( msg->what ) {
	
		case NETLET_CONNECTED:
			Connected( netletID );
			break;
		
		case NETLET_COULD_NOT_CONNECT_TO_PROXY:
		case NETLET_COULD_NOT_CONNECT:
		case NETLET_PROXY_BAD_AUTH:
			RemoveNetlet( netletID );
			if( msg->HasString("errorstring") )
				ConnectError( netletID, msg->what, BString(msg->FindString("errorstring")) );
			else
				ConnectError( netletID, msg->what );
			break;

		case NETLET_DISCONNECTED:
			RemoveNetlet( netletID );
			Disconnected( netletID );
			break;

		case DATA_RECEIVED:
			DataReceived( netletID );
			break;
	}
}

//-----------------------------------------------------

void NetworkManager::Shutdown() {

	lock.Lock();
	Netlet* templet;
	
	printf( "number of netlets to shut down: %ld\n", (int32)netlets.Count() );

	// cycle through all the netlets and close them down
	for( unsigned i = 0; i < netlets.Count(); ++i ) {
		templet = netlets[i];
		netlets[i] = NULL;
		ShutdownNetlet( templet );
	}

	// the netlets are all dead now, so we can clear the list
	netlets.Clear();
	lock.Unlock();

	// now shutdown NetManager
	Lock();
	Quit();
}

//-----------------------------------------------------

void NetworkManager::ShutdownNetlet( Netlet* templet ) {

	lock.Lock();

	printf( "ShutdownNetlet: %d\n", templet->nid.nid );

	// kill the netlet's various threads and delete it
	templet->lock.Lock();
	templet->isDying = true;
	shutdown(templet->socketID, 2);
	closesocket(templet->socketID);
	if( templet->conThread != -1 )
		kill_thread( templet->conThread );
	if( templet->readThread != -1 )
		kill_thread( templet->readThread );
	templet->lock.Unlock();
	delete templet;
	
	lock.Unlock();
}

//-----------------------------------------------------

int NetworkManager::FindNetlet( nlID nid ) {

	int ret = -1;
	lock.Lock();
	for( unsigned i = 0; i < netlets.Count(); ++i ) {
		if( netlets[i]->nid == nid ) {
			ret = (int)i;
			break;
		}
	}
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

bool NetworkManager::IsDataWaiting( nlID nid ) {

	bool ret = false;
	lock.Lock();
	int nindex = FindNetlet(nid);
	if( nindex == -1 ) {
		lock.Unlock();
		return false;
	}
	ret = netlets[nindex]->IsDataWaiting();
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

void NetworkManager::SetProxyInfo( netProxyMode mode, BString pHost, int32 pPort, bool auth, BString aName, BString aPass ) {

	// spit out some debug info
	if( beaimDebug ) {
	
		printf( "SetProxyInfo called:\n" );
		switch( mode ) {
			case NPM_NO_PROXY:
				printf( "ProxyMode.... NPM_NO_PROXY\n" );
				break;
			case NPM_HTTPS_PROXY:
				printf( "ProxyMode.... NPM_HTTPS_PROXY\n" );
				break;
			case NPM_SOCKS5_PROXY:
				printf( "ProxyMode.... NPM_SOCKS5_PROXY\n" );
				break;
		}
		if( proxyMode != NPM_NO_PROXY ) {
			printf( "server: %s:%ld\n", pHost.String(), pPort );
			printf( "auth: %s\n", auth ? "yup" : "nope" );
			if( auth )
				printf( "name/pass: %s/%s\n", aName.String(), aPass.String() );
		}
	}

	// set the actual info
	lock.Lock();
	proxyMode = mode;
	proxyHost = pHost;
	proxyPort = pPort;
	authenticate = auth;
	proxyAuthUser = aName;
	proxyAuthPass = aPass;
	lock.Unlock();
}

//-----------------------------------------------------
// notification functions... these are empty because they are
// meant to be handled by derived classes.

void NetworkManager::Connected( nlID netletID )
{
	printf( "NetworkManager::Connected\n" );
}

void NetworkManager::ConnectError( nlID netletID, int what, BString errorStr )
{
	printf( "NetworkManager::ConnectError (unknown)\n" );
}

void NetworkManager::Disconnected( nlID netletID )
{
	printf( "NetworkManager::Disconnected\n" );
}

void NetworkManager::DataReceived( nlID netletID )
{
	printf( "NetworkManager::DataReceived\n" );
}

//-----------------------------------------------------
