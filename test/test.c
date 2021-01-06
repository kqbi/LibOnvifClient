#include "LibOnvifClientAPI.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	//struct ProbeMatchNode *head = NULL;
//	ONVIF_ClientDetectDevice(&head,10);
//	printf("get Device total %d\n", ClientGetDeviceCount(head));

//	struct ProbeMatchNode *temp = head->next;
    
	//while(temp)
	//{
        char rtsp[256] = { 0 };
		char* ip = "192.168.2.117";
        printf("%s\n",ip);
        ONVIF_ClientGetStreamUri(ip, "admin", "admin123", rtsp, 2);
        printf("ÊÓÆµÁ÷rtsp = %s\n", rtsp);
        ONVIF_ClientSnapshotUri(ip, "admin", "admin123", rtsp, 2);
        printf("×¥ÅÄrtsp = %s\n", rtsp);
		//temp = temp->next;
	//}

//	FreeProbeMatchList(head);
	
	return 0;
}	
