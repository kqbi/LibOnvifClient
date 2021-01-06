#ifndef LIBONVIFCLIENTAPI_H
#define LIBONVIFCLIENTAPI_H

#ifdef __cplusplus
extern "C" {
#endif	
	//�˽ṹ�����ڻ�ȡ�豸��ַ
	struct ProbeMatchNode
	{
	   struct ProbeMatchNode *pre;
	   struct ProbeMatchNode *next;
	   char* DeviceXAddr;
	   char* ipcIp;
	   char* ipcUuid;
	};

	//̽��̶�ip���豸��ProbeMatchHead���ڻ�ȡ�豸��ַ��ͷ����ÿգ�timoutΪ�ȴ��豸������Ϣ�ĳ�ʱʱ��
	int ONVIF_ClientDetectDeviceCertainIp(const char *ip, int timeout, struct ProbeMatchNode **ProbeMatchHead);

	//̽���豸��ProbeMatchHead���ڻ�ȡ�豸��ַ��ͷ����ÿգ�timoutΪ�ȴ��豸������Ϣ�ĳ�ʱʱ��
	int ONVIF_ClientDetectDevice(struct ProbeMatchNode **ProbeMatchHead, int timeout);

	//���̽�⵽���豸����,�������ȣ�������ͷ���
	int ClientGetDeviceCount(struct ProbeMatchNode *ProbeMatchHead);

	//��ȡrtsp���ĵ�ַ���е��豸��Ҫ�û�����������ܻ�ã��еĲ���Ҫ�����ÿ�
	int ONVIF_ClientGetStreamUri(const char *DeviceXAddr, const char *username, const char *password, char rtspUri[256], int timeout);

	//��ȡ�豸��ץ�ĵ�ַ
	int ONVIF_ClientSnapshotUri(const char *DeviceXAddr, const char *username, const char *password, char SnapshotUri[256], int timeout);

	//�ͷ�̽���豸���ɵ�����
	void FreeProbeMatchList(struct ProbeMatchNode *ProbeMatchHead);

	//���ɴ���֤��Uri
	int Make_Uri_Withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri);

#ifdef __cplusplus
}
#endif

#endif
