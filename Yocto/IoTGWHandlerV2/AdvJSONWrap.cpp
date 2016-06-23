#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AdvJSON.h"
#include "MqttHal.h"

#ifdef __cplusplus
extern "C" {
#endif
extern char            g_GWInfMAC[MAX_MACADDRESS_LEN];
struct node* g_pVirtualGatewayDataListHead=NULL;

struct node * GetVirtualGatewayDataListNode(char* devID, int devType);
int DeleteVirtualGatewayDataListNode(char* devID, int devType);
void AddVirtualGatewayDataListNode(char* pVirtualGatewayDevID, char* pConnectivityType, char* pConnectivityDevID, char* pConnectivityInfo, int iConnectivityInfoSize, int devType, int iOSInfo, char* pSensorHubDevID);
int UpdateVirtualGatewayDataListNode(char* data);
void UpdateConnectivitySensorHubListNode(const char* data);
void UpdateVirtualGatewayOSInfoToDataListNode(char* data, int iOSInfo);
int PackConnectivityCapability(char* info_data);
int PackConnectivityInfo(char* data, char* info_data, int osInfo);
int GetOSInfoType(char* data);
void aTest(const char* mydata);
void printNodeInfo();

#ifdef __cplusplus
}
#endif

void printNodeInfo(struct node * r){

    printf("-------------------node info------------------------------\n");
    printf("[%s][%s] node type=%d, os info=%d\n", __FILE__, __func__, r->nodeType, r->virtualGatewayOSInfo);
    printf("[%s][%s] virtualGatewayDevID=%s\n", __FILE__, __func__, r->virtualGatewayDevID);
    printf("[%s][%s] connectivityDevID=%s\n", __FILE__, __func__, r->connectivityDevID);
    printf("[%s][%s] sensorHubDevID=%s\n", __FILE__, __func__, r->sensorHubDevID);
     printf("-------------------------------------------------\n");
}

struct node * GetVirtualGatewayDataListNode(char* devID, int devType)
{
    struct node * r;
    r=g_pVirtualGatewayDataListHead;

    if(r==NULL)
    {
    return NULL;
    }
    while(r!=NULL)
    {
        switch(devType){
            case TYPE_GATEWAY:
            {

                if (strcmp(r->virtualGatewayDevID, devID) == 0){
                    if ( r->nodeType == TYPE_GATEWAY) {
                        return r;
                    }
                }

                break;
            }
            case TYPE_CONNECTIVITY:
            {
                if (strcmp(r->connectivityDevID, devID) == 0){
                    if ( r->nodeType == TYPE_CONNECTIVITY) {
                        return r;
                    }
                }
                break;
            }
            case TYPE_SENSOR_HUB:
            {
                if (strcmp(r->sensorHubDevID, devID) == 0){
                    if ( r->nodeType == TYPE_SENSOR_HUB) {
                        return r;
                    }
                }
                break;
            }
            default:
                printf("[%s][%s]Unknown device type\n", __FILE__, __func__);
                break;
        }
        r=r->next;
    }

    return NULL;
    //printf("\n");
}

int DeleteVirtualGatewayDataListNode(char* devID, int devType)
{
    struct node *temp, *prev;
    temp=g_pVirtualGatewayDataListHead;
    char tmp_devID[32]={0};

    while(temp!=NULL)
    {
        switch(devType){
            case TYPE_GATEWAY:
            {
                strcpy(tmp_devID, temp->virtualGatewayDevID);
                break;
            }
            case TYPE_CONNECTIVITY:
            {
                strcpy(tmp_devID, temp->connectivityDevID);
                break;
            }
            case TYPE_SENSOR_HUB:
            {
                strcpy(tmp_devID, temp->sensorHubDevID);
                break;
            }
            default:
                printf("[%s][%s]Unknown device type\n", __FILE__, __func__);
                break;
        }
        //
        if( strcmp(tmp_devID,devID) == 0 && temp->nodeType == devType)
        {
            if(temp==g_pVirtualGatewayDataListHead)
            {
                g_pVirtualGatewayDataListHead=temp->next;
                if (temp->connectivityInfo){
                    free(temp->connectivityInfo);
                }
                free(temp);
                return 1;
            }
            else
            {
                prev->next=temp->next;
                if (temp->connectivityInfo){
                    free(temp->connectivityInfo);
                }
                free(temp);
                return 1;
            }
        }
        else{
            prev=temp;
            temp= temp->next;
        }
    }

    return 0;
}
void UpdateVirtualGatewayOSInfoToDataListNode(char* data, int iOSInfo){

    printf("UpdateVirtualGatewayOSInfoToDataListNode\n");
    struct node *temp=NULL;
    char virtualGatewayDevID[MAX_DEVICE_ID_LEN]={0};
    
    AdvJSON json(data);

    //Get virtual gateway devID
    strcpy(virtualGatewayDevID,json["susiCommData"]["agentID"].Value().c_str());
    printf("virtualGatewayDevID = %s\n", virtualGatewayDevID);

    temp= GetVirtualGatewayDataListNode(virtualGatewayDevID,TYPE_GATEWAY);
    if ( temp ){
         printf("Update all Nodes OS info = %d\n", temp->virtualGatewayOSInfo);
        //Update all Nodes which have this gateway device ID
      
        //Get virtual gateway info
    }
    else{
        printf("Add Node to record OS info = %d\n",iOSInfo);
        //Add Node to record os info
        AddVirtualGatewayDataListNode(virtualGatewayDevID,NULL,NULL,NULL, 0, TYPE_GATEWAY, iOSInfo, NULL);
    }

}

void AddVirtualGatewayDataListNode(char* pVirtualGatewayDevID, char* pConnectivityType, char* pConnectivityDevID, char* pConnectivityInfo, int iConnectivityInfoSize, int devType, int iOSInfo, char* pSensorHubDevID)
{
    struct node *temp=NULL;
    char tmp_devID[32]={0};
    
    switch(devType)
    {
        case TYPE_GATEWAY:
        {
            if (pVirtualGatewayDevID != NULL){
                strcpy(tmp_devID,pVirtualGatewayDevID);
            }
            break;
        }
        case TYPE_CONNECTIVITY:
        {
            if (pConnectivityDevID != NULL){
                strcpy(tmp_devID,pConnectivityDevID);
            }
            break;
        }
        case TYPE_SENSOR_HUB:
        {
            if  ( pSensorHubDevID != NULL){
                strcpy(tmp_devID,pSensorHubDevID);
            }
            break;
        }
        default:
            printf("[%s][%s]Unknown device type\n", __FILE__, __func__);
            break;
    }
    //
    temp=GetVirtualGatewayDataListNode(tmp_devID, devType);
    //
    if (temp != NULL){
        DeleteVirtualGatewayDataListNode(tmp_devID, devType);
        temp=NULL;
    }
    //
    temp=(struct node *)malloc(sizeof(struct node));
    memset(temp,0,sizeof(struct node));
 
    temp->nodeType = devType;
    temp->virtualGatewayOSInfo = iOSInfo;

    if (pConnectivityInfo != NULL){
        temp->connectivityInfo=(char*)malloc(iConnectivityInfoSize+1);
        strcpy(temp->connectivityInfo,pConnectivityInfo);
    }

    if (pConnectivityDevID != NULL){
        strcpy(temp->connectivityDevID,pConnectivityDevID);
    }
   
    if ( pVirtualGatewayDevID != NULL){
        strcpy(temp->virtualGatewayDevID, pVirtualGatewayDevID);
    }

    if  ( pSensorHubDevID != NULL){
        strcpy(temp->sensorHubDevID, pSensorHubDevID);
    }
    
    if ( pConnectivityType != NULL){
        strcpy(temp->connectivityType,pConnectivityType);
    }

    //temp->data=num;
    if (g_pVirtualGatewayDataListHead == NULL)
    {
        g_pVirtualGatewayDataListHead=temp;
        g_pVirtualGatewayDataListHead->next=NULL;
    }
    else
    {
        temp->next=g_pVirtualGatewayDataListHead;
        g_pVirtualGatewayDataListHead=temp;
    }

}

int GetSensorHubList(char* sensorHubList, int osInfo, char* connectivityDevID){
    struct node * r;
    r=g_pVirtualGatewayDataListHead;

    if(r==NULL)
    {
        return -1;
    }

    while(r!=NULL)
    {
        switch (osInfo)
        {
            case OS_IP_BASE:
            {
                if( r->virtualGatewayOSInfo == OS_IP_BASE &&
                    r->nodeType == TYPE_SENSOR_HUB ){

                    if ( strlen(r->sensorHubDevID) ==0 || strcmp(r->sensorHubDevID,"NULL") == 0){
                        printNodeInfo(r);
                    }
                    else{
		        if (strlen(sensorHubList) == 0){
				strcat(sensorHubList,r->sensorHubDevID);
			}
			else{
				strcat(sensorHubList,",");
				strcat(sensorHubList,r->sensorHubDevID);
			}
                    }
                }
                break;
            }
            case OS_NONE_IP_BASE:
            {
                break;
            }
            default:
                printf("[%s][%s] Unknown os info\n",__FILE__, __func__);
                break;
        } 
        r=r->next;
    }

    return 0;
}

int PackIPBaseConnectivityInfo(char* info_data){

    char* e_array[]={"{\"n\":\"SenHubList\",\"sv\":\"%s\"}",
                     "{\"n\":\"Neighbor\",\"sv\":\"%s\"}",
                     "{\"n\":\"Name\",\"sv\":\"Ethernet\"}"
                    };
    int i=0;
    int max_e_array_size=sizeof(e_array)/sizeof(char*);
    char sensorHubList[1024]={0};
    char tmp[1024]={0};

    if ( GetSensorHubList(sensorHubList, OS_IP_BASE, NULL) < 0 ){
        printf("[%s][%s] get sensor hub list fail\n", __FILE__, __func__);
        return -1;
    }

    strcat(info_data,"{\"IoTGW\":{\"Ethernet\":{\"Ethernet0\":");
    strcat(info_data,"{\"Info\":");
    //
    strcat(info_data,"{");
    //
    strcat(info_data,"\"e\":[");
#if 1
    for(i=0; i < max_e_array_size ; i++){
        memset(tmp,0,sizeof(tmp));
        AdvJSON json(e_array[i]);

        if ( strcmp("SenHubList", json[0].Value().c_str()) == 0){
            if ( strlen(sensorHubList) == 0){
                strcpy(tmp,"{\"n\":\"SenHubList\",\"sv\":\"\"}");
            }
            else{
                sprintf(tmp,e_array[i],sensorHubList);
            }
        }
        else if ( strcmp("Neighbor", json[0].Value().c_str()) == 0){
            if ( strlen(sensorHubList) == 0){
                strcpy(tmp,"{\"n\":\"Neighbor\",\"sv\":\"\"}");
            }
            else{
                sprintf(tmp,e_array[i],sensorHubList);
            };
        }
        else{
            strcpy(tmp,e_array[i]);
        }
        strcat(info_data,tmp);
        strcat(info_data,",");
    }
    int len=strlen(info_data);
    info_data[len-1]=0;
#endif
    strcat(info_data,"]");
    //
    strcat(info_data,",");
    strcat(info_data,"\"bn\":\"Info\"");
    //
    strcat(info_data,"}");
    //
    strcat(info_data,",");
    sprintf(tmp,"\"bn\":\"0007%s\"",g_GWInfMAC);
    strcat(info_data,tmp);
    strcat(info_data,",");
    strcat(info_data,"\"ver\":1");
    strcat(info_data,"}");
    strcat(info_data,"}}}");

    return 0;

}

int PackNoneIPBaseConnectivityInfo(char* data, char* info_data){

    char nodeContent[MAX_JSON_NODE_SIZE]={0};

    AdvJSON json(data);

    strcpy(info_data,json["susiCommData"]["data"].Value().c_str());
    if(strcmp(info_data, "NULL") == 0){
        return -1;
    }

    return 0;
}


int PackConnectivityInfo(char* data, char* info_data, int osInfo){

    switch(osInfo){
        case OS_IP_BASE:
        {
            return PackIPBaseConnectivityInfo(info_data);
        }
        case OS_NONE_IP_BASE:
        {
            return PackNoneIPBaseConnectivityInfo(data, info_data);
        }
        default:
        {
            printf("[%s][%s] unknown os info type:%d\n", __FILE__, __func__, osInfo);
            return -1;
        }
    }
#if 0
    char* e_array[]={"{\"n\":\"SenHubList\",\"sv\":\"%s\"}",
                     "{\"n\":\"Neighbor\",\"sv\":\"%s\"}",
                     "{\"n\":\"Name\",\"sv\":\"Ethernet\"}"
                    };
    int i=0;
    int max_e_array_size=sizeof(e_array)/sizeof(char*);
    char sensorHubList[1024]={0};
    char tmp[1024]={0};

    if ( GetSensorHubList(sensorHubList, OS_IP_BASE, NULL) < 0 ){
        printf("[%s][%s] get sensor hub list fail\n", __FILE__, __func__);
        return -1;
    }

    strcat(info_data,"{\"Info\":");
    //
    strcat(info_data,"{");
    //
    strcat(info_data,"\"e\":[");
#if 1
    for(i=0; i < max_e_array_size ; i++){
        memset(tmp,0,sizeof(tmp));
        AdvJSON json(e_array[i]);

        if ( strcmp("SenHubList", json[0].Value().c_str()) == 0){
            if ( strlen(sensorHubList) == 0){
                strcpy(tmp,"{\"n\":\"SenHubList\",\"sv\":\"\"}");
            }
            else{
                sprintf(tmp,e_array[i],sensorHubList);
            }
        }
        else if ( strcmp("Neighbor", json[0].Value().c_str()) == 0){
            if ( strlen(sensorHubList) == 0){
                strcpy(tmp,"{\"n\":\"Neighbor\",\"sv\":\"\"}");
            }
            else{
                sprintf(tmp,e_array[i],sensorHubList);
            };
        }
        else{
            strcpy(tmp,e_array[i]);
        }
        strcat(info_data,tmp);
        strcat(info_data,",");
    }
    int len=strlen(info_data);
    info_data[len-1]=0;
#endif
    strcat(info_data,"]");
    //
    strcat(info_data,",");
    strcat(info_data,"\"bn\":\"Info\"");
    //
    strcat(info_data,"}");
    //
    strcat(info_data,",");
    sprintf(tmp,"\"bn\":\"0007%s\"",g_GWInfMAC);
    strcat(info_data,tmp);
    strcat(info_data,",");
    strcat(info_data,"\"ver\":1");
    strcat(info_data,"}");
#endif
    return 0;
}

int PackConnectivityCapability(char* info_data){

    char* e_array[]={"{\"n\":\"SenHubList\",\"sv\":\"%s\",\"asm\":\"%s\"}",
                     "{\"n\":\"Neighbor\",\"sv\":\"%s\",\"asm\":\"%s\"}",
                     "{\"n\":\"Name\",\"sv\":\"Ethernet\",\"asm\":\"r\"}"
                    };
    char tmp[1024]={0};
    int i=0;
    int max_e_array_size=sizeof(e_array)/sizeof(char*);

    strcat(info_data,"{\"Info\":");
    //
    strcat(info_data,"{");
    //
    strcat(info_data,"\"e\":[");
#if 1
    for(i=0; i < max_e_array_size ; i++){
        memset(tmp,0,sizeof(tmp));
        AdvJSON json(e_array[i]);
        if ( strcmp("SenHubList", json[0].Value().c_str()) == 0){
            //sprintf(tmp,e_array[i],"\"\"","r");
            strcpy(tmp,"{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"}");
        }
        else if ( strcmp("Neighbor", json[0].Value().c_str()) == 0){
            //sprintf(tmp,e_array[i],"3333","r");
            strcpy(tmp,"{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"}");
        }
        else{
            strcpy(tmp,e_array[i]);
        }
        strcat(info_data,tmp);
        strcat(info_data,",");
    }
    int len=strlen(info_data);
    info_data[len-1]=0;
#endif
    strcat(info_data,"]");
    //
    strcat(info_data,",");
    strcat(info_data,"\"bn\":\"Info\"");
    //
    strcat(info_data,"}");
    //
    strcat(info_data,",");
    sprintf(tmp,"\"bn\":\"0007%s\"",g_GWInfMAC);
    strcat(info_data,tmp);
    strcat(info_data,",");
    strcat(info_data,"\"ver\":1");
    strcat(info_data,"}");

    return 0;
}

int GetOSInfoType(char* data){

    char virtualGatewayDevID[MAX_DEVICE_ID_LEN]={0};
    int osInfo=OS_TYPE_UNKNOWN;

    AdvJSON json(data);
    strcpy(virtualGatewayDevID,json["susiCommData"]["agentID"].Value().c_str());
    

    struct node* temp= GetVirtualGatewayDataListNode(virtualGatewayDevID,TYPE_GATEWAY);
    if ( temp != NULL ){
        osInfo=temp->virtualGatewayOSInfo;
    }
      
    printf("[%s][%s] virtualGatewayDevID=%s, os info type:%d\n", __FILE__, __func__, virtualGatewayDevID, osInfo);
    return osInfo;
}

int UpdateVirtualGatewayDataListNode(char* data){

#if 0
//sample message:
/*
//Only WSN
{"susiCommData":{"infoSpec":{"IoTGW":{"WSN":{"WSN0":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD01","ver":1},"WSN1":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD02","ver":1},"bn":"WSN","ver":1},"ver":1}},"commCmd":2052,"requestID":2001,"agentID":"0000000E40ABCDEF","handlerName":"general","sendTS":160081020}}

//WSN and BLE
{"susiCommData":{"infoSpec":{"IoTGW":{"WSN":{"WSN0":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD01","ver":1},"WSN1":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD02","ver":1},"bn":"WSN","ver":1},"BLE":{"BLE0":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD05","ver":1},"BLE1":{"Info":{"e":[{"n":"SenHubList","sv":"","asm":"r"},{"n":"Neighbor","sv":"","asm":"r"},{"n":"Name","sv":"WSN0","asm":"r"},{"n":"Health","v":"100.000000","asm":"r"},{"n":"sw","sv":"1.2.1.12","asm":"r"},{"n":"reset","bv":"0","asm":"rw"}],"bn":"Info"},"bn":"0007000E40ABCD06","ver":1},"bn":"BLE","ver":1},"ver":1}},"commCmd":2052,"requestID":2001,"agentID":"0000000E40ABCDEF","handlerName":"general","sendTS":160081020}}

*/
/*
    char mydata[1024]={"{\"susiCommData\":{\"infoSpec\":{\"IoTGW\":{\"WSN\":{\"WSN0\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD01\",\"ver\":1},\"WSN1\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD02\",\"ver\":1},\"bn\":\"WSN\",\"ver\":1},\"ver\":1}},\"commCmd\":2052,\"requestID\":2001,\"agentID\":\"0000000E40ABCDEF\",\"handlerName\":\"general\",\"sendTS\":160081020}"};
*/
    char mydata[2024]={"{\"susiCommData\":{\"infoSpec\":{\"IoTGW\":{\"WSN\":{\"WSN0\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD01\",\"ver\":1},\"WSN1\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD02\",\"ver\":1},\"bn\":\"WSN\",\"ver\":1},\"BLE\":{\"BLE0\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD05\",\"ver\":1},\"BLE1\":{\"Info\":{\"e\":[{\"n\":\"SenHubList\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Neighbor\",\"sv\":\"\",\"asm\":\"r\"},{\"n\":\"Name\",\"sv\":\"WSN0\",\"asm\":\"r\"},{\"n\":\"Health\",\"v\":\"100.000000\",\"asm\":\"r\"},{\"n\":\"sw\",\"sv\":\"1.2.1.12\",\"asm\":\"r\"},{\"n\":\"reset\",\"bv\":\"0\",\"asm\":\"rw\"}],\"bn\":\"Info\"},\"bn\":\"0007000E40ABCD06\",\"ver\":1},\"bn\":\"BLE\",\"ver\":1},\"ver\":1}},\"commCmd\":2052,\"requestID\":2001,\"agentID\":\"0000000E40ABCDEF\",\"handlerName\":\"general\",\"sendTS\":160081020}}"};
    
#endif

    AdvJSON json(data);
    int i=0, j=0;
    int max=0;
    char virtualGatewayDevID[MAX_DEVICE_ID_LEN]={0};
    char connectivityInfo[MAX_JSON_NODE_SIZE]={0};
    char connectivityDevID[MAX_DEVICE_ID_LEN]={0};

    //Get virtual gateway devID
    strcpy(virtualGatewayDevID,json["susiCommData"]["agentID"].Value().c_str());
    //printf(virtualGatewayDevID);

    //char a[128]={"susiCommData"};
    max=json["susiCommData"]["infoSpec"]["IoTGW"].Size();
    for(i=0; i < max ;i++){
#if 0
        printf("json[\"susiCommData\"][\"infoSpec\"][\"IoTGW\"][%d]=%s\n",i,json["susiCommData"]["infoSpec"]["IoTGW"][i].Key().c_str());
#endif
        char type[128]={0};
        char device[128]={0};

        strcpy(type,json["susiCommData"]["infoSpec"]["IoTGW"][i].Key().c_str());
#if 0
        printf("json[\"susiCommData\"][\"infoSpec\"][\"IoTGW\"][%s]=%s\n",\
        type,
        json["susiCommData"]["infoSpec"]["IoTGW"][type].Value().c_str());
#endif        
        int max_device=json["susiCommData"]["infoSpec"]["IoTGW"][type].Size();
        for(j=0; j<max_device ; j++){
            strcpy(device,json["susiCommData"]["infoSpec"]["IoTGW"][type][j].Key().c_str());
            //int tmp_value[1024]={0};
            if ( strcmp("NULL",json["susiCommData"]["infoSpec"]["IoTGW"][type][device]["Info"].Value().c_str()) != 0){
#if 0
                printf("@@@@ json[\"susiCommData\"][\"infoSpec\"][\"IoTGW\"][%s][%s]=%s\n",\
                type,device, \
                json["susiCommData"]["infoSpec"]["IoTGW"][type][device].Value().c_str());
#endif
                //Get connectivity Info
                strcpy(connectivityInfo,json["susiCommData"]["infoSpec"]["IoTGW"][type][device].Value().c_str());
                //Get connectivity device ID
                strcpy(connectivityDevID,json["susiCommData"]["infoSpec"]["IoTGW"][type][device]["bn"].Value().c_str());
#if 1
		printf("********************************************\n");
		printf("virtualGateway DevID: "); printf(virtualGatewayDevID); printf("\n");
		printf("connectivity Info: "); printf(connectivityInfo); printf("\n");
		printf("connectivity DevID: "); printf(connectivityDevID);
		printf("\n********************************************\n");
#endif
                int osInfo=OS_TYPE_UNKNOWN;
                struct node* temp= GetVirtualGatewayDataListNode(virtualGatewayDevID,TYPE_GATEWAY);
                if ( temp != NULL ){
                    osInfo = temp->virtualGatewayOSInfo;
                }
                //Add Node
#if 1
                if ( osInfo == OS_IP_BASE){
                    //sprintf(connectivityDevID,"0007%s",g_GWInfMAC);
                    char info_data[1024]={0};
                    PackConnectivityCapability(info_data);
                    strcpy(connectivityInfo,info_data); 
                }
#endif
                AddVirtualGatewayDataListNode(virtualGatewayDevID,type,connectivityDevID,connectivityInfo, strlen(connectivityInfo), TYPE_CONNECTIVITY, osInfo, NULL);
            }
        }

    }
    
    return 0;
}

void UpdateConnectivitySensorHubListNode(const char* data){

    AdvJSON json(data);
    int i=0, j=0, k=0;
    int max=0;
    int osInfo=OS_TYPE_UNKNOWN;
    char virtualGatewayDevID[MAX_DEVICE_ID_LEN]={0};
    char connectivityInfo[MAX_JSON_NODE_SIZE]={0};
    char connectivityDevID[MAX_DEVICE_ID_LEN]={0};
    char virtualConnectivityDevID[MAX_DEVICE_ID_LEN]={0};
    char packet_type[32]={"data"};
    struct node *temp=NULL;

    //Get virtual gateway devID
    strcpy(virtualGatewayDevID,json["susiCommData"]["agentID"].Value().c_str());
    //check OS type
    temp=GetVirtualGatewayDataListNode(virtualGatewayDevID, TYPE_GATEWAY);
    if (temp){
        osInfo = temp->virtualGatewayOSInfo;
    }

#if 0
    if ( temp->virtualGatewayOSInfo == OS_IP_BASE){
        printf("[%s][%s] IP base device, DO NOT update gateway Info\n", __FILE__, __func__);
        return;
    }
#endif
    //

    AdvJSON IoTGW_json=json["susiCommData"][packet_type]["IoTGW"];
    max=IoTGW_json.Size();

    for(i=0; i < max ;i++){
        char type[128]={0};
        char device[128]={0};

        strcpy(type,IoTGW_json[i].Key().c_str());
 
        int max_device=IoTGW_json[type].Size();

        for(j=0; j<max_device ; j++){
            strcpy(device,IoTGW_json[type][j].Key().c_str());

            if ( strcmp("NULL",IoTGW_json[type][device]["Info"].Value().c_str()) != 0){

                //Get connectivity Info
                strcpy(connectivityInfo,IoTGW_json[type][device].Value().c_str());
                //Get connectivity device ID
                strcpy(connectivityDevID,IoTGW_json[type][device]["bn"].Value().c_str());
#if 1
		printf("********************************************\n");
		printf("virtualGateway DevID: "); printf(virtualGatewayDevID); printf("\n");
                printf("os Info: %d\n", osInfo);
		printf("connectivity Info: "); printf(connectivityInfo); printf("\n");
		printf("connectivity DevID: "); printf(connectivityDevID);
		printf("\n********************************************\n");
#endif
		
#if 1
                //get connectivity node
                if( OS_NONE_IP_BASE == osInfo){
		    temp=GetVirtualGatewayDataListNode(connectivityDevID, TYPE_CONNECTIVITY);
		    if ( temp == NULL ){
			printf("[%s][%s]can not find connectivity:%s node\n", __FILE__, __func__, connectivityDevID);
			continue;
		    }
                }
#endif

                AdvJSON IoTGW_device_info=IoTGW_json[type][device]["Info"]["e"];
                int max_info_size=IoTGW_device_info.Size();

                for(k=0; k < max_info_size; k++){
                    int l=0;
                    int max_dev_info_size=IoTGW_device_info[k].Size();

                    for(l=0; l<max_dev_info_size ; l++){
                        
                        //Get SenHubList value
                        if ( strcmp("n",IoTGW_device_info[k][l].Key().c_str()) == 0 &&
                             strcmp("SenHubList",IoTGW_device_info[k][l].Value().c_str()) == 0 && 
                             strcmp("sv",IoTGW_device_info[k][l+1].Key().c_str()) == 0){

                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l].Key().c_str(), IoTGW_device_info[k][l].Value().c_str()); 
                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l+1].Key().c_str(), IoTGW_device_info[k][l+1].Value().c_str());

                             if( OS_NONE_IP_BASE == osInfo){
                                 strcpy(temp->connectivitySensorHubList, IoTGW_device_info[k][l+1].Value().c_str());
                             }
                             else if ( OS_IP_BASE == osInfo){
                                 char sensorHubDevID[MAX_DEVICE_ID_LEN]={0};
                                 //add sensor hub node
                                 strcpy(sensorHubDevID, IoTGW_device_info[k][l+1].Value().c_str());
                                 if (strlen(sensorHubDevID) && strcmp(sensorHubDevID,"NULL") != 0 ){
                                     strcpy(virtualConnectivityDevID,connectivityDevID);
                                     AddVirtualGatewayDataListNode(virtualGatewayDevID,NULL,virtualConnectivityDevID,NULL, 0, TYPE_SENSOR_HUB, osInfo, sensorHubDevID);
                                 }
#if 0
                                 sprintf(trueConnectivityDevID,"0007%s", g_GWInfMAC);
				 temp=GetVirtualGatewayDataListNode(trueConnectivityDevID, TYPE_CONNECTIVITY);
				 if ( temp == NULL ){
				     printf("[%s][%s]can not find connectivity:%s node\n", __FILE__, __func__, connectivityDevID);
				 }
                                 else{
                                 }
#endif
                             }
                             else{
                                 printf("[%s][%s]Unknown OS Info: %d\n", __FILE__, __func__, osInfo);
                             }
                        }
                        //Get Neighbor value
                        if ( strcmp("n",IoTGW_device_info[k][l].Key().c_str()) == 0 &&
                             strcmp("Neighbor",IoTGW_device_info[k][l].Value().c_str()) == 0 && 
                             strcmp("sv",IoTGW_device_info[k][l+1].Key().c_str()) == 0){

                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l].Key().c_str(), IoTGW_device_info[k][l].Value().c_str()); 
                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l+1].Key().c_str(), IoTGW_device_info[k][l+1].Value().c_str());

                             if( OS_NONE_IP_BASE == osInfo){
                                 strcpy(temp->connectivityNeighborList, IoTGW_device_info[k][l+1].Value().c_str());
                             }
                             else if ( OS_IP_BASE == osInfo){
                                 //ToDo
                             }
                             else{
                                 printf("[%s][%s]Unknown OS Info\n", __FILE__, __func__);
                             }                             
                        }
                    }
                }

            }
        }

    }
}

void aTest(const char* mydata){

    AdvJSON json(mydata);
    int i=0, j=0, k=0;
    int max=0;
    char virtualGatewayDevID[MAX_DEVICE_ID_LEN]={0};
    char connectivityInfo[MAX_JSON_NODE_SIZE]={0};
    char connectivityDevID[MAX_DEVICE_ID_LEN]={0};
    char packet_type[32]={"data"};
    struct node *temp=NULL;


    //Get virtual gateway devID
    strcpy(virtualGatewayDevID,json["susiCommData"]["agentID"].Value().c_str());

    AdvJSON IoTGW_json=json["susiCommData"][packet_type]["IoTGW"];
    max=IoTGW_json.Size();

    for(i=0; i < max ;i++){
        char type[128]={0};
        char device[128]={0};

        strcpy(type,IoTGW_json[i].Key().c_str());
 
        int max_device=IoTGW_json[type].Size();

        for(j=0; j<max_device ; j++){
            strcpy(device,IoTGW_json[type][j].Key().c_str());

            if ( strcmp("NULL",IoTGW_json[type][device]["Info"].Value().c_str()) != 0){

                //Get connectivity Info
                strcpy(connectivityInfo,IoTGW_json[type][device].Value().c_str());
                //Get connectivity device ID
                strcpy(connectivityDevID,IoTGW_json[type][device]["bn"].Value().c_str());
#if 1
		printf("********************************************\n");
		printf("virtualGateway DevID: "); printf(virtualGatewayDevID); printf("\n");
		printf("connectivity Info: "); printf(connectivityInfo); printf("\n");
		printf("connectivity DevID: "); printf(connectivityDevID);
		printf("\n********************************************\n");
#endif
		//get connectivity node
#if 1
		temp=GetVirtualGatewayDataListNode(connectivityDevID, TYPE_CONNECTIVITY);
		if ( temp == NULL ){
			printf("[%s][%s]can not find connectivity:%s node\n", __FILE__, __func__, connectivityDevID);
			continue;
		}
#endif

                AdvJSON IoTGW_device_info=IoTGW_json[type][device]["Info"]["e"];
                int max_info_size=IoTGW_device_info.Size();

                for(k=0; k < max_info_size; k++){
                    int l=0;
                    int max_dev_info_size=IoTGW_device_info[k].Size();

                    for(l=0; l<max_dev_info_size ; l++){
                        
                        //Get SenHubList value
                        if ( strcmp("n",IoTGW_device_info[k][l].Key().c_str()) == 0 &&
                             strcmp("SenHubList",IoTGW_device_info[k][l].Value().c_str()) == 0 && 
                             strcmp("sv",IoTGW_device_info[k][l+1].Key().c_str()) == 0){

                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l].Key().c_str(), IoTGW_device_info[k][l].Value().c_str()); 
                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l+1].Key().c_str(), IoTGW_device_info[k][l+1].Value().c_str());
                             strcpy(temp->connectivitySensorHubList, IoTGW_device_info[k][l+1].Value().c_str());
                        }
                        //Get Neighbor value
                        if ( strcmp("n",IoTGW_device_info[k][l].Key().c_str()) == 0 &&
                             strcmp("Neighbor",IoTGW_device_info[k][l].Value().c_str()) == 0 && 
                             strcmp("sv",IoTGW_device_info[k][l+1].Key().c_str()) == 0){

                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l].Key().c_str(), IoTGW_device_info[k][l].Value().c_str()); 
                             printf("@@@@@@@@@@@ Key=%s, Value=%s\n", IoTGW_device_info[k][l+1].Key().c_str(), IoTGW_device_info[k][l+1].Value().c_str());
                             strcpy(temp->connectivityNeighborList, IoTGW_device_info[k][l+1].Value().c_str());
                        }
                    }
                }

            }
        }

    }
}
