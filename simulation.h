#include <string.h>
#include <math.h>
#include <stdio.h>
#include "simgrid/msg.h"
#include "xbt/synchro.h"
#include "rand.h"
#include "simgrid/plugins/energy.h"




#define NUM_DATACENTERS 5
#define NUM_SERVERS_PER_DATACENTER 5
#define NUM_VM 1
#define NUM_DISPATCHERS 5
#define NUM_FOG 0
#define NUM_IOT_CLUSTERS 5
#define NUM_DEVICES_PER_IOT_CLUSTER 100

#define SIZE_DATA 243 * 1024			 //KB
#define SIZE_OUTPUT_DATA 60 * 1024		 //KB


#define NUM_TASKS 2000			 									//Tasks to generate
#define MFLOPS_BASE (10 * 1000 * 1000 * 1000) 						//To compute service time for each task
#define SERVICE_RATE 1.0				 							// Mu = 1  service time = 1 / 1; tasa de servicio de cada servidor



double ARRIVAL_RATE; 												//Arrival rate, argument of the program
const long MAX_TIMEOUT_SERVER = (86400 * 0.001); 					//Timeout= 10 días sin actividad

int computeTasks;													//Variable that determines where the tasks have to execute 0-locally, 1-fog, 2-cloud
double percentageTasks; 											//Variable that determines which percentage of the tasks are executed on the devices



// Structure to handle the task queue on each Datacenter
struct TaskDatacenters
{
	xbt_dynar_t client_requests[NUM_SERVERS_PER_DATACENTER]; 	// cola de peticiones en cada servidor, array dinamico de SimGrid
	xbt_mutex_t mutex[NUM_SERVERS_PER_DATACENTER];
	xbt_cond_t cond[NUM_SERVERS_PER_DATACENTER];
	int EmptyQueue[NUM_SERVERS_PER_DATACENTER]; 				// indicacion de fin de cola en cada servidor

	// Statistical variables
	int Nqueue[NUM_SERVERS_PER_DATACENTER];		  				// elementos en la cola de cada servidor esperando a ser servidos
	double Navgqueue[NUM_SERVERS_PER_DATACENTER]; 				// tamanio medio de la cola de cada servidor

	int Nsystem[NUM_SERVERS_PER_DATACENTER];	   				// número de tareas en cada servidor (esperando y siendo atendidas)
	double Navgsystem[NUM_SERVERS_PER_DATACENTER]; 				// número medio de tareas por servidor (esperando y siendo atendidas)
};
struct TaskDatacenters tasksManagement[NUM_DATACENTERS];



struct AverageServiceTime
{
	double avServiceTime;
	int numTasks;
};

struct AverageServiceTime avServTime[NUM_DATACENTERS];

//Structure sent by the IoT device that contains the task to execute
struct ClientRequest
{
	char id_task[30];
	int n_task;		  // número de tarea
	double t_arrival; /* momento en el que llega la tarea (tiempo de creacion)*/
	double t_service; /* tiempo de servicio asignado en FLOPS*/
	int iot_cluster;
	int device;
};


struct ServerResponse
{
	char id_task[30];
	char response[30];
	int server_cluster;
	int server;
};



int client(int argc, char *argv[]);
int dispatcher(int argc, char *argv[]);
int server(int argc, char *argv[]);
int dispatcherServer(int argc, char *argv[]);
void test_all(char *file);
