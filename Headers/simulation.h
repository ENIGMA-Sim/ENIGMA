#include <string.h>
#include <math.h>
#include <stdio.h>
#include "simgrid/plugins/energy.h"
#include "simgrid/msg.h"
#include "xbt/synchro.h"


#define MAX_DATACENTERS 10
#define MAX_SERVERS 1000


#define MFLOPS_BASE (1000*1000*1000) 						//To compute service time for each task
#define SERVICE_RATE 1.0				 							// Mu = 1  service time = 1 / 1; tasa de servicio de cada servidor







// Structure to handle the task queue on each Datacenter
struct TaskDatacenters
{
	xbt_dynar_t client_requests[MAX_SERVERS]; 	// cola de peticiones en cada servidor, array dinamico de SimGrid
	xbt_mutex_t mutex[MAX_SERVERS];
	xbt_cond_t cond[MAX_SERVERS];
	int EmptyQueue[MAX_SERVERS]; 				// indicacion de fin de cola en cada servidor

	// Statistical variables
	int Nqueue[MAX_SERVERS];		  				// elementos en la cola de cada servidor esperando a ser servidos
	double Navgqueue[MAX_SERVERS]; 				// tamanio medio de la cola de cada servidor

	int Nsystem[MAX_SERVERS];	   				// número de tareas en cada servidor (esperando y siendo atendidas)
	double Navgsystem[MAX_SERVERS]; 				// número medio de tareas por servidor (esperando y siendo atendidas)
};


struct AverageServiceTime
{
	double avServiceTime;
	int numTasks;
};



//Structure sent by the IoT device that contains the task to execute
struct ClientRequest
{
	char id_task[30];
	int n_task;		  // número de tarea
	double t_arrival; /* momento en el que llega la tarea (tiempo de creacion)*/
	double t_service; /* tiempo de servicio asignado en FLOPS*/
	int iot_cluster;
	int device;
	int finish_controller;
};


struct ServerResponse
{
	char id_task[30];
	char response[30];
	int server_cluster;
	int server;
	int iot_cluster;
	int device;
};



struct ControllerResponse
{
	int iot_cluster;
	int device;
	int finish;
};



struct TaskDatacenters tasksManagement[MAX_DATACENTERS];
struct AverageServiceTime avServTime[MAX_DATACENTERS];







int iot(int argc, char *argv[]);
int dispatcher(int argc, char *argv[]);
int datacenter(int argc, char *argv[]);
int controller(int argc, char *argv[]);
int dispatcherDatacenter(int argc, char *argv[]);
void test_all(char *file);
