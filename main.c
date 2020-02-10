#include "./Headers/simulation.h"

void test_all(char *file)
{
	int argc, i, j;
	char str[50];
	msg_process_t p;

	MSG_create_environment(file);
	MSG_function_register("iot", iot);
	MSG_function_register("dispatcher", dispatcher);
	MSG_function_register("datacenter", datacenter);
	MSG_function_register("dispatcherDatacenter", dispatcherDatacenter);
	MSG_function_register("controller", controller);


	int request_data, num_tasks, num_datacenters, output_data;
	double arrival, percentage;

	//Datacenter 0

	i = 0;

	for(j = 0; j < 10; j++)
	{
		sprintf(str, "s-%d-%d", i, j);
		argc = 2;
		char **argvc = xbt_new(char *, 3);

		argvc[0] = bprintf("%d",i);
		argvc[1] = bprintf("%d",j);
		argvc[2] = NULL;

		p = MSG_process_create_with_arguments(str, datacenter, NULL, MSG_get_host_by_name(str), argc, argvc);
		if(p == NULL)
		{
			printf("Error en ......... s-%d-%d", i, j);
			printf("\n");
			exit(0);
		}
	}

	//Dispatcher Datacenter 0

	i = 0;

	for(j = 0; j < 10; j++)
	{
		sprintf(str, "s-%d-%d", i, j);
		argc = 3;
		char **argvc = xbt_new(char *, 4);

		output_data = 1024;
		argvc[0] = bprintf("%d",i);
		argvc[1] = bprintf("%d",j);
		argvc[2] = bprintf("%d",output_data);
		argvc[3] = NULL;

		p = MSG_process_create_with_arguments(str, dispatcherDatacenter, NULL, MSG_get_host_by_name(str), argc, argvc);
		if(p == NULL)
		{
			printf("Error en ......... s-%d-%d", i, j);
			printf("\n");
			exit(0);
		}
	}

	//IoT Devices from cluster 0

	i = 0;
	request_data = 1024;
	num_tasks = 2000;
	percentage = 0.5;
	num_datacenters = 1;
	arrival = 1 * MAX_SERVERS;

	for(j = 0; j < 10000; j++)
	{
		sprintf(str, "iot-%d-%d", i, j);
		argc = 7;
		char **argvc = xbt_new(char *, 8);

		argvc[0] = bprintf("%d",i);
		argvc[1] = bprintf("%d",j);
		argvc[2] = bprintf("%d",request_data);
		argvc[3] = bprintf("%d",num_tasks);
		argvc[4] = bprintf("%g",percentage);
		argvc[5] = bprintf("%d",num_datacenters);
		argvc[6] = bprintf("%g",arrival);
		argvc[7] = NULL;

		p = MSG_process_create_with_arguments(str, iot, NULL, MSG_get_host_by_name(str), argc, argvc);
		if(p == NULL)
		{
			printf("Error en ......... iot-%d-%d", i, j);
			printf("\n");
			exit(0);
		}
	}
	int nservers;

	//Dispatcher 0

	i = 0;

	sprintf(str, "d-%d-0", i);
	argc = 2;
	char **argvc0 = xbt_new(char *, 3);

	nservers = 10;
	argvc0[0] = bprintf("%d",i);
	argvc0[1] = bprintf("%d",nservers);
	argvc0[2] = NULL;

	p = MSG_process_create_with_arguments(str, dispatcher, NULL, MSG_get_host_by_name(str), argc, argvc0);
	if(p == NULL)
	{
		printf("Error en ......... d-%d-0", i);
		printf("\n");
		exit(0);
	}


	//Controller

	int total_devices = 10000;
	int total_datacenters = 1;

	sprintf(str, "cont-0");
	argc = 2;
	char **argvc1 = xbt_new(char *, 3);

	argvc1[0] = bprintf("%d", total_devices);
	argvc1[1] = bprintf("%d", total_datacenters);
	argvc1[2] = NULL;

	p = MSG_process_create_with_arguments(str, controller, NULL, MSG_get_host_by_name(str), argc, argvc1);
	if(p == NULL)
	{
		printf("Error en ......... cont-0");
		printf("\n");
		exit(0);
	}
	return;
}




int main(int argc, char *argv[])
{
	msg_error_t res = MSG_OK;
	int i, j, tasksExecuted = 0, days, hours, min;
	double q_medio = 0.0;
	double n_medio = 0.0, t;

	if (argc < 2)
	{
		printf("Usage: %s platform_file", argv[0]);
		printf("\n");
		exit(1);
	}

	t = (double)time(NULL);	
	seed((int)time(NULL));
	sg_host_energy_plugin_init();
	MSG_init(&argc, argv);




	for(j = 0; j < 10; j++)
	{
		tasksManagement[0].Nqueue[j] = 0;
		tasksManagement[0].Nsystem[j] = 0;
		tasksManagement[0].EmptyQueue[j] = 0;
		tasksManagement[0].mutex[j] = xbt_mutex_init();
		tasksManagement[0].cond[j] = xbt_cond_init();
		tasksManagement[0].client_requests[j] = xbt_dynar_new(sizeof(struct ClientRequest *), NULL);
	}

	test_all(argv[1]);
	res = MSG_main();


	FILE *fp = fopen("./Resultados/results_10s_10000iot_2000t_50_50.csv", "w+");
	char h[30];
	msg_host_t host;
	fprintf(fp, "Server,Tasks Executed,Energy Consumed,Average Energy Consumed,Average Time\n");



	i = 0;
	for(j = 0; j < 10; j++)
	{
		q_medio = q_medio + tasksManagement[0].Navgqueue[j];
		n_medio = n_medio + tasksManagement[0].Navgsystem[j];
		sprintf(h, "s-%d-%d", i, j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s,%d,%.6f,%g,%g\n",MSG_host_get_name(host), statsDatacenter[i].numTasks[j], statsDatacenter[i].totalEnergy[j], statsDatacenter[i].avEnergy[j], statsDatacenter[i].avTime[j]);
		tasksExecuted += statsDatacenter[i].numTasks[j];
	}

	fprintf(fp,"\n");
	fprintf(fp,"DATACENTER,TamañoMediocola,TareasMediasEnElSistema,tareas\n");
	fprintf(fp,"%i,%g,%g,%d\n\n", i, q_medio, n_medio, tasksExecuted);

	q_medio = 0;
	n_medio = 0;

	fprintf(fp,"\n\n\n\n");


	fprintf(fp, "IoT Device,Tasks Executed,Energy Consumed,Average Energy Consumed,Average Time\n");






	i = 0;
	for(j = 0; j < 10000; j++)
	{
		sprintf(h, "iot-%d-%d", i, j);
		host = MSG_host_by_name(h);
		fprintf(fp,"%s,%d,%.6f,%g,%g\n",MSG_host_get_name(host), statsIoT[i].numTasks[j], statsIoT[i].totalEnergy[j], statsIoT[i].avEnergy[j], statsIoT[i].avTime[j]);
	}



	fprintf(fp,"\n\n\n");

	double t_s = MSG_get_clock();	
	int days_s = (int)(t_s / (24*3600));	
	t_s -= (days_s*24*3600);
	int hours_s = (int)(t_s/3600);		
	t_s -= (hours_s*3600);
	int min_s = (int)(t_s/60);
	t_s -= (min*60);


	fprintf(fp,"Simulation time,%d days,%d hours,%d min,%d s\n\n", days_s, hours_s, min_s, (int)round(t_s));

	t = (double)time(NULL) - t;	// Program time
	days = (int)(t / (24*3600));	// Calculate days
	t -= (days*24*3600);
	hours = (int)(t/3600);		// Calculate hours
	t -= (hours*3600);
	min = (int)(t/60);		// Calculate minutes
	t -= (min*60);
	
	fprintf(fp,"Execution time,%d days,%d hours,%d min,%d s\n\n", days, hours, min, (int)round(t));


	fclose(fp);



	for(j = 0; j < 10; j++)
	{
		xbt_dynar_free(&tasksManagement[0].client_requests[j]);
	}


	if (res == MSG_OK) return 0;
	else return 1;
}
