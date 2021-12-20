#include "./simulation.h"
#include "./rand.h"

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
#define SENDPERCENTAGE 100

int init = 0;
xbt_mutex_t m;
xbt_cond_t con;
int iot_finished = 0;
int edge_finished = 0;
int fog_finished = 0;

FILE *trucks;

double fog_x[FOG0];
double fog_y[FOG0];

struct pos{
	double x;
	double y;
};

int iot0(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot1(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot2(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot3(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot4(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}
	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot5(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot6(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot7(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int iot8(int argc, char *argv[])
{
	double task_comp_size = 0, task_comm_size = 0, t_arrival, t, i = 0;
	int my_iot_cluster, my_device, res, iot_x, iot_y, j = 0, track, org;
	char sprintf_buffer[40], mailbox[256], buf[64];
	msg_task_t task = NULL;
	
	my_iot_cluster = atoi(argv[0]);
	my_device = atoi(argv[1]);
	track = atoi(argv[2]);
	org = atoi(argv[3]);

	struct pos position;
	position_sensor_x_y (track, org, j, &position);

	iot_x = position.x;
	iot_y = position.y;

	sprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);
	MSG_mailbox_set_async(buf);
	msg_host_t host = MSG_host_by_name(buf);
	MSG_host_set_pstate(host, 2);

	while(1)
	{
		int value_arr = position_sensor_x_y (track, org, j, &position);
		if (value_arr == 1)break;
		
		MSG_host_set_pstate(host, 0);
		
		struct ClientRequest *req0;
		req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));

		sprintf(sprintf_buffer, "Data");
		sprintf(req0->id_task, "%s", sprintf_buffer);
		req0->device = track;
		req0->edge_usage = 0.5;
		req0->fog_usage = 0.5;
		//req0->n_task = i;
		req0->size_task = 4096;
		req0->t_service = 50000.0;
		req0->org = org;
		req0->temp = ((double)rand() * ( 45.0 - 5.0 ) ) / (double)RAND_MAX + 5.0;
		req0->people = (int)(rand() % (60 - 5 + 1)) + 5;
		req0->coord_x = position.x;
		req0->coord_y = position.y;
		req0->gps = 0;

		if(my_device == 0)
		{
			MSG_process_sleep(1);
			j = j + 1;
		}
		else if(my_device == 1)
		{
			MSG_process_sleep(5);
			j = j + 5;
		}
		else
		{
			MSG_process_sleep(30);
			req0->gps = 1;
			j = j + 30;
		}

		task = MSG_task_create("Data", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req0);

		int subestation = get_subestation(req0->coord_x, req0->coord_y);

		if(rand() % 100 <= SENDPERCENTAGE)
		{
			sprintf(mailbox, "fog-0-%d", subestation);
			MSG_task_send(task, mailbox);		
		}
			
		MSG_host_set_pstate(host, 2);
		task = NULL;
		statsIoT[my_iot_cluster].numTasks[my_device] += 1;
		i++;
	}

	iot_terminated();
	statsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);
	statsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];
	statsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];
	return 0;
}

int fog0(int argc, char *argv[])
{
	int res, my_d, server = 0, length = 0, my_dis, j = 0;
	char buf[64], buf2[40], mailbox[64];
	msg_task_t task = NULL, new_task = NULL, host = NULL;
	struct ClientRequest *req, reqAux;

	my_d = atoi(argv[0]);
	my_dis = atoi(argv[1]);
	sprintf(buf, "fog-0-%d", my_d, my_dis);
	MSG_mailbox_set_async(buf);
	host = MSG_host_by_name(buf);

	statsFog[my_d].numTasks[my_dis] = 0;
	statsFog[my_d].executionTime[my_dis] = 0.0;
	statsFog[my_d].dataProcessed[my_dis] = 0.0;
	MSG_host_set_pstate(host, 2);

	while (1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if (res != MSG_OK) break;

		if (strcmp(MSG_task_get_name(task), "Finalize") == 0)
		{
			MSG_task_destroy(task);
			task = NULL;
			break;
		}

		if (strcmp(MSG_task_get_name(task), "Data") == 0)
		{
			double c = MSG_get_clock();
			req = MSG_task_get_data(task);
			reqAux = *req;
			free(req);

			if (reqAux.fog_usage > 0)
			{
				MSG_host_set_pstate(host,0);
				new_task = NULL;
				double proc = reqAux.t_service * reqAux.fog_usage;
				double data = reqAux.size_task * reqAux.fog_usage;
				new_task = MSG_task_create("Data", proc, data, NULL);
				MSG_task_execute(new_task);
				MSG_task_destroy(new_task);
				MSG_host_set_pstate(host, 2);
			}

			MSG_task_destroy(task);
			task = NULL;

			struct ClientRequest *req0;
			req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
			*req0 = reqAux;
			req0->t_arrival = MSG_get_clock();
			req0->t_service = req0->t_service * (1 - req0->fog_usage);
			req0->size_task = req0->size_task * (1 - req0->fog_usage);
			
			new_task = NULL;
			new_task = MSG_task_create("Data", req0->t_service, req0->size_task, NULL);
			MSG_task_set_data(new_task, (void *)req0);
			sprintf(mailbox, "edge-%d-%d", req0->org, j % EDGE0);
			//printf("%s\n", mailbox);
			MSG_task_send(new_task, mailbox);

			statsFog[my_d].numTasks[my_dis] += 1;
			statsFog[my_d].executionTime[my_dis] += (MSG_get_clock() - c);
			statsFog[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.fog_usage;
			j++;
		}
	}

	statsFog[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsFog[my_d].numTasks[my_dis]);
	statsFog[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);
	if (statsFog[my_d].numTasks[my_dis] == 0) statsFog[my_d].executionTime[my_dis] = 0;
	edge_terminated();

	return 0;
}

int edge0(int argc, char *argv[])
{
	int res, my_d, server = 0, length = 0, my_dis, j = 0;
	char buf[64], buf2[40], mailbox[64];
	msg_task_t task = NULL, new_task = NULL, host = NULL;
	struct ClientRequest *req, reqAux;

	my_d = atoi(argv[0]);
	my_dis = atoi(argv[1]);
	sprintf(buf, "edge-%d-%d", my_d, my_dis);
	MSG_mailbox_set_async(buf);
	host = MSG_host_by_name(buf);

	statsEdge[my_d].numTasks[my_dis] = 0;
	statsEdge[my_d].executionTime[my_dis] = 0.0;
	statsEdge[my_d].dataProcessed[my_dis] = 0.0;
	MSG_host_set_pstate(host, 2);

	while (1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if (res != MSG_OK) break;

		if (strcmp(MSG_task_get_name(task), "Finalize") == 0)
		{
			MSG_task_destroy(task);
			task = NULL;
			break;
		}

		if (strcmp(MSG_task_get_name(task), "Data") == 0)
		{
			double c = MSG_get_clock();
			req = MSG_task_get_data(task);
			reqAux = *req;
			free(req);

			if (reqAux.edge_usage > 0)
			{
				MSG_host_set_pstate(host,0);
				new_task = NULL;
				double proc = reqAux.t_service * (reqAux.edge_usage + reqAux.fog_usage);
				double data = reqAux.size_task * (reqAux.edge_usage + reqAux.fog_usage);
				new_task = MSG_task_create("Data", proc, data, NULL);
				MSG_task_execute(new_task);
				MSG_task_destroy(new_task);
				MSG_host_set_pstate(host, 2);
			}

			MSG_task_destroy(task);
			task = NULL;

			struct ClientRequest *req0;
			req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
			*req0 = reqAux;
			req0->t_arrival = MSG_get_clock();
			req0->t_service = req0->t_service * (1 - req0->edge_usage + reqAux.fog_usage);
			req0->size_task = req0->size_task * (1 - req0->edge_usage + reqAux.fog_usage);
			
			new_task = NULL;
			new_task = MSG_task_create("Data", req0->t_service, req0->size_task, NULL);
			MSG_task_set_data(new_task, (void *)req0);
			sprintf(mailbox, "cloud-0-%d", j % NODESDATACENTER0);
			MSG_task_send(new_task, mailbox);

			statsEdge[my_d].numTasks[my_dis] += 1;
			statsEdge[my_d].executionTime[my_dis] += (MSG_get_clock() - c);
			statsEdge[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.edge_usage;
			j++;
		}
	}

	statsEdge[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsEdge[my_d].numTasks[my_dis]);
	statsEdge[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);
	if (statsEdge[my_d].numTasks[my_dis] == 0) statsEdge[my_d].executionTime[my_dis] = 0;
	edge_terminated();

	return 0;
}



int edge1(int argc, char *argv[])
{
	int res, my_d, server = 0, length = 0, my_dis, j = 0;
	char buf[64], buf2[40], mailbox[64];
	msg_task_t task = NULL, new_task = NULL, host = NULL;
	struct ClientRequest *req, reqAux;

	my_d = atoi(argv[0]);
	my_dis = atoi(argv[1]);
	sprintf(buf, "edge-%d-%d", my_d, my_dis);
	MSG_mailbox_set_async(buf);
	host = MSG_host_by_name(buf);

	statsEdge[my_d].numTasks[my_dis] = 0;
	statsEdge[my_d].executionTime[my_dis] = 0.0;
	statsEdge[my_d].dataProcessed[my_dis] = 0.0;
	MSG_host_set_pstate(host, 2);

	while (1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if (res != MSG_OK) break;

		if (strcmp(MSG_task_get_name(task), "Finalize") == 0)
		{
			MSG_task_destroy(task);
			task = NULL;
			break;
		}

		if (strcmp(MSG_task_get_name(task), "Data") == 0)
		{
			double c = MSG_get_clock();
			req = MSG_task_get_data(task);
			reqAux = *req;
			free(req);

			if (reqAux.edge_usage > 0)
			{
				MSG_host_set_pstate(host,0);
				new_task = NULL;
				double proc = reqAux.t_service * (reqAux.edge_usage + reqAux.fog_usage);
				double data = reqAux.size_task * (reqAux.edge_usage + reqAux.fog_usage);
				new_task = MSG_task_create("Data", proc, data, NULL);
				MSG_task_execute(new_task);
				MSG_task_destroy(new_task);
				MSG_host_set_pstate(host, 2);
			}

			MSG_task_destroy(task);
			task = NULL;

			struct ClientRequest *req0;
			req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
			*req0 = reqAux;
			req0->t_arrival = MSG_get_clock();
			req0->t_service = req0->t_service * (1 - req0->edge_usage + reqAux.fog_usage);
			req0->size_task = req0->size_task * (1 - req0->edge_usage + reqAux.fog_usage);
			
			new_task = NULL;
			new_task = MSG_task_create("Data", req0->t_service, req0->size_task, NULL);
			MSG_task_set_data(new_task, (void *)req0);
			sprintf(mailbox, "cloud-0-%d", j % NODESDATACENTER0);
			MSG_task_send(new_task, mailbox);

			statsEdge[my_d].numTasks[my_dis] += 1;
			statsEdge[my_d].executionTime[my_dis] += (MSG_get_clock() - c);
			statsEdge[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.edge_usage;
			j++;
		}
	}

	statsEdge[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsEdge[my_d].numTasks[my_dis]);
	statsEdge[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);
	if (statsEdge[my_d].numTasks[my_dis] == 0) statsEdge[my_d].executionTime[my_dis] = 0;
	edge_terminated();

	return 0;
}

int edge2(int argc, char *argv[])
{
	int res, my_d, server = 0, length = 0, my_dis, j = 0;
	char buf[64], buf2[40], mailbox[64];
	msg_task_t task = NULL, new_task = NULL, host = NULL;
	struct ClientRequest *req, reqAux;

	my_d = atoi(argv[0]);
	my_dis = atoi(argv[1]);
	sprintf(buf, "edge-%d-%d", my_d, my_dis);
	MSG_mailbox_set_async(buf);
	host = MSG_host_by_name(buf);

	statsEdge[my_d].numTasks[my_dis] = 0;
	statsEdge[my_d].executionTime[my_dis] = 0.0;
	statsEdge[my_d].dataProcessed[my_dis] = 0.0;
	MSG_host_set_pstate(host, 2);

	while (1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if (res != MSG_OK) break;

		if (strcmp(MSG_task_get_name(task), "Finalize") == 0)
		{
			MSG_task_destroy(task);
			task = NULL;
			break;
		}

		if (strcmp(MSG_task_get_name(task), "Data") == 0)
		{
			double c = MSG_get_clock();
			req = MSG_task_get_data(task);
			reqAux = *req;
			free(req);

			if (reqAux.edge_usage > 0)
			{
				MSG_host_set_pstate(host,0);
				new_task = NULL;
				double proc = reqAux.t_service * (reqAux.edge_usage + reqAux.fog_usage);
				double data = reqAux.size_task * (reqAux.edge_usage + reqAux.fog_usage);
				new_task = MSG_task_create("Data", proc, data, NULL);
				MSG_task_execute(new_task);
				MSG_task_destroy(new_task);
				MSG_host_set_pstate(host, 2);
			}

			MSG_task_destroy(task);
			task = NULL;

			struct ClientRequest *req0;
			req0 = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
			*req0 = reqAux;
			req0->t_arrival = MSG_get_clock();
			req0->t_service = req0->t_service * (1 - req0->edge_usage + reqAux.fog_usage);
			req0->size_task = req0->size_task * (1 - req0->edge_usage + reqAux.fog_usage);
			
			new_task = NULL;
			new_task = MSG_task_create("Data", req0->t_service, req0->size_task, NULL);
			MSG_task_set_data(new_task, (void *)req0);
			sprintf(mailbox, "cloud-0-%d", j % NODESDATACENTER0);
			MSG_task_send(new_task, mailbox);

			statsEdge[my_d].numTasks[my_dis] += 1;
			statsEdge[my_d].executionTime[my_dis] += (MSG_get_clock() - c);
			statsEdge[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.edge_usage;
			j++;
		}
	}

	statsEdge[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsEdge[my_d].numTasks[my_dis]);
	statsEdge[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);
	if (statsEdge[my_d].numTasks[my_dis] == 0) statsEdge[my_d].executionTime[my_dis] = 0;
	edge_terminated();

	return 0;
}

int cloud0(int argc, char *argv[])
{
	msg_task_t task = NULL;
	struct ClientRequest *req;
	int res, my_datacenter = atoi(argv[1]), my_server = atoi(argv[2]);
	char buf[40], hostS[30], mailbox[64];

	sprintf(buf, "%s", argv[0]);
	msg_host_t host = MSG_host_by_name(buf);
	statsCloud[my_datacenter].numTasks[my_server] = 0;
	statsCloud[my_datacenter].executionTime[my_server] = 0.0;
	statsCloud[my_datacenter].dataProcessed[my_server] = 0.0;
	MSG_host_set_pstate(host, 2);

	while (1)
	{
		res = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));
		if(res != MSG_OK) break;
		if(strcmp(MSG_task_get_name(task), "Finalize") == 0)
		{
			MSG_task_destroy(task);
			task = NULL;
			break;
		}
		if(strcmp(MSG_task_get_name(task), "Data") == 0)
		{
			req = MSG_task_get_data(task);
			MSG_task_destroy(task);
			task = NULL;

			if (req->gps == 1)
			{
				struct InfoTruck *truck;
				truck = (struct InfoTruck *)malloc(sizeof(struct InfoTruck));

				truck->t_arrival = req->t_arrival;
				truck->t_service = req->t_service;
				truck->device = req->device;
				truck->size_task = req->size_task;
				truck->temp = req->temp;
				truck->people = req->people;
				truck->coord_x = req->coord_x;
				truck->coord_y = req->coord_y;
				truck->org = req->org;
				
				//printf("%f %f %d %d ", truck->t_arrival, truck->t_service, truck->device, truck->size_task);
				/*if(truck->org == 0 && truck->device == 2)
				{
					printf("%d-%d %f %f\n", truck->org, truck->device, truck->coord_x, truck->coord_y);
				}
				*/
				xbt_mutex_acquire(tasksM[my_datacenter].mutex[my_server]);
				tasksM[my_datacenter].Nqueue[my_server]++;
				tasksM[my_datacenter].Nsystem[my_server]++;
				fprintf(trucks,"%d, %d, %f, %d, %f, %f\n", truck->device, truck->org, truck->temp, truck->people, truck->coord_x, truck->coord_y);
				xbt_dynar_push(tasksM[my_datacenter].client_requests[my_server], (const char *)&truck);
				xbt_cond_signal(tasksM[my_datacenter].cond[my_server]);
				xbt_mutex_release(tasksM[my_datacenter].mutex[my_server]);
			}
		}
	}

	xbt_mutex_acquire(tasksM[my_datacenter].mutex[my_server]);
	tasksM[my_datacenter].EmptyQueue[my_server] = 1;
	xbt_cond_signal(tasksM[my_datacenter].cond[my_server]);
	xbt_mutex_release(tasksM[my_datacenter].mutex[my_server]);
	return 0;
}

int dispatcherCloud0(int argc, char *argv[])
{
	char mailbox[64], hostS[30], vm_name[30];
	struct InfoTruck *req, reqAux;
	//struct ClientRequest *req, reqAux;
	msg_task_t task = NULL;
	double Nqueue_avg = 0.0, Nsystem_avg = 0.0, c = 0.0, wait = 0.0;
	int n_tasks = 0, my_datacenter = atoi(argv[1]), my_server = atoi(argv[2]), opt_vm = 0, res = 0;

	sprintf(hostS, "%s", argv[0]);
	msg_host_t host = MSG_host_by_name(hostS);
	double c1 = MSG_get_clock();
	double ca = 0.0;

	while(1)
	{
		xbt_mutex_acquire(tasksM[my_datacenter].mutex[my_server]);
		
		while((tasksM[my_datacenter].Nqueue[my_server] == 0) && (tasksM[my_datacenter].EmptyQueue[my_server] == 0))
		{
			xbt_cond_wait(tasksM[my_datacenter].cond[my_server], tasksM[my_datacenter].mutex[my_server]);
		}
		
		if((tasksM[my_datacenter].EmptyQueue[my_server] == 1) && (tasksM[my_datacenter].Nqueue[my_server] == 0))
		{
			statsCloud[my_datacenter].avEnergy[my_server] = (sg_host_get_consumed_energy(host)/statsCloud[my_datacenter].numTasks[my_server]);
			statsCloud[my_datacenter].totalEnergy[my_server] = sg_host_get_consumed_energy(host);
			xbt_mutex_release(tasksM[my_datacenter].mutex[my_server]);
			break;
		}
		
		xbt_dynar_shift(tasksM[my_datacenter].client_requests[my_server], (char *)&req);
		tasksM[my_datacenter].Nqueue[my_server]--;
		n_tasks++;
		tasksM[my_datacenter].Navgqueue[my_server] = (tasksM[my_datacenter].Navgqueue[my_server] * (n_tasks - 1) + tasksM[my_datacenter].Nqueue[my_server]) / n_tasks;
		tasksM[my_datacenter].Navgsystem[my_server] = (tasksM[my_datacenter].Navgsystem[my_server] * (n_tasks - 1) + tasksM[my_datacenter].Nsystem[my_server]) / n_tasks;
		xbt_mutex_release(tasksM[my_datacenter].mutex[my_server]);
		
		MSG_host_set_pstate(host, 0);
		
		msg_vm_t vm;
		size_t ram;
		sprintf(vm_name, "vm-%d-%d",my_datacenter,my_server);
		vm = MSG_vm_create_multicore(host, vm_name, 16);
		ram = 32 * 1024;
		MSG_vm_set_ramsize(vm, ram);
		MSG_vm_start(vm);
		
		reqAux = *req;
		free(req);
		//printf("A\n");
		task = MSG_task_create("Data", reqAux.t_service, reqAux.size_task, &reqAux);
		c = MSG_get_clock();
		MSG_task_execute(task);
		ca = ca + MSG_get_clock() - c;
		MSG_task_destroy(task);
		task = NULL;
		MSG_host_set_pstate(host, 2);
		wait = MSG_get_clock();

		MSG_vm_shutdown(vm);
		MSG_vm_destroy(vm);

		xbt_mutex_acquire(tasksM[my_datacenter].mutex[my_server]);
		tasksM[my_datacenter].Nsystem[my_server]--;
		statsCloud[my_datacenter].numTasks[my_server] += 1;
		statsCloud[my_datacenter].dataProcessed[my_server] += reqAux.size_task;
		xbt_mutex_release(tasksM[my_datacenter].mutex[my_server]);

	}

	double c2 = MSG_get_clock() - c1;  // felix
	statsCloud[my_datacenter].executionTime[my_server] = ca;
}


int controller(int argc, char *argv[])
{
	int res, total_iot = 27, total_nodes = 20, total_edge_nodes = 30, total_fog_devices = 0;
	msg_task_t task = NULL;
	char mailbox[64], buffer[64], node[30];
	struct ClientRequest *req;
	struct ControllerResponse *response;

	sprintf(mailbox, "cont-0");
	MSG_mailbox_set_async(mailbox);
	init_m();

	xbt_mutex_acquire(m);
	while(total_iot != iot_finished)
	{
		xbt_cond_wait(con,m);
	}
	xbt_mutex_release(m);

	req = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
	sprintf(buffer, "Finalize");
	sprintf(req->id_task, "%s", buffer);

	for (int i = 0; i < EDGE0; i++)
	{
		task = MSG_task_create("Finalize", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req);
		sprintf(node, "edge-0-%d", i);
		MSG_task_send(task, node);
	}

	for (int i = 0; i < EDGE1; i++)
	{
		task = MSG_task_create("Finalize", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req);
		sprintf(node, "edge-1-%d", i);
		MSG_task_send(task, node);
	}

	for (int i = 0; i < EDGE2; i++)
	{
		task = MSG_task_create("Finalize", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req);
		sprintf(node, "edge-2-%d", i);
		MSG_task_send(task, node);
	}

	xbt_mutex_acquire(m);
	while(total_edge_nodes != edge_finished)
	{
		xbt_cond_wait(con,m);
	}
	xbt_mutex_release(m);


	req = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));
	sprintf(buffer, "Finalize");
	sprintf(req->id_task, "%s", buffer);

	for (int i = 0; i < FOG0; i++)
	{
		task = MSG_task_create("Finalize", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req);
		sprintf(node, "fog-0-%d", i);
		MSG_task_send(task, node);
	}

	xbt_mutex_acquire(m);
	while(total_fog_devices != fog_finished)
	{
		xbt_cond_wait(con,m);
	}
	xbt_mutex_release(m);


	for (int i = 0; i < NODESDATACENTER0; i++)
	{
		task = MSG_task_create("Finalize", 0, 0, NULL);
		MSG_task_set_data(task, (void *)req);
		sprintf(node, "cloud-0-%d", i);
		MSG_task_send(task, node);
	}

	fclose(trucks);

	return 0;
}

void init_m(void)
{
	if(init == 0)
	{
		init = 1;
		m = xbt_mutex_init();
		con = xbt_cond_init();

		trucks = fopen("./Results/traces-30s.csv", "w+");
		fprintf(trucks,"Truck, Organization, Temperature (ºC), # People, Latitude, Longitude\n");

		char filename[30], line[100];
		sprintf(filename, "./Mobility/substations.csv");
		FILE *file = fopen(filename, "r");

		int count = 0, x = 1, y = 3, i = 0;
		char delim[] = ",";
		
		while (fgets(line, sizeof line, file) != NULL) /* read a line */
	    {
	    	for (char *p = strtok(line, delim); p != NULL; p = strtok(NULL, delim))
			{
				if ( i == x ) sscanf(p, "%lf", &fog_x[count]);
       			if ( i == y )
            	{
            		sscanf(p, "%lf", &fog_y[count]);
            		break;
            	}
				i++;
			}
        	i = 0;
            count++;
	    }

	    fclose(file);
	}
}

void iot_terminated (void)
{
	xbt_mutex_acquire(m);
	iot_finished += 1;
	xbt_cond_signal(con);
	xbt_mutex_release(m);
}

void edge_terminated (void)
{
	xbt_mutex_acquire(m);
	edge_finished += 1;
	xbt_cond_signal(con);
	xbt_mutex_release(m);
}

void fog_terminated (void)
{
	xbt_mutex_acquire(m);
	fog_finished += 1;
	xbt_cond_signal(con);
	xbt_mutex_release(m);
}


int position_sensor_x_y (int truck, int organization, int position, struct pos * pos_sensor)
{
	char filename[20], line[50], delim[] = ",";
	int count = 0, i = 0;

	sprintf(filename, "./Mobility/%d_%d.csv", organization, truck);
	FILE *file = fopen(filename, "r");

	while (fgets(line, sizeof line, file) != NULL) 
    {
    	if ( position + 2 == count )
        {
        	for (char *p = strtok(line, delim); p != NULL; p = strtok(NULL, delim))
			{
				if ( i == 0 )sscanf(p, "%lf", &pos_sensor->x);
					
	        	if ( i == 1 )
	        	{
	        		sscanf(p, "%lf", &pos_sensor->y);
	        		fclose(file);
	        		return 0;
	        	} 
				
				i++;
			}
        }
        count++;  
    }
    fclose(file);
    return 1;				//The truck has arrived at its destination
}


int get_subestation (double x, double y)		//Function that allows to obtain the closest substation to the truck.
{
	double lowestDistance = sqrt( (fog_x[0] - x)*(fog_x[0] - x) + (fog_y[0] - y)*(fog_y[0] - y) );
	int subestation = 0;

	for (int i = 1; i < FOG0; i++) {
		double dist = sqrt( (fog_x[i] - x)*(fog_x[i] - x) + (fog_y[i] - y)*(fog_y[i] - y) );
        if ( dist < lowestDistance ) 
        {
            lowestDistance = dist;
            subestation = i;
        }
    }
    return subestation;
}