#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "onvif/soapH.h"
#include "onvif/wsaapi.h"
#include "LibOnvifClientAPI.h"

#ifndef __ONVIF_COMM_H__
#define __ONVIF_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define bool int
#define true 1
#define false 0

#ifndef DIM
#define DIM(array)  (sizeof(array) / sizeof(array[0]))
#endif

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define SOAP_ASSERT     assert
#define SOAP_DBGLOG    
#define SOAP_DBGERR     

#define USERNAME        "admin"                                                 // 认证信息（用户名、密码）
#define PASSWORD        "admin123"

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单秒秒）

#define ONVIF_ADDRESS_SIZE   (128)                                              // URI地址长度
#define ONVIF_TOKEN_SIZE     (65)                                               // token长度

/* 视频编码器配置信息 */
struct tagVideoEncoderConfiguration
{
    char token[ONVIF_TOKEN_SIZE];                                               // 唯一标识该视频编码器的令牌字符串
    int Width;                                                                  // 分辨率
    int Height;
};

/* 设备配置信息 */
struct tagProfile {
    char token[ONVIF_TOKEN_SIZE];                                               // 唯一标识设备配置文件的令牌字符串

    struct tagVideoEncoderConfiguration venc;                                   // 视频编码器配置信息
};

/* 设备能力信息 */
struct tagCapabilities {
    char MediaXAddr[ONVIF_ADDRESS_SIZE];                                        // 媒体服务地址
    char EventXAddr[ONVIF_ADDRESS_SIZE];                                        // 事件服务地址
                                                                                // 其他服务器地址就不列出了
};

#define SOAP_CHECK_ERROR(result, soap, str) \
    do { \
        if (SOAP_OK != (result) || SOAP_OK != (soap)->error) { \
            soap_perror((soap), (str)); \
            if (SOAP_OK == (result)) { \
                (result) = (soap)->error; \
            } \
            goto EXIT; \
        } \
    } while (0)


void soap_perror(struct soap *soap, const char *str);
void* ONVIF_soap_malloc(struct soap *soap, unsigned int n);
struct soap* ONVIF_soap_new(int timeout);
void ONVIF_soap_delete(struct soap *soap);

int ONVIF_SetAuthInfo(struct soap *soap, const char *username, const char *password);
void ONVIF_init_header(struct soap *soap);
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe);
int ONVIF_DetectDevice(struct ProbeMatchNode **PorbeMatch, int timeout);
int ONVIF_DetectDeviceCertainIp(const char *ip, int timeout, struct ProbeMatchNode **ProbeMatchHead);

int ONVIF_GetCapabilities(const char *DeviceXAddr, struct tagCapabilities *capa, const char* username, const char *password,int *authFlag, int timeout);
int ONVIF_GetProfiles(const char *MediaXAddr, struct tagProfile **profiles, const char *username, const char *password, int timeout);
int make_uri_withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri);

void Free_ProbeMatchNode(struct ProbeMatchNode *head);


#ifdef __cplusplus
}
#endif

#endif
