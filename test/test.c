#include "LibOnvifClientAPI.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	struct ProbeMatchNode *head = NULL;
	ONVIF_ClientDetectDevice(&head,10);
	printf("get Device total %d\n", ClientGetDeviceCount(head));

	struct ProbeMatchNode *temp = head->next;
    
	while(temp)
	{
        char rtsp[256] = { 0 };
        printf("%s\n",temp->DeviceXAddr);
        ONVIF_ClientGetStreamUri(temp->DeviceXAddr, "admin", "admin123", rtsp);
        printf("��Ƶ��rtsp = %s\n", rtsp);
        ONVIF_ClientSnapshotUri(temp->DeviceXAddr, "admin", "admin123", rtsp);
        printf("ץ��rtsp = %s\n", rtsp);
		temp = temp->next;
	}

	FreeProbeMatchList(head);
	
	return 0;
}	
