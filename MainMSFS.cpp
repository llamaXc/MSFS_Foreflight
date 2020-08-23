
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <windows.h>
#include <tchar.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "SimConnect.h"
#include "string""
#include <vector>
#include <iostream>
#include <cmath>

#pragma comment (lib, "Ws2_32.lib")

static enum DATA_DEFINE_ID {
    LATITUDE,
    LONGITUDE,
    SPEED,
    ALTITUDE,
    PLANE_POS
};

//Struct for Simconnect to fill data with 
struct Position
{
    double  altitude;  // METERS 
    double  latitude;  // GPS 
    double  longitude; // GPS
    double  ground_track;  // Radians  
    double ground_speed;   //
    double true_heading;   // Radians 
    double pitch_degrees;  // Radians 
    double roll_degrees;   // Radians
};

//Request id for simconnnect
static enum DATA_REQUEST_ID {
    PLAYER_POSITION_ATITUDE = 10
};

//Data to be sent to foreflight
const char* packet;
std::string positionString;
std::string attitudeString;

//Socket connection info
const char* destIP;  //Ip for machine running foreflight
sockaddr_in dest;  
sockaddr_in local;
WSAData data;
SOCKET s;

int     quit = 0;
HANDLE  hSimConnect = NULL;


void CALLBACK UnpackSimConnectData(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    HRESULT hr;

    switch (pData->dwID){

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

            switch (pObjData->dwRequestID)
            {


                case PLAYER_POSITION_ATITUDE:
                {
                    DWORD ObjectID = pObjData->dwObjectID;
                    Position* pS = (Position*)&pObjData->dwData;

                    double altitude_meters = pS->altitude * 0.3048; //Convert Simconnect altitude to meters
                    double ground_track_degrees = pS->ground_track * (180.0 / M_PI); //Foreflight expects ground track in degrees (0-360)
                    double pitch_degrees = -1 * pS->pitch_degrees * (180.0 / M_PI); //Foreflight expects pitch up to be positive   (Degrees)
                    double roll_degrees = -1 * pS->roll_degrees * (180.0 / M_PI);   //Foreflight expects roll right to be positive (Degrees)


                    //Create data strings to send to foreflight
                    positionString = "XGPSMSFS," + std::to_string(pS->longitude) + "," + std::to_string(pS->latitude) + "," + std::to_string(altitude_meters)
                        + "," + std::to_string(ground_track_degrees) + "," + std::to_string(pS->ground_speed);

                    attitudeString = "XATTMSFS," + std::to_string(pS->true_heading) + "," + std::to_string(pitch_degrees) + "," + std::to_string(roll_degrees);


                    //Send position data to foreflight
                    packet = positionString.c_str();
                    sendto(s, packet, strlen(packet), 0, (sockaddr*)&dest, sizeof(dest));

                    //Send atitutde data to foreflight 
                    packet = attitudeString.c_str();
                    sendto(s, packet, strlen(packet), 0, (sockaddr*)&dest, sizeof(dest));

                    break;
                }



                default:
                    break;
            }
        }
    }
}

void connectToSim()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Flight Simulator!\n");

        //Fill up the plane pos struct to receive back info we want about this object 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION ALT", "feet"); //meters
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION LAT", "degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS POSITION LON", "degrees"); 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS GROUND TRUE TRACK", "Radians"); //to degreess
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "GPS GROUND SPEED", "Meters per second"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE HEADING DEGREES TRUE", "Radians"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE PITCH DEGREES", "Radians"); // good 
        hr = SimConnect_AddToDataDefinition(hSimConnect, PLANE_POS, "PLANE BANK DEGREES", "Radians"); // good 

        //request plane pos! 
        hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, PLAYER_POSITION_ATITUDE, PLANE_POS, 0, SIMCONNECT_SIMOBJECT_TYPE_AIRCRAFT);


        //Listen to sim data and send to Foreflight
        while (0 == quit)
        {
            SimConnect_CallDispatch(hSimConnect, UnpackSimConnectData, NULL);
            Sleep(200); //Send every 20ms (Foreflight requires position data at 1HZ and attitude at 6-10HZ at least.
        }

        hr = SimConnect_Close(hSimConnect);
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    printf("Argument Count Found: %d\n", argc);

    if (argc != 2) {
        printf("Usage: ForeFlightSupport.exe <ip>\n<ip> is your IPV4 ip of device running ForeFlight or your local broadcast ip\n\n");
        return -1;
    }

    //Get ip argument and print it
    std::wstring wstr(argv[1]);
    std::string str(wstr.begin(), wstr.end());
    std::cout << "Sending Data To: " << str << std::endl;
    destIP = str.c_str();


    //Setup socket 
    WSAStartup(MAKEWORD(2, 2), &data);

    dest.sin_family = AF_INET;
    inet_pton(AF_INET, destIP, &dest.sin_addr.s_addr);
    dest.sin_port = htons(49002);

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    char broadcast = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    //Connect and start
    connectToSim();


    //Clean up connections
    SimConnect_Close(hSimConnect);
    closesocket(s);
    WSACleanup();

    return 0;
}





