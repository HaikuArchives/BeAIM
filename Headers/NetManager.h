#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <Looper.h>
#include <Locker.h>
#include <String.h>
#include <kernel/OS.h>
//#include <net/socket.h>
//#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
#include <errno.h>
#include "DataContainer.h"
#include "GenList.h"
#include "constants.h"

#ifdef BEAIM_BONE
#	define closesocket(s)		close(s)
#endif

//-----------------------------------------------------

struct nlID {
	int nid;
	int type;
	
	// only the id matters for comparison purposes
	bool operator==( const nlID& id ) {
		return bool(nid == id.nid);
	};
};

//-----------------------------------------------------

class NetworkManager;

const uint32 NETLET_COULD_NOT_CONNECT = 'NOcn';
const uint32 NETLET_COULD_NOT_CONNECT_TO_PROXY = 'NOpn';
const uint32 NETLET_PROXY_BAD_AUTH = 'pRbA';
const uint32 NETLET_CONNECTED = 'YScn';
const uint32 NETLET_DISCONNECTED = 'YDcn';
const uint32 DATA_RECEIVED = 'dRCV';

//-----------------------------------------------------

enum netletStatus {
	NS_NOTCONNECTED,
	NS_CONNECTING,
	NS_CONNECTED
};

//-----------------------------------------------------

class Netlet {

	friend class NetworkManager;

	public:
		Netlet();
		~Netlet();
		void GrabContents( DataContainer&, int howmuch=-1, bool clear=true );
		bool IsDataWaiting();
		void Send( DataContainer );
		
	private:
	
		int32 ConnectThread();
		int32 ReadThread();
		static int32 ReadThreadEntry( void* );
		static int32 ConnectThreadEntry( void* );
		void AddToMessage( BMessage* msg, nlID nid );

		// proxy intermediate connection functions
		bool HTTPSConnect();
		bool SOCKS5Connect();
	
		nlID nid;
		netletStatus status;
		BString host;
		int socketID;
		int port;
		thread_id conThread, readThread;
		DataContainer data;
		BLocker lock;
		NetworkManager* owner;
		bool isDying;
		bool dataSent;
};

//-----------------------------------------------------

class NetworkManager : public BLooper {

	friend class Netlet;

	public:
		NetworkManager();
		~NetworkManager();
		
		void MessageReceived( BMessage* );
		
		nlID MakeNetlet( int type = -1 );	
		void Connect( nlID nid, BString host, int port );
		void Read( nlID nid, DataContainer& what, int howmuch = -1 );
		void Send( nlID nid, DataContainer what );
		bool IsDataWaiting( nlID nid );
		void Disconnect( nlID nid );
		void SetProxyInfo( netProxyMode mode, BString pHost, int32 pPort, bool auth, BString aName, BString aPass );
		void Shutdown();

	protected:

		// notification functions
		virtual void Connected( nlID netletID );
		virtual void ConnectError( nlID netletID, int what, BString error = "" );
		virtual void Disconnected( nlID netletID );
		virtual void DataReceived( nlID netletID );
	
	private:

		int FindNetlet( nlID );
		void RemoveNetlet( nlID );
		void ShutdownNetlet( Netlet* );

		// proxy settings
		netProxyMode proxyMode;
		BString proxyHost;
		int32 proxyPort;
		bool authenticate;
		BString proxyAuthUser;
		BString proxyAuthPass;
	
		// netlet stuff
		GenList<Netlet*> netlets;
		int netletIDPool;

		BLocker lock;
};

//-----------------------------------------------------

#endif
