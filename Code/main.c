#include "./simulation.h"

#define NODESDATACENTER0 20
#define EDGE0 10
#define EDGE1 10
#define EDGE2 10
#define FOG0 15
#define IOT0 3
#define IOT1 3
#define IOT2 3
#define IOT3 3
#define IOT4 3
#define IOT5 3
#define IOT6 3
#define IOT7 3
#define IOT8 3

void test_all(char *file)
{
	int argc, num_edge, cluster_edge, num_datacenters, nodes_datacenter, tasks[9];
	char str[50];
	msg_process_t p;

	MSG_create_environment(file);
	MSG_function_register("cloud0", cloud0);
	MSG_function_register("dispatcherCloud0", dispatcherCloud0);
	MSG_function_register("edge0", edge0);
	MSG_function_register("edge1", edge1);
	MSG_function_register("edge2", edge2);
	MSG_function_register("fog0", fog0);
	MSG_function_register("iot0", iot0);
	MSG_function_register("iot1", iot1);
	MSG_function_register("iot2", iot2);
	MSG_function_register("iot3", iot3);
	MSG_function_register("iot4", iot4);
	MSG_function_register("iot5", iot5);
	MSG_function_register("iot6", iot6);
	MSG_function_register("iot7", iot7);
	MSG_function_register("iot8", iot8);
	MSG_function_register("controller", controller);

	sprintf(str, "cont-0");
	printf("Creating %s\n", str);
	argc = 0;
	char **argvc14 = xbt_new(char *, 1);
	argvc14[0] = NULL;
	p = MSG_process_create_with_arguments(str, controller, NULL, MSG_get_host_by_name(str), argc, argvc14);

	if( p == NULL )
	{
		printf("Error en %s\n", str);
		exit(0);
	}


	/*IOT*/

	for(int i = 0; i < IOT0; i++)
	{
		sprintf(str, "iot-0-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc0 = xbt_new(char *, 6);
		argvc0[0] = bprintf("%d", 0);
		argvc0[1] = bprintf("%d", i);
		argvc0[2] = bprintf("%d", 0); //Track number
		argvc0[3] = bprintf("%d", 0); //Organization
		argvc0[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot0, NULL, MSG_get_host_by_name(str), argc, argvc0);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT1; i++)
	{
		sprintf(str, "iot-1-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc1 = xbt_new(char *, 6);
		argvc1[0] = bprintf("%d", 1);
		argvc1[1] = bprintf("%d", i);
		argvc1[2] = bprintf("%d", 1); //Track number
		argvc1[3] = bprintf("%d", 0); //Organization
		argvc1[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot1, NULL, MSG_get_host_by_name(str), argc, argvc1);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT2; i++)
	{
		sprintf(str, "iot-2-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc2 = xbt_new(char *, 6);
		argvc2[0] = bprintf("%d", 2);
		argvc2[1] = bprintf("%d", i);
		argvc2[2] = bprintf("%d", 2); //Track number
		argvc2[3] = bprintf("%d", 0); //Organization
		argvc2[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot2, NULL, MSG_get_host_by_name(str), argc, argvc2);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT3; i++)
	{
		sprintf(str, "iot-3-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc3 = xbt_new(char *, 6);
		argvc3[0] = bprintf("%d", 3);
		argvc3[1] = bprintf("%d", i);
		argvc3[2] = bprintf("%d", 0); //Track number
		argvc3[3] = bprintf("%d", 1); //Organization
		argvc3[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot3, NULL, MSG_get_host_by_name(str), argc, argvc3);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT4; i++)
	{
		sprintf(str, "iot-4-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc4 = xbt_new(char *, 6);
		argvc4[0] = bprintf("%d", 4);
		argvc4[1] = bprintf("%d", i);
		argvc4[2] = bprintf("%d", 1); //Track number
		argvc4[3] = bprintf("%d", 1); //Organization
		argvc4[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot4, NULL, MSG_get_host_by_name(str), argc, argvc4);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT5; i++)
	{
		sprintf(str, "iot-5-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc5 = xbt_new(char *, 6);
		argvc5[0] = bprintf("%d", 5);
		argvc5[1] = bprintf("%d", i);
		argvc5[2] = bprintf("%d", 2); //Track number
		argvc5[3] = bprintf("%d", 1); //Organization
		argvc5[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot5, NULL, MSG_get_host_by_name(str), argc, argvc5);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT6; i++)
	{
		sprintf(str, "iot-6-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc6 = xbt_new(char *, 6);
		argvc6[0] = bprintf("%d", 6);
		argvc6[1] = bprintf("%d", i);
		argvc6[2] = bprintf("%d", 0); //Track number
		argvc6[3] = bprintf("%d", 2); //Organization
		argvc6[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot6, NULL, MSG_get_host_by_name(str), argc, argvc6);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT7; i++)
	{
		sprintf(str, "iot-7-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc7 = xbt_new(char *, 6);
		argvc7[0] = bprintf("%d", 7);
		argvc7[1] = bprintf("%d", i);
		argvc7[2] = bprintf("%d", 1); //Track number
		argvc7[3] = bprintf("%d", 2); //Organization
		argvc7[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot7, NULL, MSG_get_host_by_name(str), argc, argvc7);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < IOT8; i++)
	{
		sprintf(str, "iot-8-%d", i);
		printf("Creating %s\n", str);
		argc = 4;
		char **argvc8 = xbt_new(char *, 6);
		argvc8[0] = bprintf("%d", 8);
		argvc8[1] = bprintf("%d", i);
		argvc8[2] = bprintf("%d", 2); //Track number
		argvc8[3] = bprintf("%d", 2); //Organization
		argvc8[4] = NULL;
		p = MSG_process_create_with_arguments(str, iot8, NULL, MSG_get_host_by_name(str), argc, argvc8);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}


	/*EDGE*/

	for(int i = 0; i < EDGE0; i++)
	{
		sprintf(str, "edge-0-%d", i);
		printf("Creating %s\n", str);
		argc = 2;
		char **argvc9 = xbt_new(char *, 3);
		argvc9[0] = bprintf("%d", 0);
		argvc9[1] = bprintf("%d", i);
		argvc9[2] = NULL;
		p = MSG_process_create_with_arguments(str, edge0, NULL, MSG_get_host_by_name(str), argc, argvc9);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < EDGE1; i++)
	{
		sprintf(str, "edge-1-%d", i);
		printf("Creating %s\n", str);
		argc = 2;
		char **argvc10 = xbt_new(char *, 3);
		argvc10[0] = bprintf("%d", 1);
		argvc10[1] = bprintf("%d", i);
		argvc10[2] = NULL;
		p = MSG_process_create_with_arguments(str, edge1, NULL, MSG_get_host_by_name(str), argc, argvc10);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	for(int i = 0; i < EDGE2; i++)
	{
		sprintf(str, "edge-2-%d", i);
		printf("Creating %s\n", str);
		argc = 2;
		char **argvc11 = xbt_new(char *, 3);
		argvc11[0] = bprintf("%d", 2);
		argvc11[1] = bprintf("%d", i);
		argvc11[2] = NULL;
		p = MSG_process_create_with_arguments(str, edge2, NULL, MSG_get_host_by_name(str), argc, argvc11);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}


	/*FOG*/

	for(int i = 0; i < FOG0; i++)
	{
		sprintf(str, "fog-0-%d", i);
		printf("Creating %s\n", str);
		argc = 2;
		char **argvc15 = xbt_new(char *, 3);
		argvc15[0] = bprintf("%d", 0);
		argvc15[1] = bprintf("%d", i);
		argvc15[2] = NULL;
		p = MSG_process_create_with_arguments(str, fog0, NULL, MSG_get_host_by_name(str), argc, argvc15);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}


	/*DATACENTER0*/

	for(int i = 0; i < NODESDATACENTER0; i++)
	{
		sprintf(str, "cloud-0-%d", i);
		printf("Creating %s\n", str);
		argc = 3;
		char **argvc12 = xbt_new(char *, 4);
		argvc12[0] = bprintf("%s", str);
		argvc12[1] = bprintf("%d", 0);
		argvc12[2] = bprintf("%d", i);
		argvc12[3] = NULL;
		p = MSG_process_create_with_arguments(str, cloud0, NULL, MSG_get_host_by_name(str), argc, argvc12);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}


	/*DISPATCHER0*/

	for(int i = 0; i < NODESDATACENTER0; i++)
	{
		sprintf(str, "cloud-0-%d", i);
		//printf("Creating %s\n", str);
		argc = 3;
		char **argvc13 = xbt_new(char *, 4);
		argvc13[0] = bprintf("%s", str);
		argvc13[1] = bprintf("%d", 0);
		argvc13[2] = bprintf("%d", i);
		argvc13[3] = NULL;
		p = MSG_process_create_with_arguments(str, dispatcherCloud0, NULL, MSG_get_host_by_name(str), argc, argvc13);

		if( p == NULL )
		{
			printf("Error en %s\n", str);
			exit(0);
		}
	}

	return;

}

int main(int argc, char *argv[])
{
	msg_error_t res = MSG_OK;
	int taskExecuted = 0, days, min, hours;
	double q_medio = 0.0, n_medio = 0.0, t, t_s;

	if (argc < 3)
	{
		printf("Usage: %s platform_user.xml output.xml\n", argv[0]);
		exit(1);
	}
	t = (double)time(NULL);
	seed((int)time(NULL));
	sg_host_energy_plugin_init(t);
	MSG_init(&argc, argv);

	for(int i = 0; i < MAX_DATACENTERS; i++)
	{
		for(int j = 0; j < MAX_SERVERS; j++)
		{
			tasksM[i].Nqueue[j] = 0;
			tasksM[i].Nsystem[j] = 0;
			tasksM[i].EmptyQueue[j] = 0;
			tasksM[i].mutex[j] = xbt_mutex_init();
			tasksM[i].cond[j] = xbt_cond_init();
			tasksM[i].client_requests[j] = xbt_dynar_new(sizeof(struct InfoTruck *), NULL);
			//tasksM[i].client_requests[j] = new_queue();
		}
	}

	test_all(argv[1]);
	res = MSG_main();
	FILE *fp = fopen(argv[2], "w+");
	char h[30];
	msg_host_t host;

	fprintf(fp, "Server;Tasks Executed;Data Processed (MB);Energy Consumed;Average Energy Consumed;Total Time;Execution Time;CPU Usage %\n");

	/*STATISTICS DATACENTER0*/

	for(int i = 0; i < NODESDATACENTER0; i++)
	{
		q_medio = q_medio + tasksM[0].Navgqueue[i];
		n_medio = n_medio + tasksM[0].Navgsystem[i];
		sprintf(h, "cloud-0-%d", i);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",MSG_host_get_name(host), statsCloud[0].numTasks[i], statsCloud[0].dataProcessed[i]/(1024*1024), statsCloud[0].totalEnergy[i], statsCloud[0].avEnergy[i], MSG_get_clock(), statsCloud[0].executionTime[0], statsCloud[0].executionTime[i]*100/MSG_get_clock());
		taskExecuted += statsCloud[0].numTasks[i];
	}
	fprintf(fp, "\n\n\n");
	fprintf(fp, "Edge;Tasks Executed;Data Processed (MB);Energy Consumed;Average Energy Consumed;Total Time;Execution Time;CPU Usage %\n");


	/*STATISTICS EDGE0*/

	for (int j = 0; j < EDGE0; j++)
	{
		sprintf(h, "edge-0-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",MSG_host_get_name(host),statsEdge[0].numTasks[j], statsEdge[0].dataProcessed[j]/(1024*1024),statsEdge[0].totalEnergy[j], statsEdge[0].avEnergy[j], MSG_get_clock(),statsEdge[0].executionTime[j], statsEdge[0].executionTime[j]*100/MSG_get_clock());
		taskExecuted += statsEdge[0].numTasks[j];
	}

	/*STATISTICS EDGE1*/

	for (int j = 0; j < EDGE1; j++)
	{
		sprintf(h, "edge-1-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",MSG_host_get_name(host),statsEdge[1].numTasks[j], statsEdge[1].dataProcessed[j]/(1024*1024),statsEdge[1].totalEnergy[j], statsEdge[1].avEnergy[j], MSG_get_clock(),statsEdge[1].executionTime[j], statsEdge[1].executionTime[j]*100/MSG_get_clock());
		taskExecuted += statsEdge[1].numTasks[j];
	}

	/*STATISTICS EDGE2*/

	for (int j = 0; j < EDGE2; j++)
	{
		sprintf(h, "edge-2-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",MSG_host_get_name(host),statsEdge[2].numTasks[j], statsEdge[2].dataProcessed[j]/(1024*1024),statsEdge[2].totalEnergy[j], statsEdge[2].avEnergy[j], MSG_get_clock(),statsEdge[2].executionTime[j], statsEdge[2].executionTime[j]*100/MSG_get_clock());
		taskExecuted += statsEdge[2].numTasks[j];
	}
	fprintf(fp, "\n\n\n");
	fprintf(fp, "Fog;Tasks Executed;Data Processed (MB);Energy Consumed;Average Energy Consumed;Total Time;Execution Time;CPU Usage %\n");

	for (int j = 0; j < FOG0; j++)
	{
		sprintf(h, "fog-0-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",MSG_host_get_name(host),statsFog[0].numTasks[j], statsFog[0].dataProcessed[j]/(1024*1024),statsFog[0].totalEnergy[j], statsFog[0].avEnergy[j], MSG_get_clock(),statsFog[0].executionTime[j], statsFog[0].executionTime[j]*100/MSG_get_clock());
		taskExecuted += statsFog[0].numTasks[j];
	}


	fprintf(fp, "\n\n\n");
	fprintf(fp, "IoT Device;Tasks Executed;Energy Consumed;Average Energy Consumed\n");


	/*STATISTICS IOT0*/

	for (int j = 0; j < IOT0; j++)
	{
		sprintf(h, "iot-0-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[0].numTasks[j],statsIoT[0].totalEnergy[j], statsIoT[0].avEnergy[j]);
	}


	/*STATISTICS IOT1*/

	for (int j = 0; j < IOT1; j++)
	{
		sprintf(h, "iot-1-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[1].numTasks[j],statsIoT[1].totalEnergy[j], statsIoT[1].avEnergy[j]);
	}


	/*STATISTICS IOT2*/

	for (int j = 0; j < IOT2; j++)
	{
		sprintf(h, "iot-2-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[2].numTasks[j],statsIoT[2].totalEnergy[j], statsIoT[2].avEnergy[j]);
	}


	/*STATISTICS IOT3*/

	for (int j = 0; j < IOT3; j++)
	{
		sprintf(h, "iot-3-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[3].numTasks[j],statsIoT[3].totalEnergy[j], statsIoT[3].avEnergy[j]);
	}


	/*STATISTICS IOT4*/

	for (int j = 0; j < IOT4; j++)
	{
		sprintf(h, "iot-4-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[4].numTasks[j],statsIoT[4].totalEnergy[j], statsIoT[4].avEnergy[j]);
	}


	/*STATISTICS IOT5*/

	for (int j = 0; j < IOT5; j++)
	{
		sprintf(h, "iot-5-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[5].numTasks[j],statsIoT[5].totalEnergy[j], statsIoT[5].avEnergy[j]);
	}


	/*STATISTICS IOT6*/

	for (int j = 0; j < IOT6; j++)
	{
		sprintf(h, "iot-6-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[6].numTasks[j],statsIoT[6].totalEnergy[j], statsIoT[6].avEnergy[j]);
	}


	/*STATISTICS IOT7*/

	for (int j = 0; j < IOT7; j++)
	{
		sprintf(h, "iot-7-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[7].numTasks[j],statsIoT[7].totalEnergy[j], statsIoT[7].avEnergy[j]);
	}


	/*STATISTICS IOT8*/

	for (int j = 0; j < IOT8; j++)
	{
		sprintf(h, "iot-8-%d", j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s;%d;%.2f;%.2f\n",MSG_host_get_name(host), statsIoT[8].numTasks[j],statsIoT[8].totalEnergy[j], statsIoT[8].avEnergy[j]);
	}

	fprintf(fp, "\n\n\n");
	t_s = MSG_get_clock();
	days = (int) (t_s / (24 * 3600));
	t_s -= (days * 24 * 3600);
	hours = (int) (t_s / 3600);
	t_s -= (hours * 3600);
	min = (int) (t_s / 60);
	t_s -= (min * 60);
	fprintf(fp, "Simulation time:; %d days; %d hours; %d min; %d seconds\n", days, hours, min, (int)round(t_s));

	t = (double)time(NULL) - t;
	days = (int) (t / (24 * 3600));
	t -= (days * 24 * 3600);
	hours = (int) (t / 3600);
	t -= (hours * 3600);
	min = (int) (t / 60);
	t -= (min * 60);
	fprintf(fp, "Simulation time:; %d days; %d hours; %d min; %d seconds\n", days, hours, min, (int)round(t));


	for(int i = 0; i < MAX_DATACENTERS; i++)
	{
		for(int j = 0; j < MAX_SERVERS; j++)
		{
			xbt_dynar_free(&tasksM[i].client_requests[j]);
		}
	}

	if (res == MSG_OK) return 0;
	else return 1; 
}
