
uint8_t sem_tracker=0;
sem_t sem_array[TOTAL_THREADS];

void all_sem_init(void)
{
	uint8_t i=0,error=0;
	for(i=0;i<TOTAL_THREADS;i++)
	{
		error=sem_init(&sem_array[i],0,1);
		if(error)
		{
			cout<<"Semaphore initialization failed.";
			exit(-1);
		}
	}
}

void next_sem_post(void)
{
	if(sem_tracker == TOTAL_THREADS-1)
	{
		sem_tracker = 0;
	}
	else
	{
		sem_tracker++;
	}
	sem_post(&sem_array[sem_tracker]);
	return;
}

void current_sem_wait(void)
{
	sem_wait(&sem_array[sem_tracker]);
	return;
}