#include "./Headers/simulation.h"
#include "./Headers/rand.h"

const long MAX_TIMEOUT_SERVER = (86400 * 0.001); 					//Timeout= 10 días sin actividad



// Sort two elements of type struct ClientRequest. It uses the function xbt_dynar_sort
static int sort_function_short(const void *e1, const void *e2)
{
	struct ClientRequest *c1 = *(void **)e1;
	struct ClientRequest *c2 = *(void **)e2;

	if (c1->t_service == c2->t_service)
		return 0;
	else if (c1->t_service < c2->t_service)
		return -1;
	else
		return 1;
}



// Client function: Creates the requests
int iot(int argc, char *argv[])
{
	double task_comp_size = 0;
	double task_comm_size = 0;
	char sprintf_buffer[64];
	char mailbox[256], buf[64];
	msg_task_t task = NULL;
	struct ClientRequest *req;
	struct ServerResponse *resServer;
	double t_arrival;
	int my_iot_cluster, my_device, dispatcher, num_tasks, size_request, num_datacenters;
	double t, percentage, arrival;
	int res, k;

	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	size_request = atoi(argv[2]);
	num_tasks = atoi(argv[3]);
	percentage = atof(argv[4]);
	num_datacenters = atoi(argv[5]);
	arrival = atof(argv[6]);

	sprintf(buf, "c-%d-%d", my_iot_cluster,my_device);
	MSG_mailbox_set_async(buf); //mailbox asincrono

	msg_host_t host = MSG_host_by_name(buf);
	
	if (percentage == 1)								//If percentage is equal to 1 then the tasks execute on the devices
	{
		for (k = 0; k < num_tasks; k++)
		{
			sprintf(sprintf_buffer, "Task_%d_%d_%d", my_iot_cluster, my_device, k);
			double start = MSG_get_clock();
			msg_task_t taskLocally = NULL;
			double serviceLocally = MFLOPS_BASE * exponential((double)SERVICE_RATE) * percentage;
			taskLocally = MSG_task_create(sprintf_buffer, serviceLocally, size_request, NULL);
			MSG_task_execute(taskLocally);
			MSG_task_destroy(taskLocally);

			printf("Task done in %s (duration: %.2f s). Current peak speed=%.0E flop/s; Current consumption: from %.0fW to %.0fW"
			" depending on load; Energy dissipated=%.0f J\n\n",
			MSG_host_get_name(host), MSG_get_clock() - start, MSG_host_get_speed(host), sg_host_get_wattmin_at(host, MSG_host_get_pstate(host)),
			sg_host_get_wattmax_at(host, MSG_host_get_pstate(host)), sg_host_get_consumed_energy(host));
		}
	}
	else 													//If not, the devices create the requests that execute the datacenters 
	{
		for (k = 0; k < num_tasks; k++)
		{
			req = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

			t_arrival = exponential((double)arrival);
			MSG_process_sleep(t_arrival);

			sprintf(sprintf_buffer, "Task_%d_%d_%d", my_iot_cluster, my_device, k);
			sprintf(req->id_task, "%s", sprintf_buffer);

			req->t_arrival = MSG_get_clock(); // tiempo de llegada

			req->iot_cluster = my_iot_cluster;
			req->device = my_device;

			t = exponential((double)SERVICE_RATE);

			req->t_service = MFLOPS_BASE * t; 			// calculo del tiempo de servicio en funcion
											  			// de la velocidad del host del servidor

			if (percentage != 0)					//The devices compute locally part of the tasks
			{
				double start = MSG_get_clock();
				msg_task_t taskLocally = NULL;
				double serviceLocally = MFLOPS_BASE * t * percentage;
				taskLocally = MSG_task_create(sprintf_buffer, serviceLocally, size_request, NULL);
				MSG_task_execute(taskLocally);
				MSG_task_destroy(taskLocally);
				req->t_service = req->t_service - serviceLocally;
				
				printf("Task partially done in %s (duration: %.2f s). Current peak speed=%.0E flop/s; Current consumption: from %.0fW to %.0fW"
				" depending on load; Energy dissipated=%.0f J\n\n",
				MSG_host_get_name(host), MSG_get_clock() - start, MSG_host_get_speed(host), sg_host_get_wattmin_at(host, MSG_host_get_pstate(host)),
				sg_host_get_wattmax_at(host, MSG_host_get_pstate(host)), sg_host_get_consumed_energy(host));

			}

			req->n_task = k;
			task_comp_size = req->t_service;
			task_comm_size = 0;

			task = MSG_task_create(sprintf_buffer, task_comp_size, task_comm_size, NULL);
			MSG_task_set_data(task, (void *)req);

			dispatcher = uniform_int(0, num_datacenters-1);
			sprintf(mailbox, "d-%d-0", dispatcher);
			MSG_task_send(task, mailbox);
			task = NULL;
		}

		while(1)
		{
			res = MSG_task_receive_with_timeout(&(task), MSG_host_get_name(MSG_host_self()), MAX_TIMEOUT_SERVER);
			if (res != MSG_OK) break;
			resServer = MSG_task_get_data(task);
			//printf("%s completed on server %d-%d\n",resServer->id_task, resServer->server_cluster, resServer->server);
			
			free(resServer);
			MSG_task_destroy(task);
			task = NULL;
		}

	}

	/* finalizar */
	return 0;
}





// dispatcher function, recibe las peticiones de los clientes y las envía a los servidores
int dispatcher(int argc, char *argv[])
{
	int res;
	struct ClientRequest *req;
	msg_task_t task = NULL;
	msg_task_t new_task = NULL;
	int my_d;
	char mailbox[64];
	int datacenter = 0, server = 0;
	char buf[64];

	my_d = atoi(argv[0]);
	
	sprintf(buf, "d-%d-0", my_d);
	MSG_mailbox_set_async(buf); //mailbox asincrono

	while (1)
	{
		res = MSG_task_receive_with_timeout(&(task), MSG_host_get_name(MSG_host_self()), MAX_TIMEOUT_SERVER);
		if (res != MSG_OK) break;
		req = MSG_task_get_data(task);

		// copia la tarea en otra
		new_task = MSG_task_create(MSG_task_get_name(task), MSG_task_get_flops_amount(task), 0, NULL);
		MSG_task_set_data(new_task, (void *)req);
		MSG_task_destroy(task);
		task = NULL;

		datacenter = my_d; 
		server = uniform_int(0, atoi(argv[1])-1);						//argv[1] == nservers [datacenter]

		//printf("Tarea %s enviada al servidor s-%d-%d\n", req->id_task, datacenter, server);

		sprintf(mailbox, "s-%d-%d", datacenter, server);
		MSG_task_send(new_task, mailbox);
	}
	return 0;
}







/* server function  */
int datacenter(int argc, char *argv[])
{
	msg_task_t task = NULL;
	msg_task_t t = NULL;
	struct ClientRequest *req;
	int res;
	int my_datacenter, my_server;
	char buf[64];

	my_datacenter = atoi(argv[0]);
	my_server = atoi(argv[1]);
	sprintf(buf, "s-%d-%d", my_datacenter, my_server);
	MSG_mailbox_set_async(buf);

	

	while (1)
	{
		res = MSG_task_receive_with_timeout(&(task), MSG_host_get_name(MSG_host_self()), MAX_TIMEOUT_SERVER);
		if (res != MSG_OK) break;
		req = MSG_task_get_data(task);

		// inserta la petición en la cola
		xbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);
		tasksManagement[my_datacenter].Nqueue[my_server]++;  // un elemento mas en la cola
		tasksManagement[my_datacenter].Nsystem[my_server]++; // un elemento mas en el sistema

		xbt_dynar_push(tasksManagement[my_datacenter].client_requests[my_server], (const char *)&req);
		xbt_cond_signal(tasksManagement[my_datacenter].cond[my_server]); // despierta al proceso server
		xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);
		MSG_task_destroy(task);
		task = NULL;
	}

	// marca el fin
	xbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);
	tasksManagement[my_datacenter].EmptyQueue[my_server] = 1;
	xbt_cond_signal(tasksManagement[my_datacenter].cond[my_server]);
	xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);

	return 0;
}



/* server function  */
int dispatcherDatacenter(int argc, char *argv[])
{
	int res;
	char mailbox[64];
	struct ClientRequest *req;
	struct ServerResponse *resServer;
	msg_task_t task = NULL;
	msg_task_t ans_task = NULL;
	double Nqueue_avg = 0.0;
	double Nsystem_avg = 0.0;
	double c;
	int n_tasks = 0;
	int my_datacenter, my_server;
	
	my_datacenter = atoi(argv[0]);
	my_server = atoi(argv[1]);
	int output_size_data = atoi(argv[2]);

	char hostS[30];
	sprintf(hostS,"s-%d-%d",my_datacenter,my_server);
	msg_host_t host = MSG_host_by_name(hostS);

	while (1)
	{

		xbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);

		while ((tasksManagement[my_datacenter].Nqueue[my_server] == 0) && (tasksManagement[my_datacenter].EmptyQueue[my_server] == 0))
		{
			xbt_cond_wait(tasksManagement[my_datacenter].cond[my_server], tasksManagement[my_datacenter].mutex[my_server]);
		}

		if ((tasksManagement[my_datacenter].EmptyQueue[my_server] == 1) && (tasksManagement[my_datacenter].Nqueue[my_server] == 0))
		{
			xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);
			break;
		}

		// extrae un elemento de la cola
		xbt_dynar_shift(tasksManagement[my_datacenter].client_requests[my_server], (char *)&req);
		tasksManagement[my_datacenter].Nqueue[my_server]--; // un elemento menos en la cola
		n_tasks++;

		// calculo de estadisticas
		tasksManagement[my_datacenter].Navgqueue[my_server] = (tasksManagement[my_datacenter].Navgqueue[my_server] * (n_tasks - 1) + tasksManagement[my_datacenter].Nqueue[my_server]) / n_tasks;
		tasksManagement[my_datacenter].Navgsystem[my_server] = (tasksManagement[my_datacenter].Navgsystem[my_server] * (n_tasks - 1) + tasksManagement[my_datacenter].Nsystem[my_server]) / n_tasks;
		xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);

		// crea una tarea para su ejecución
		task = MSG_task_create("task", req->t_service, 0, NULL);
		MSG_task_execute(task);

		



		printf("Task done in %s (duration: %.2f s). Current peak speed=%.0E flop/s; Current consumption: from %.0fW to %.0fW"
		" depending on load; Energy dissipated=%.0f J\n\n",
		MSG_host_get_name(host), MSG_get_clock() - req->t_arrival, MSG_host_get_speed(host), sg_host_get_wattmin_at(host, MSG_host_get_pstate(host)),
		sg_host_get_wattmax_at(host, MSG_host_get_pstate(host)), sg_host_get_consumed_energy(host));



		xbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);
		tasksManagement[my_datacenter].Nsystem[my_server]--; // un elemento menos en el sistema
		xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);
		c = MSG_get_clock(); // tiempo de terminacion de la tarea
		
		resServer = (struct ServerResponse *)malloc(sizeof(struct ServerResponse));
		avServTime[my_datacenter].avServiceTime += (c - (req->t_arrival));
		avServTime[my_datacenter].numTasks += 1;
		resServer->server_cluster = my_datacenter;
		resServer->server = my_server;

		ans_task = MSG_task_create(MSG_task_get_name(task), MSG_task_get_flops_amount(task), output_size_data, NULL);
		sprintf(resServer->response, "Task finished on %d-%d", resServer->server_cluster, resServer->server);

		sprintf(resServer->id_task,"%s",req->id_task);
		sprintf(mailbox, "c-%d-%d", req->iot_cluster, req->device);
		MSG_task_set_data(ans_task, (void *)resServer);

		/*Reenvio de terminacion de la tarea al cliente*/
		MSG_task_send(ans_task, mailbox);
		MSG_task_destroy(task);
	}
}

