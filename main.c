#include "simulation.h"


void test_all(char *file)
{
	int argc;
	char str[50];
	int i, j;
	msg_process_t p;

	MSG_create_environment(file);

	// el proceso client es el que genera las tareas en los dispositivos IoT
	MSG_function_register("client", client);
	
	// el proceso dispatcher es el que distribuye las peticiones que le llegan a los Datacenters
	MSG_function_register("dispatcher", dispatcher);

	// cada servidor tiene un proceso server que recibe las peticiones: server
	// y un proceso dispatcher que las ejecuta
	MSG_function_register("server", server);
	MSG_function_register("dispatcherServer", dispatcherServer);

	for(i = 0; i < NUM_DATACENTERS; i++)
	{
		for (j = 0; j < NUM_SERVERS_PER_DATACENTER; j++)
		{
			sprintf(str, "s-%d-%d", i, j);
			argc = 2;
			char **argvc = xbt_new(char *, 3);

			argvc[0] = bprintf("%d", i);
			argvc[1] = bprintf("%d", j);
			argvc[2] = NULL;

			p = MSG_process_create_with_arguments(str, server, NULL, MSG_get_host_by_name(str), argc, argvc);
			
			if (p == NULL)
			{
				printf("Error en ......... s-%d-%d\n", i, j);
				exit(0);
			}
		}
	}
	

	for(i = 0; i < NUM_DATACENTERS; i++)
	{
		for (j = 0; j < NUM_SERVERS_PER_DATACENTER; j++)
		{
			sprintf(str, "s-%d-%d", i, j);
			argc = 2;
			char **argvc = xbt_new(char *, 3);

			argvc[0] = bprintf("%d", i);
			argvc[1] = bprintf("%d", j);
			argvc[2] = NULL;
			
			p = MSG_process_create_with_arguments(str, dispatcherServer, NULL, MSG_get_host_by_name(str), argc, argvc);
			if (p == NULL)
			{
				printf("Error en ......... s-%d-%d\n", j);
				exit(0);
			}
		}
	}



	for(i = 0; i < NUM_IOT_CLUSTERS; i++)
	{
		for (j = 0; j < NUM_DEVICES_PER_IOT_CLUSTER; j++)
		{
			sprintf(str, "c-%d-%d", i,j);
			argc = 2;
			char **argvc = xbt_new(char *, 3);

			argvc[0] = bprintf("%d", i);
			argvc[1] = bprintf("%d", j);
			argvc[2] = NULL;

			p = MSG_process_create_with_arguments(str, client, NULL, MSG_get_host_by_name(str), argc, argvc);
			if (p == NULL)
			{
				printf("Error en ......... c-%d-%d\n", i, j);
				exit(0);
			}
		}
	}

	

	for (i = 0; i < NUM_DISPATCHERS; i++)
	{
		sprintf(str, "d-%d-0", i);
		argc = 1;
		char **argvc = xbt_new(char *, 2);

		argvc[0] = bprintf("%d", i);
		argvc[1] = NULL;

		p = MSG_process_create_with_arguments(str, dispatcher, NULL, MSG_get_host_by_name(str), argc, argvc);
		if (p == NULL)
		{
			printf("Error en ......... d-%d-0\n", i);
			exit(0);
		}
	}

	return;
}







/** Main function */
int main(int argc, char *argv[])
{
	msg_error_t res = MSG_OK;
	int i,j;

	double t_medio_servicio = 0.0; // tiempo medio de servicio de cada tarea
	double q_medio = 0.0;		   // tamaño medio de la cola (esperando a ser servidos)
	double n_medio = 0.0;		   // número medio de tareas en el sistema (esperando y ejecutando)

	if (argc < 4)
	{
		printf("Usage: %s platform_file lambda percentageTasks \n", argv[0]);
		exit(1);
	}

	if (atof(argv[3]) > 1 || atof(argv[3]) < 0)
	{
		printf("Percentage Tasks must be a value between 0 and 1.\n");
		exit(1);
	}

	percentageTasks = atof(argv[3]);


	seed((int)time(NULL));
	ARRIVAL_RATE = atof(argv[2]) * NUM_SERVERS_PER_DATACENTER;
	sg_host_energy_plugin_init();
	MSG_init(&argc, argv);

	for (i = 0; i < NUM_DATACENTERS; i++)
	{
		for (j = 0; j < NUM_SERVERS_PER_DATACENTER; j++)
		{
			tasksManagement[i].Nqueue[j] = 0;
			tasksManagement[i].Nsystem[j] = 0;
			tasksManagement[i].EmptyQueue[j] = 0;
			tasksManagement[i].mutex[j] = xbt_mutex_init();
			tasksManagement[i].cond[j] = xbt_cond_init();
			tasksManagement[i].client_requests[j] = xbt_dynar_new(sizeof(struct ClientRequest *), NULL);
		}
	}

	test_all(argv[1]);

	res = MSG_main();
	int totaltasks = 0;
	for (i = 0; i < NUM_DATACENTERS; i++)
	{
		for (j = 0; j < NUM_SERVERS_PER_DATACENTER; j++)
		{
			q_medio = q_medio + tasksManagement[i].Navgqueue[j];
			n_medio = n_medio + tasksManagement[i].Navgsystem[j];
		}

		t_medio_servicio = avServTime[i].avServiceTime / (avServTime[i].numTasks);
		q_medio = q_medio / (NUM_SERVERS_PER_DATACENTER);
		n_medio = n_medio / (NUM_SERVERS_PER_DATACENTER);

		printf("DATACENTER \t tiempoMedioServicio \t TamañoMediocola \t    TareasMediasEnElSistema  \t   tareas\n");
		printf("%i \t\t %g \t\t\t %g \t\t\t  %g  \t\t\t  %d \n\n", i, t_medio_servicio, q_medio, n_medio, avServTime[i].numTasks);

		t_medio_servicio = 0;
		q_medio = 0;
		n_medio = 0;
		totaltasks += avServTime[i].numTasks;
	}
	
	printf("Simulation time %g\n",MSG_get_clock());

	for(i = 0; i < NUM_DATACENTERS; i++)
	{
		for (j = 0; j < NUM_SERVERS_PER_DATACENTER; j++)
		{
			xbt_dynar_free(&tasksManagement[i].client_requests[j]);
		}
	}
	

	if (res == MSG_OK)
		return 0;
	else
		return 1;
}