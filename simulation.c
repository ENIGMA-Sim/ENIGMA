#include "./Headers/simulation.h"
#include "./Headers/rand.h"


// Client function: Creates the requests
int iot(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0;
	char sprintf_buffer[64], mailbox[256], buf[64];
	msg_task_t task = NULL;
	struct ClientRequest *req;
	struct ServerResponse *resServer;
	struct ControllerResponse *conResponse;
	double t_arrival, t, percentage, arrival;
	int my_iot_cluster, my_device, dispatcher, num_tasks, size_request, num_datacenters, res, k, tasks_sent, tasks_completed;

	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	size_request = atoi(argv[2]);
	num_tasks = atoi(argv[3]);
	percentage = atof(argv[4]);
	num_datacenters = atoi(argv[5]);
	arrival = atof(argv[6]);


	sprintf(buf, "iot-%d-%d", my_iot_cluster,my_device);
	MSG_mailbox_set_async(buf); //mailbox asincrono

	msg_host_t host = MSG_host_by_name(buf);


	if (percentage == 1)								//If percentage is equal to 1 then the tasks execute on the devices
	{
		for (k = 0; k < num_tasks; k++)
		{
			MSG_host_set_pstate(host, 0);
			sprintf(sprintf_buffer, "Task_%d_%d_%d", my_iot_cluster, my_device, k);
			double start = MSG_get_clock();
			msg_task_t taskLocally = NULL;
			double serviceLocally = 1E06;
			taskLocally = MSG_task_create(sprintf_buffer, serviceLocally, size_request, NULL);
			MSG_task_execute(taskLocally);
			MSG_task_destroy(taskLocally);
			statsIoT[my_iot_cluster].avTime[my_device] += (MSG_get_clock() - start);
			statsIoT[my_iot_cluster].numTasks[my_device] += 1;
			MSG_host_set_pstate(host, 2);
			//printf("%d. Total energy dissipated = %.0f J\n\n",k,my_iot_cluster,my_device, sg_host_get_consumed_energy(host));
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

			req->t_arrival = MSG_get_clock(); 				// tiempo de llegada
			req->finish_controller = 0;
			req->iot_cluster = my_iot_cluster;
			req->device = my_device;

			t = exponential((double)SERVICE_RATE);

			//req->t_service = MFLOPS_BASE * t; 			// calculo del tiempo de servicio en funcion de la velocidad del host del servidor
			req->t_service = 1E06;


			if (percentage != 0)							//The devices compute locally part of the tasks
			{
				MSG_host_set_pstate(host, 0);
				msg_task_t taskLocally = NULL;
				double serviceLocally = req->t_service * percentage;
				taskLocally = MSG_task_create(sprintf_buffer, serviceLocally, size_request, NULL);
				MSG_task_execute(taskLocally);
				MSG_task_destroy(taskLocally);
				req->t_service = req->t_service - serviceLocally;
				taskLocally = NULL;
				MSG_host_set_pstate(host, 2);
			}

			req->n_task = k;
			task_comp_size = req->t_service;
			task_comm_size = size_request;

			task = MSG_task_create(sprintf_buffer, task_comp_size, task_comm_size, NULL);
			MSG_task_set_data(task, (void *)req);

			dispatcher = uniform_int(0, num_datacenters-1);
			sprintf(mailbox, "d-%d-0", dispatcher);
			MSG_task_send(task, mailbox);
			task = NULL;
			statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		}

		while(1)
		{
			
			res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));

			if (res == MSG_OK)
			{
				resServer = MSG_task_get_data(task);
				//printf("%s completed on server %d-%d\n",resServer->id_task, resServer->server_cluster, resServer->server);
				//printf("%d %d - %d %d\n", resServer->iot_cluster , my_iot_cluster, resServer->device , my_device);
				if(resServer->iot_cluster == my_iot_cluster && resServer->device == my_device)
				{
					tasks_completed++;
					statsIoT[my_iot_cluster].avTime[my_device] += (MSG_get_clock() - resServer->t_arrival);
				} 
				

				free(resServer);
				MSG_task_destroy(task);
				task = NULL;
			}

			//printf("%d %d - %d %d\n", my_iot_cluster , my_device, tasks_completed , num_tasks);
			if(tasks_completed == num_tasks) break;

		}

	}


		
	/* TASKS COMPLETED */




	conResponse = (struct ControllerResponse *)malloc(sizeof(struct ControllerResponse));
	conResponse->iot_cluster = my_iot_cluster;
	conResponse->device = my_device;
	conResponse->finish = 1;

	task = MSG_task_create(sprintf_buffer, task_comp_size, task_comm_size, NULL);
	MSG_task_set_data(task, (void *)conResponse);

	sprintf(mailbox, "cont-0");
	MSG_task_send(task, mailbox);
	task = NULL;

	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	//printf("Device %d-%d shutting down\n",my_iot_cluster,my_device);

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
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if (res != MSG_OK) break;
		req = MSG_task_get_data(task);


		// copia la tarea en otra
		new_task = MSG_task_create(MSG_task_get_name(task), MSG_task_get_flops_amount(task), 0, NULL);
		MSG_task_set_data(new_task, (void *)req);
		MSG_task_destroy(task);
		task = NULL;

		datacenter = my_d; 

		if(req->finish_controller == 0)
		{
			server = uniform_int(0, atoi(argv[1])-1);						//argv[1] == nservers [datacenter]

			//printf("Tarea %s enviada al servidor s-%d-%d\n", req->id_task, datacenter, server);

			sprintf(mailbox, "s-%d-%d", datacenter, server);
			MSG_task_send(new_task, mailbox);
		}
		else
		{
			for (int i = 0; i < atoi(argv[1]); i++)
			{
				sprintf(mailbox, "s-%d-%d", datacenter, i);
				MSG_task_send(new_task, mailbox);
			}
			break;
		}
		

	}
	return 0;
}




/* datacenter function  */
int datacenter(int argc, char *argv[])
{
	msg_task_t task = NULL, t = NULL;
	struct ClientRequest *req;
	int res, my_datacenter, my_server;
	char buf[64], hostS[30];
	double time_task = MSG_get_clock();

	my_datacenter = atoi(argv[0]);
	my_server = atoi(argv[1]);
	sprintf(buf, "s-%d-%d", my_datacenter, my_server);
	MSG_mailbox_set_async(buf);

	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	statsDatacenter[my_datacenter].numTasks[my_server] = 0;

	while (1)
	{
		
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		
		if (res != MSG_OK) break;
	
		req = MSG_task_get_data(task);

		if(req->finish_controller == 1) 
		{
			//printf("Server %d-%d shutting down. Total energy dissipated = %.0f J\n\n",my_datacenter,my_server, sg_host_get_consumed_energy(host));
			statsDatacenter[my_datacenter].avEnergy[my_server] = (sg_host_get_consumed_energy(host)/statsDatacenter[my_datacenter].numTasks[my_server]);
			statsDatacenter[my_datacenter].totalEnergy[my_server] = sg_host_get_consumed_energy(host);
			statsDatacenter[my_datacenter].avTime[my_server] = statsDatacenter[my_datacenter].avTime[my_server] / statsDatacenter[my_datacenter].numTasks[my_server];
			break;
		}

		//req->t_arrival = MSG_get_clock();

		// FCFS

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
	
	MSG_host_set_pstate(host, 2);

	return 0;
}



/* dispatcher datacenter function  */
int dispatcherDatacenter(int argc, char *argv[])
{
	int res;
	char mailbox[64];
	struct ClientRequest *req;
	struct ServerResponse *resServer;
	msg_task_t task = NULL, ans_task = NULL;
	double Nqueue_avg = 0.0, Nsystem_avg = 0.0, c;
	int n_tasks = 0, my_datacenter, my_server;
	
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

		MSG_host_set_pstate(host, 0);
		task = MSG_task_create("task", req->t_service, 0, NULL);
		MSG_task_execute(task);

		
		//printf("Task done. Duration: %.2f s. Current peak speed=%.0E flop/s; Energy dissipated=%.0f J\n", MSG_host_get_name(host), MSG_get_clock() - req->t_arrival, MSG_host_get_speed(host), sg_host_get_consumed_energy(host));

		MSG_host_set_pstate(host, 2);


		xbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);
		tasksManagement[my_datacenter].Nsystem[my_server]--; // un elemento menos en el sistema
		xbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);
		

		c = MSG_get_clock(); // tiempo de terminacion de la tarea
		
		resServer = (struct ServerResponse *)malloc(sizeof(struct ServerResponse));
		statsDatacenter[my_datacenter].avTime[my_server] += (c - (req->t_arrival));
		statsDatacenter[my_datacenter].numTasks[my_server] += 1;
		resServer->server_cluster = my_datacenter;
		resServer->server = my_server;
		resServer->iot_cluster = req->iot_cluster;
		resServer->device = req->device;


		ans_task = MSG_task_create(MSG_task_get_name(task), MSG_task_get_flops_amount(task), output_size_data, NULL);
		sprintf(resServer->response, "Task %s finished", req->id_task);
		resServer->t_arrival = req->t_arrival;
		sprintf(resServer->id_task,"%s",req->id_task);
		sprintf(mailbox, "iot-%d-%d", req->iot_cluster, req->device);
		MSG_task_set_data(ans_task, (void *)resServer);

		/*Reenvio de terminacion de la tarea al cliente*/
		MSG_task_send(ans_task, mailbox);
		MSG_task_destroy(task);
	}
}




/* controller function  */
int controller(int argc, char *argv[])
{

	int res, devices_finished = 0;
	msg_task_t task = NULL;
	char mailbox[64], buffer[64], dispatcher[30];
	struct ClientRequest *req;
	struct ControllerResponse *response;
	int total_devices = atoi(argv[0]);
	int total_datacenters = atoi(argv[1]);


	sprintf(mailbox, "cont-0");
	MSG_mailbox_set_async(mailbox);


	while(1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));

		if(res != MSG_OK) break;

		response = MSG_task_get_data(task);

		if (response->finish == 1) devices_finished++;
		
		task = NULL;
		
		//printf("%d %d\n",devices_finished, total_devices);
		if (devices_finished == total_devices)
		{
			req = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
			sprintf(buffer, "Finish");
			sprintf(req->id_task, "%s", buffer);
			req->finish_controller = 1;

			for(int l = 0; l < total_datacenters; l++)
			{
				task = MSG_task_create(req->id_task, 0, 0, NULL);
				MSG_task_set_data(task, (void *)req);
				sprintf(dispatcher, "d-%d-0", l);
				MSG_task_send(task, dispatcher);
				
			}
			MSG_task_destroy(task);
			task = NULL;
			break;
		}

	}


	return 0;

}
