//------------------------------------------------------------------------------
//
//  SimConnect Data Request Sample
//  
//	Description:
//				After a flight has loaded, request the lat/lon/alt of the user 
//				aircraft
//------------------------------------------------------------------------------

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* for memset() */
#include "SimConnect.h"
#include "string""
#include <vector>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

int     quit = 0;
HANDLE  hSimConnect = NULL;


static enum DATA_DEFINE_ID {
    LATITUDE,
    LONGITUDE,
    SPEED,
    ALTITUDE,
    PLANE_POS
};

struct Struct1
{
    double  altitude;
    double  latitude;
    double  longitude;
    double  ground_track; //degrees true north positive only  
    double ground_speed;
    double true_heading;
    double pitch_degrees;
    double roll_degrees;
};


const char* pkt = "XGPSAS,-80.11,34.55,1200.1,359.05,55.6";

const char* destIP;
sockaddr_in dest;
sockaddr_in local;
WSAData data;
SOCKET s;
std::string data_from_msfs;
std::string att_from_msfs;

static enum DATA_REQUEST_ID {
    REQUEST_1,
};

void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    HRESULT hr;

    switch (pData->dwID){

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

        switch (pObjData->dwRequestID)
        {
        case REQUEST_1:
        {
            DWORD ObjectID = pObjData->dwObjectID;
            Struct1* pS = (Struct1*)&pObjData->dwData;

            hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_1, PLANE_POS, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

            double ground_track_degrees = pS->ground_track * (180.0 / 3.141592653589793238463);
            double altitude_meters = pS->altitude * 0.3048;
            
            data_from_msfs = "XGPSMSFS," + std::to_string(pS->longitude) + "," + std::to_string(pS->latitude) + "," + std::to_string(altitude_meters)
                + "," + std::to_string(ground_track_degrees) + "," + std::to_string(pS->ground_speed);

           // printf(data_from_msfs.c_str());
           //  printf("\n");

            double pitch_d = -1 *  pS->pitch_degrees * (180.0 / 3.141592653589793238463); //up is + 
            double roll_d =  -1 * pS->roll_degrees * (180.0 / 3.141592653589793238463); //right is + 

            //  printf("true heading: %f.2   pitch: %f.2  roll: %f.2 \n", pS->true_heading, pitch_d, roll_d);


            att_from_msfs = "XATTMSFS," + std::to_string(pS->true_heading) + "," + std::to_string(pitch_d) + "," + std::to_string(roll_d);
         
            pkt = data_from_msfs.c_str();
            sendto(s, pkt, strlen(pkt), 0, (sockaddr*)&dest, sizeof(dest));

            pkt = att_from_msfs.c_str();
            sendto(s, pkt, strlen(pkt), 0, (sockaddr*)&dest, sizeof(dest));

            break;
        }

        default:
            break;
        }
        break;
    }


    }
}

void testDataRequest()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Flight Simulator!");


        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION ALT", "feet"); //meters
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION LAT", "degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION LON", "degrees"); 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS GROUND TRUE TRACK", "Radians"); //to degreess
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS GROUND SPEED", "Meters per second"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE HEADING DEGREES TRUE", "Radians"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE PITCH DEGREES", "Radians"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE BANK DEGREES", "Radians"); // good 

        //request plane pos! 
        hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_1, PLANE_POS, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

        while (0 == quit)
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
            Sleep(20);
        }

        hr = SimConnect_Close(hSimConnect);
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    std::wstring wstr(argv[1]);
    std::string str(wstr.begin(), wstr.end());
    std::cout << str << std::endl;

    destIP = str.c_str();

    std::vector<std::string> all_args;

    WSAStartup(MAKEWORD(2, 2), &data);

 
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, destIP, &dest.sin_addr.s_addr);
    dest.sin_port = htons(49002);

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    char broadcast = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

     testDataRequest();


   closesocket(s);
   WSACleanup();

    return 0;
}





