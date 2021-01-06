#ifndef LIBONVIFCLIENTAPI_H
#define LIBONVIFCLIENTAPI_H

#ifdef __cplusplus
extern "C" {
#endif	
	//此结构体用于获取设备地址
	struct ProbeMatchNode
	{
	   struct ProbeMatchNode *pre;
	   struct ProbeMatchNode *next;
	   char* DeviceXAddr;
	   char* ipcIp;
	   char* ipcUuid;
	};

	//探测固定ip的设备，ProbeMatchHead用于获取设备地址，头结点置空，timout为等待设备返回信息的超时时间
	int ONVIF_ClientDetectDeviceCertainIp(const char *ip, int timeout, struct ProbeMatchNode **ProbeMatchHead);

	//探测设备，ProbeMatchHead用于获取设备地址，头结点置空，timout为等待设备返回信息的超时时间
	int ONVIF_ClientDetectDevice(struct ProbeMatchNode **ProbeMatchHead, int timeout);

	//获得探测到的设备个数,即链表长度，不包含头结点
	int ClientGetDeviceCount(struct ProbeMatchNode *ProbeMatchHead);

	//获取rtsp流的地址，有的设备需要用户名和密码才能获得，有的不需要，可置空
	int ONVIF_ClientGetStreamUri(const char *DeviceXAddr, const char *username, const char *password, char rtspUri[256], int timeout);

	//获取设备的抓拍地址
	int ONVIF_ClientSnapshotUri(const char *DeviceXAddr, const char *username, const char *password, char SnapshotUri[256], int timeout);

	//释放探测设备生成的链表
	void FreeProbeMatchList(struct ProbeMatchNode *ProbeMatchHead);

	//生成带验证的Uri
	int Make_Uri_Withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri);

#ifdef __cplusplus
}
#endif

#endif
