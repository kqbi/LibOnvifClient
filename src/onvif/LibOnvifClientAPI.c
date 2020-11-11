#include "LibOnvifClientAPI.h"
#include "comm/onvif_comm.h"

int ONVIF_GetStreamUri(const char *MediaXAddr, char *ProfileToken, char *uri, unsigned int sizeuri, const char *username, const char *password);
int ONVIF_GetSnapshotUri(const char *MediaXAddr, char *ProfileToken, char *uri, unsigned int sizeuri, const char *username, const char *password);

int ONVIF_ClientGetStreamUri(const char *DeviceXAddr, const char *username, const char *password, char rtspUri[256])
{
	int stmno = 0;
	int profile_cnt = 0;
	struct tagProfile *profiles = NULL;
	struct tagCapabilities capa;
	char uri[ONVIF_ADDRESS_SIZE] = { 0 };

	int flag = 1;
	ONVIF_GetCapabilities(DeviceXAddr, &capa, username, password,&flag);
	profile_cnt = ONVIF_GetProfiles(capa.MediaXAddr, &profiles, username, password);
	
	if (profile_cnt > stmno)
	{
		ONVIF_GetStreamUri(capa.MediaXAddr, profiles[stmno].token, uri, sizeof(uri),username,password);
		strcpy(rtspUri, uri);
	}
	else
	{
		flag = -1;
	}
	if (NULL != profiles)
	{
		free(profiles);
		profiles = NULL;
	}
	return flag;
}


int ONVIF_GetStreamUri(const char *MediaXAddr, char *ProfileToken, char *uri, unsigned int sizeuri, const char *username, const char *password)
{
	int result = 0;
	struct soap *soap = NULL;
	struct tt__StreamSetup              ttStreamSetup;
	struct tt__Transport                ttTransport;
	struct _trt__GetStreamUri           req;
	struct _trt__GetStreamUriResponse   rep;

	SOAP_ASSERT(NULL != MediaXAddr);
	SOAP_ASSERT(NULL != uri);
	memset(uri, 0x00, sizeuri);

	soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT);

	memset(&req, 0x00, sizeof(req));
	memset(&rep, 0x00, sizeof(rep));
	memset(&ttStreamSetup, 0x00, sizeof(ttStreamSetup));
	memset(&ttTransport, 0x00, sizeof(ttTransport));
	ttStreamSetup.Stream = tt__StreamType__RTP_Unicast;
	ttStreamSetup.Transport = &ttTransport;
	ttStreamSetup.Transport->Protocol = tt__TransportProtocol__RTSP;
	ttStreamSetup.Transport->Tunnel = NULL;
	req.StreamSetup = &ttStreamSetup;
	req.ProfileToken = ProfileToken;

	ONVIF_SetAuthInfo(soap, username, password);
	result = soap_call___trt__GetStreamUri(soap, MediaXAddr, NULL, &req, &rep);
	SOAP_CHECK_ERROR(result, soap, "GetServices");

	result = -1;
	if (NULL != rep.MediaUri)
	{
		if (NULL != rep.MediaUri->Uri)
		{
			if (sizeuri > strlen(rep.MediaUri->Uri))
			{
				strcpy(uri, rep.MediaUri->Uri);
				result = 0;
			}
			else
			{
				SOAP_DBGERR("Not enough cache!\n");
			}
		}
	}

EXIT:

	if (NULL != soap)
	{
		ONVIF_soap_delete(soap);
	}

	return result;
}

int GetDeviceCount(struct ProbeMatchNode *ProbeMatchHead)
{
	int count = 0;
	while (ProbeMatchHead)
	{
		++count;
		ProbeMatchHead = ProbeMatchHead->next;
	}

	if (!count)
		return count;

	return count - 1;
}

int ONVIF_ClientDetectDevice(struct ProbeMatchNode **ProbeMatchHead, int timeout)
{
	return	ONVIF_DetectDevice(ProbeMatchHead, timeout);
}

int ONVIF_ClientDetectDeviceCertainIp(const char *ip, int timeout,struct ProbeMatchNode **ProbeMatchHead)
{
	return	ONVIF_DetectDeviceCertainIp(ip, timeout, ProbeMatchHead);
}

int ONVIF_ClientSnapshotUri(const char *DeviceXAddr, const char *username, const char *password, char SnapshotUri[256])
{
	int stmno = 0;
	int profile_cnt = 0;
	struct tagProfile *profiles = NULL;
	struct tagCapabilities capa;

	char cmd[256];
	char uri[ONVIF_ADDRESS_SIZE] = { 0 };
	int flag = 1;
	ONVIF_GetCapabilities(DeviceXAddr, &capa,username,password,&flag);

	profile_cnt = ONVIF_GetProfiles(capa.MediaXAddr, &profiles, username, password);

	if (profile_cnt > stmno)
	{
		ONVIF_GetSnapshotUri(capa.MediaXAddr, profiles[stmno].token, uri, sizeof(uri), username, password);
		strcpy(SnapshotUri, uri);
	}
	else
	{
		flag = -1;
	}

	if (NULL != profiles)
	{
		free(profiles);
		profiles = NULL;
	}
	return flag;
}

int ONVIF_GetSnapshotUri(const char *MediaXAddr, char *ProfileToken, char *uri, unsigned int sizeuri,const char *username, const char *password)
{
	int result = 0;
	struct soap *soap = NULL;
	struct _trt__GetSnapshotUri         req;
	struct _trt__GetSnapshotUriResponse rep;

	SOAP_ASSERT(NULL != MediaXAddr);
	SOAP_ASSERT(NULL != uri);
	memset(uri, 0x00, sizeuri);

	soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT);

	ONVIF_SetAuthInfo(soap, username, password);

	memset(&req, 0x00, sizeof(req));
	memset(&rep, 0x00, sizeof(rep));
	req.ProfileToken = ProfileToken;
	result = soap_call___trt__GetSnapshotUri(soap, MediaXAddr, NULL, &req, &rep);
	SOAP_CHECK_ERROR(result, soap, "GetSnapshotUri");

	result = -1;
	if (NULL != rep.MediaUri) {
		if (NULL != rep.MediaUri->Uri) {
			if (sizeuri > strlen(rep.MediaUri->Uri)) {
				strcpy(uri, rep.MediaUri->Uri);
				result = 0;
			}
			else {
				SOAP_DBGERR("Not enough cache!\n");
			}
		}
	}

EXIT:

	if (NULL != soap) {
		ONVIF_soap_delete(soap);
	}

	return result;
}


int ClientGetDeviceCount(struct ProbeMatchNode *ProbeMatchHead)
{
	return GetDeviceCount(ProbeMatchHead);
}

void FreeProbeMatchList(struct ProbeMatchNode *ProbeMatchHead)
{
	Free_ProbeMatchNode(ProbeMatchHead);
}


int Make_Uri_Withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri)
{
	return  make_uri_withauth(src_uri, username, password, dest_uri, size_dest_uri);
}
