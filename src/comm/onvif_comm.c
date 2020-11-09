#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "onvif/wsseapi.h"
#include "onvif_comm.h"
#include "onvif_dump.h"

/*
int substr(char *s1, char *s2)
{
	char *s3 = strstr(s1, s2);
	if (s3 == NULL)
	    return -1;
	return strlen(s1) - strlen(s3);
}
*/

void soap_perror(struct soap *soap, const char *str)
{
    if (NULL == str) {
        SOAP_DBGERR("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    } else {
        SOAP_DBGERR("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    }
}

void* ONVIF_soap_malloc(struct soap *soap, unsigned int n)
{
    void *p = NULL;

    if (n > 0) {
        p = soap_malloc(soap, n);
        SOAP_ASSERT(NULL != p);
        memset(p, 0x00 ,n);
    }
    return p;
}

struct soap *ONVIF_soap_new(int timeout)
{
    struct soap *soap = NULL;                                                   // soap��������

    SOAP_ASSERT(NULL != (soap = soap_new()));
	
    soap_set_namespaces(soap, namespaces);                                      // ����soap��namespaces
    soap->recv_timeout    = timeout;                                            // ���ó�ʱ������ָ��ʱ��û�����ݾ��˳���
    soap->send_timeout    = timeout;
    soap->connect_timeout = timeout;

#if defined(__linux__) || defined(__linux)                                      // �ο�https://www.genivia.com/dev.html#client-c���޸ģ�
    soap->socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
#endif

    soap_set_mode(soap, SOAP_C_UTFSTRING);                                      // ����ΪUTF-8���룬�����������OSD������

    return soap;
}

void ONVIF_soap_delete(struct soap *soap)
{
    soap_destroy(soap);                                                         // remove deserialized class instances (C++ only)
    soap_end(soap);                                                             // Clean up deserialized data (except class instances) and temporary data
    soap_done(soap);                                                            // Reset, close communications, and remove callbacks
    soap_free(soap);                                                            // Reset and deallocate the context created with soap_new or soap_copy
}

/************************************************************************
**������ONVIF_SetAuthInfo
**���ܣ�������֤��Ϣ
**������
        [in] soap     - soap��������
        [in] username - �û���
        [in] password - ����
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
************************************************************************/
int ONVIF_SetAuthInfo(struct soap *soap, const char *username, const char *password)
{
    int result = 0;

    SOAP_ASSERT(NULL != username);
    SOAP_ASSERT(NULL != password);

    result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
    SOAP_CHECK_ERROR(result, soap, "add_UsernameTokenDigest");

EXIT:

    return result;
}

/************************************************************************
**������ONVIF_init_header
**���ܣ���ʼ��soap������Ϣͷ
**������
        [in] soap - soap��������
**���أ���
**��ע��
    1). �ڱ������ڲ�ͨ��ONVIF_soap_malloc������ڴ棬����ONVIF_soap_delete�б��ͷ�
************************************************************************/
void ONVIF_init_header(struct soap *soap)
{
    struct SOAP_ENV__Header *header = NULL;

    SOAP_ASSERT(NULL != soap);

    header = (struct SOAP_ENV__Header *)ONVIF_soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
    soap_default_SOAP_ENV__Header(soap, header);
    header->wsa__MessageID = (char*)soap_wsa_rand_uuid(soap);
    header->wsa__To        = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TO) + 1);
    header->wsa__Action    = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ACTION) + 1);
    strcpy(header->wsa__To, SOAP_TO);
    strcpy(header->wsa__Action, SOAP_ACTION);
    soap->header = header;

    return;
}

/************************************************************************
**������ONVIF_init_ProbeType
**���ܣ���ʼ��̽���豸�ķ�Χ������
**������
        [in]  soap  - soap��������
        [out] probe - ���Ҫ̽����豸��Χ������
**���أ�
        0����̽�⵽����0����δ̽�⵽
**��ע��
    1). �ڱ������ڲ�ͨ��ONVIF_soap_malloc������ڴ棬����ONVIF_soap_delete�б��ͷ�
************************************************************************/
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe)
{
    struct wsdd__ScopesType *scope = NULL;                                      // �����������������Web����

    SOAP_ASSERT(NULL != soap);
    SOAP_ASSERT(NULL != probe);

    scope = (struct wsdd__ScopesType *)ONVIF_soap_malloc(soap, sizeof(struct wsdd__ScopesType));
    soap_default_wsdd__ScopesType(soap, scope);                                 // ����Ѱ���豸�ķ�Χ
    scope->__item = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ITEM) + 1);
    strcpy(scope->__item, SOAP_ITEM);

    memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
    soap_default_wsdd__ProbeType(soap, probe);
    probe->Scopes = scope;
    probe->Types  = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TYPES) + 1);     // ����Ѱ���豸������
    strcpy(probe->Types, SOAP_TYPES);

    return;
}

int ONVIF_DetectDeviceCertainIp(const char *ip, int timeout, struct ProbeMatchNode **ProbeMatchHead)
{
	int ret = 0;
	struct soap *soap = NULL;
	struct wsdd__ProbeType req;
	struct __wsdd__ProbeMatches rep;
	struct ProbeMatchNode *temProbeMatchNode = (struct ProbeMatchNode *)malloc(sizeof(struct ProbeMatchNode));
	temProbeMatchNode->pre = NULL;
	temProbeMatchNode->next = NULL;
	temProbeMatchNode->DeviceXAddr = NULL;
	temProbeMatchNode->ipcIp = NULL;
	temProbeMatchNode->ipcUuid = NULL;
	*ProbeMatchHead = temProbeMatchNode;
	soap = ONVIF_soap_new(timeout);
	if (!soap)
		return -1;  //����soap����ʧ��

	ONVIF_init_header(soap);
	ONVIF_init_ProbeType(soap, &req);

	char endPoint[50];
	sprintf(endPoint, "soap.udp://%s:3702", ip);

	int result = soap_send___wsdd__Probe(soap, endPoint, NULL, &req);

	if (SOAP_OK == result)
	{
		memset(&rep, 0x00, sizeof(rep));
		if (SOAP_OK == soap_recv___wsdd__ProbeMatches(soap, &rep))
		{
			if (soap->error)
			{
				soap_perror(soap, "ProbeMatches");
			}
			else
			{
				dump__wsdd__ProbeMatches(&rep);
				if (NULL != rep.wsdd__ProbeMatches)
				{
					int i = 0;
					
					for (; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++)
					{
						/*
						std::string addr;
						int flagIndex = 0;
						addr = rep.wsdd__ProbeMatches->ProbeMatch->XAddrs;
						flagIndex = addr.find_first_of(" ");
						if (flagIndex > 0)
							ipcInfo.ipcDevAddr = addr.substr(0, flagIndex);
						else
							ipcInfo.ipcDevAddr = rep.wsdd__ProbeMatches->ProbeMatch->XAddrs;
						ipcInfo.ipcUuid = rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address;
						ipcInfo.ipcIp = ip;
						*/

						struct ProbeMatchNode *temProbe = (struct ProbeMatchNode *)malloc(sizeof(struct ProbeMatchNode));
						memset(temProbe, 0, sizeof(struct ProbeMatchNode));
						temProbe->ipcIp = 0;
						temProbe->ipcUuid = 0;
						temProbe->DeviceXAddr = 0;
						temProbe->next = 0;
						if (!temProbe)
							return -2;
						int Len = 0;
						int pos = substr(rep.wsdd__ProbeMatches->ProbeMatch->XAddrs, " ");
						if (pos > 0) {
							temProbe->DeviceXAddr = (char*)malloc(pos + 1);
							if (!temProbe->DeviceXAddr)
								return -2; //����ռ�ʧ��
							memcpy(temProbe->DeviceXAddr, rep.wsdd__ProbeMatches->ProbeMatch->XAddrs, pos);
							temProbe->DeviceXAddr[pos] = '\0';
						}
						else {
							Len = strlen(rep.wsdd__ProbeMatches->ProbeMatch->XAddrs);
							temProbe->DeviceXAddr = (char*)malloc(Len + 1);
							if (!temProbe->DeviceXAddr)
								return -2; //����ռ�ʧ��
							memcpy(temProbe->DeviceXAddr, rep.wsdd__ProbeMatches->ProbeMatch->XAddrs, Len + 1);
						}
						Len = strlen(rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
					
						temProbe->ipcUuid = (char*)malloc(Len + 1);
						if (!temProbe->ipcUuid)
							return -2; //����ռ�ʧ��
						memcpy(temProbe->ipcUuid, rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address, Len + 1);
						
						Len = strlen(ip);
						temProbe->ipcIp = (char*)malloc(Len + 1);
						if (!temProbe->ipcIp)
							return -2; //����ռ�ʧ��
						memcpy(temProbe->ipcIp, ip, Len + 1);
						
						temProbe->pre = temProbeMatchNode;
						temProbeMatchNode->next = temProbe;
						temProbe->next = NULL;
						temProbeMatchNode = temProbe;
					
					}
				}
				else {
					printf("ip:%s\n", ip);
					ret = -1;
				}
			}
		}
		else if (soap->error)
		{
			return soap->error;
		}
	}

	if (NULL != soap)
	{
		ONVIF_soap_delete(soap);
	}

	return ret;
}

int ONVIF_DetectDevice(struct ProbeMatchNode **ProbeMatchHead, int timeout)
{
	struct soap *soap = NULL;
	struct wsdd__ProbeType req;
	struct __wsdd__ProbeMatches rep;
	struct ProbeMatchNode *temProbeMatchNode = (struct ProbeMatchNode *)malloc(sizeof(struct ProbeMatchNode));
	temProbeMatchNode->pre = NULL;
	temProbeMatchNode->next = NULL;
	temProbeMatchNode->DeviceXAddr = NULL;
	*ProbeMatchHead = temProbeMatchNode;
	soap = ONVIF_soap_new(timeout);
	if (!soap)
		return -1;  //����soap����ʧ��

	ONVIF_init_header(soap);
	ONVIF_init_ProbeType(soap, &req);
	
	int result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, NULL, &req);
	
	while (SOAP_OK == result)
	{
		memset(&rep, 0x00, sizeof(rep));
		
		int a = soap_recv___wsdd__ProbeMatches(soap, &rep);
		
		if (SOAP_OK == a)
		{
			if (soap->error)
			{
				soap_perror(soap, "ProbeMatches");
			}
			else
			{
				dump__wsdd__ProbeMatches(&rep);
				if (NULL != rep.wsdd__ProbeMatches)
				{
					int i = 0;
		
					for ( ; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++)
					{

						struct ProbeMatchNode *temProbe = (struct ProbeMatchNode *)malloc(sizeof(struct ProbeMatchNode));
						memset(temProbe, 0, sizeof(struct ProbeMatchNode));
						if (!temProbe)
							return -2;
						int Len = strlen((rep.wsdd__ProbeMatches->ProbeMatch + i)->XAddrs);
						temProbe->DeviceXAddr = (char*)malloc(Len + 1);
						if (!temProbe->DeviceXAddr)
							return -2; //����ռ�ʧ��
						memcpy(temProbe->DeviceXAddr, (rep.wsdd__ProbeMatches->ProbeMatch + i)->XAddrs,Len+1);
						temProbe->pre = temProbeMatchNode;
						temProbeMatchNode->next = temProbe;
						temProbe->next = NULL;
						temProbeMatchNode = temProbe;	

					}
				}
			}
		}
		else if (soap->error)
		{
			break;
		}
	}
	
	if (NULL != soap)
	{
		ONVIF_soap_delete(soap);
	}
	
	return 0;
}


/************************************************************************
**������ONVIF_GetProfiles
**���ܣ���ȡ�豸������Ƶ����������Ϣ
**������
        [in] MediaXAddr - ý������ַ
        [out] profiles  - ���ص��豸����Ƶ����������Ϣ�б�������������ʹ��free�ͷŸû���
**���أ�
        �����豸��֧�ֵ�����������ͨ������/������������ʹprofiles�б����
**��ע��
        1). ע�⣺һ���������������������԰�����Ƶ����Ƶ���ݣ�Ҳ���Խ���������Ƶ���ݡ�
************************************************************************/
int ONVIF_GetProfiles(const char *MediaXAddr, struct tagProfile **profiles, const char *username, const char *password)
{
    int i = 0;
    int result = 0;
    struct soap *soap = NULL;
    struct _trt__GetProfiles            req;
    struct _trt__GetProfilesResponse    rep;

    SOAP_ASSERT(NULL != MediaXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	if (ONVIF_SetAuthInfo(soap, username, password))
		return -1;			// ��֤ʧ��

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    result = soap_call___trt__GetProfiles(soap, MediaXAddr, NULL, &req, &rep);
   // SOAP_CHECK_ERROR(result, soap, "GetProfiles");

    dump_trt__GetProfilesResponse(&rep);

    if (rep.__sizeProfiles > 0) {                                               // ���仺��
        (*profiles) = (struct tagProfile *)malloc(rep.__sizeProfiles * sizeof(struct tagProfile));
        SOAP_ASSERT(NULL != (*profiles));
        memset((*profiles), 0x00, rep.__sizeProfiles * sizeof(struct tagProfile));
    }

    for(i = 0; i < rep.__sizeProfiles; i++) {                                   // ��ȡ���������ļ���Ϣ�����������ĵģ�
        struct tt__Profile *ttProfile = &rep.Profiles[i];
        struct tagProfile *plst = &(*profiles)[i];

        if (NULL != ttProfile->token) {                                         // �����ļ�Token
            strncpy(plst->token, ttProfile->token, sizeof(plst->token) - 1);
        }

        if (NULL != ttProfile->VideoEncoderConfiguration) {                     // ��Ƶ������������Ϣ
            if (NULL != ttProfile->VideoEncoderConfiguration->token) {          // ��Ƶ������Token
                strncpy(plst->venc.token, ttProfile->VideoEncoderConfiguration->token, sizeof(plst->venc.token) - 1);
            }
            if (NULL != ttProfile->VideoEncoderConfiguration->Resolution) {     // ��Ƶ�������ֱ���
                plst->venc.Width  = ttProfile->VideoEncoderConfiguration->Resolution->Width;
                plst->venc.Height = ttProfile->VideoEncoderConfiguration->Resolution->Height;
            }
        }
    }

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return rep.__sizeProfiles;
}

/************************************************************************
**������ONVIF_GetCapabilities
**���ܣ���ȡ�豸������Ϣ
**������
        [in] DeviceXAddr - �豸�����ַ
        [out] capa       - �����豸������Ϣ��Ϣ
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
    1). ��������Ҫ�Ĳ���֮һ��ý������ַ
************************************************************************/
int ONVIF_GetCapabilities(const char *DeviceXAddr, struct tagCapabilities *capa, const char *username, const char *password, int *authFlag)
{
    int result = 0;
    struct soap *soap = NULL;
    struct _tds__GetCapabilities            req;
    struct _tds__GetCapabilitiesResponse    rep;

    SOAP_ASSERT(NULL != DeviceXAddr);
    SOAP_ASSERT(NULL != capa);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    *authFlag = ONVIF_SetAuthInfo(soap, username, password);

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    result = soap_call___tds__GetCapabilities(soap, DeviceXAddr, NULL, &req, &rep);
    SOAP_CHECK_ERROR(result, soap, "GetCapabilities");

    dump_tds__GetCapabilitiesResponse(&rep);

    memset(capa, 0x00, sizeof(struct tagCapabilities));
    if (NULL != rep.Capabilities) {
        if (NULL != rep.Capabilities->Media) {
            if (NULL != rep.Capabilities->Media->XAddr) {
                strncpy(capa->MediaXAddr, rep.Capabilities->Media->XAddr, sizeof(capa->MediaXAddr) - 1);
            }
        }
        if (NULL != rep.Capabilities->Events) {
            if (NULL != rep.Capabilities->Events->XAddr) {
                strncpy(capa->EventXAddr, rep.Capabilities->Events->XAddr, sizeof(capa->EventXAddr) - 1);
            }
        }
    }

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}

/************************************************************************
**������make_uri_withauth
**���ܣ����������֤��Ϣ��URI��ַ
**������
        [in]  src_uri       - δ����֤��Ϣ��URI��ַ
        [in]  username      - �û���
        [in]  password      - ����
        [out] dest_uri      - ���صĴ���֤��Ϣ��URI��ַ
        [in]  size_dest_uri - dest_uri�����С
**���أ�
        0�ɹ�����0ʧ��
**��ע��
    1). ���ӣ�
    ����֤��Ϣ��uri��rtsp://100.100.100.140:554/av0_0
    ����֤��Ϣ��uri��rtsp://username:password@100.100.100.140:554/av0_0
************************************************************************/
int make_uri_withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri)
{
    int result = 0;
    unsigned int needBufSize = 0;

    SOAP_ASSERT(NULL != src_uri);
    SOAP_ASSERT(NULL != username);
    SOAP_ASSERT(NULL != password);
    SOAP_ASSERT(NULL != dest_uri);
    memset(dest_uri, 0x00, size_dest_uri);

    needBufSize = strlen(src_uri) + strlen(username) + strlen(password) + 3;    // ��黺���Ƿ��㹻��������:���͡�@�����ַ���������
    if (size_dest_uri < needBufSize) {
        SOAP_DBGERR("dest uri buf size is not enough.\n");
        result = -1;
        goto EXIT;
    }

    if (0 == strlen(username) && 0 == strlen(password)) {                       // �����µ�uri��ַ
        strcpy(dest_uri, src_uri);
    } else {
        char *p = strstr(src_uri, "//");
        if (NULL == p) {
            SOAP_DBGERR("can't found '//', src uri is: %s.\n", src_uri);
            result = -1;
            goto EXIT;
        }
        p += 2;

        memcpy(dest_uri, src_uri, p - src_uri);
        sprintf(dest_uri + strlen(dest_uri), "%s:%s@", username, password);
        strcat(dest_uri, p);
    }

EXIT:

    return result;
}

void Free_ProbeMatchNode(struct ProbeMatchNode *head)
{
	if (head == NULL)
		return;

	while (head != NULL)
	{
		struct ProbeMatchNode *temp = NULL;
		temp = head;
		head = head->next;
		if (temp->DeviceXAddr)
		{
			free(temp->DeviceXAddr);
			temp->DeviceXAddr = NULL;
		}
		if (temp->ipcIp)
		{
			free(temp->ipcIp);
			temp->ipcIp = NULL;
		}
		if (temp->ipcUuid)
		{
			free(temp->ipcUuid);
			temp->ipcUuid = NULL;
		}
		free(temp);
		temp = NULL;
	}
}
