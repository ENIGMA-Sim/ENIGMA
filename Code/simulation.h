#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "simgrid/plugins/energy.h"
#include "simgrid/msg.h"
#include "xbt/synchro.h"
//#include "queue.h"

#define MAX_REQUESTS 10000

#define MAX_DATACENTERS 1
#define MAX_FOG 1
#define MAX_EDGE 3
#define MAX_IOT 9
#define MAX_SERVERS 20
#define MAX_NODES_EDGE 10
#define MAX_NODES_FOG 15
#define MAX_DEVICES_IOT 3


struct TaskClouds
{
	xbt_dynar_t client_requests[MAX_REQUESTS];
	//queue client_requests[MAX_REQUESTS];
	xbt_mutex_t mutex[MAX_SERVERS];
	xbt_cond_t cond[MAX_SERVERS];
	int EmptyQueue[MAX_SERVERS];
	int Nqueue[MAX_SERVERS];
	double Navgqueue[MAX_SERVERS];
	int Nsystem[MAX_SERVERS];
	double Navgsystem[MAX_SERVERS];
};

struct TaskClouds tasksM[MAX_DATACENTERS];

struct InfoTruck
{
    int device;
    double t_arrival;
    double t_service;
    int size_task;
    double temp;
    int people;
    double coord_x;
    double coord_y;
    int org;
};

struct ClientRequest
{
	char id_task[15];
	//int n_task;
	double t_arrival;
	double t_service;
	int iot_cluster;
	int device;
	double edge_usage;
	double fog_usage;
	int size_task;
	double temp;
	int people;
	double coord_x;
	double coord_y;
	int org;
	int gps;
};

struct ControllerResponse
{
	int iot_cluster;
	int device;
	int finish;
};

struct StatisticsCloud
{
	double totalEnergy[MAX_SERVERS];
	double avEnergy[MAX_SERVERS];
	int numTasks[MAX_SERVERS];
	double executionTime[MAX_SERVERS];
	double dataProcessed[MAX_SERVERS];
};

struct StatisticsCloud statsCloud[MAX_DATACENTERS];

struct StatisticsEdge
{
	double totalEnergy[MAX_NODES_EDGE];
	double avEnergy[MAX_NODES_EDGE];
	int numTasks[MAX_NODES_EDGE];
	double executionTime[MAX_NODES_EDGE];
	double dataProcessed[MAX_NODES_EDGE];
};

struct StatisticsEdge statsEdge[MAX_EDGE];

struct StatisticsFog
{
	double totalEnergy[MAX_NODES_FOG];
	double avEnergy[MAX_NODES_FOG];
	int numTasks[MAX_NODES_FOG];
	double executionTime[MAX_NODES_FOG];
	double dataProcessed[MAX_NODES_FOG];
};

struct StatisticsEdge statsFog[MAX_FOG];

struct StatisticsIot
{
	double totalEnergy[MAX_DEVICES_IOT];
	double avEnergy[MAX_DEVICES_IOT];
	int numTasks[MAX_DEVICES_IOT];
	double avTime[MAX_DEVICES_IOT];
};

struct StatisticsIot statsIoT[MAX_IOT];

int cloud0(int argc, char *argv[]);
int dispatcherCloud0(int argc, char *argv[]);
int fog0(int argc, char *argv[]);
int edge0(int argc, char *argv[]);
int edge1(int argc, char *argv[]);
int edge2(int argc, char *argv[]);
int iot0(int argc, char *argv[]);
int iot1(int argc, char *argv[]);
int iot2(int argc, char *argv[]);
int iot3(int argc, char *argv[]);
int iot4(int argc, char *argv[]);
int iot5(int argc, char *argv[]);
int iot6(int argc, char *argv[]);
int iot7(int argc, char *argv[]);
int iot8(int argc, char *argv[]);
int controller(int argc, char *argv[]);
void test_all(char *file);
