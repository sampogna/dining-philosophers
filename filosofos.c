#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define _CRT_SECURE_NO_WARNINGS 
//#pragma warning(disable:4996)

#define TIME_THINKING 2 
#define TIME_EATING 2 


/* BEGIN - STRUCTS */

typedef struct phil {
	//Condicao de parada
	int cond_stop;

	//Mutexes
	pthread_mutex_t id_mtx;
	pthread_mutex_t control_print;
	pthread_mutex_t control_whos_is_eating;
	pthread_mutex_t* fork;

	//Semaforos
	pthread_cond_t* forkCond;
	pthread_cond_t* philCond;

	//Indicadores do filosofo
	int id; //Id do filosofo
	int* who_is_eating; //Lista que vai apontar quem esta comendo no momento
	int* consume_totals; // Lista que vai guardar totalizadores de quanto cada filoso comeu
	int* forks_being_used; //Lista que vai guardar quais garfos estao sendo utilizados

	//Controles pegos em linha de comando
	int max_iterations;
	int philosopher_number;

} Philosopher;

typedef struct consume {
	int id;
	int total_consume;
} Consume_Philosopher;

/* END - STRUCTS */

/* BEGIN - METHODS */

int check_if_nobody_is_eating(int current_id, int* who_is_eating, int philosopher_number) {
	for (int i = 0; i < philosopher_number; i++) {
		if (who_is_eating[i] == 1 && i != current_id) {
			return 0;
		}
	}
	return 1;
}

void print_whos_eating_with_me(int current_id, int* who_is_eating, pthread_mutex_t control_who_is_eating, int philosopher_number) {
	pthread_mutex_lock(&control_who_is_eating);
	if (check_if_nobody_is_eating(current_id, who_is_eating, philosopher_number) == 0) {
		printf("\nFilosofo %d esta comendo ao mesmo tempo que os filosofos ", current_id + 1);
		for (int i = 0; i < philosopher_number; i++) {
			if (who_is_eating[i] == 1 && i != current_id) {
				printf("%d,", i + 1);
			}
		}
		printf("\b.");
	}

	pthread_mutex_unlock(&control_who_is_eating);
}

void print_consume_totals(Consume_Philosopher* consume_p, int philosopher_number) {
	for (int i = 0; i < philosopher_number; i++) {
		printf("\nFilosofo %d comeu %d vezes.", consume_p[i].id + 1, consume_p[i].total_consume);
	}
}

Consume_Philosopher create_consume_struct(int current_id, int consume_totals) {
	Consume_Philosopher p;
	p.id = current_id;
	p.total_consume = consume_totals;

	return p;
}

void take_fork(void* arg, int right_fork, int left_fork) {
	Philosopher* p = (Philosopher*)arg;

	p->forks_being_used[right_fork] = 1;
	p->forks_being_used[left_fork] = 1;

}

void leave_fork(void* arg, int right_fork, int left_fork) {
	Philosopher* p = (Philosopher*)arg;

	p->forks_being_used[right_fork] = 0;
	p->forks_being_used[left_fork] = 0;
}

void* philosopher_existence(void* arg) {
	Philosopher* p = (Philosopher*)arg;

	pthread_mutex_lock(&p->id_mtx);
	int current_id = p->id;
	p->id++;
	pthread_mutex_unlock(&p->id_mtx);

	int right_fork = p->philosopher_number - 1;
	int left_fork = current_id;
	if (current_id == (p->philosopher_number - 1)) {
		right_fork = 0;
		left_fork = p->philosopher_number - 1;
	}
	else {
		right_fork = (current_id + 1) % p->philosopher_number;
		left_fork = current_id;
	}


	int rigth_person = right_fork;
	int left_person = left_fork - 1;
	if (current_id == 0) {
		left_person = p->philosopher_number - 1;
	}

	while (p->cond_stop == 0) {

		if (p->forks_being_used[right_fork] == 0 && p->forks_being_used[left_fork] == 0) {

			//Pegar garfo
			take_fork(p, right_fork, left_fork);


			/* Verificando condição de parada */
			p->consume_totals[current_id]++;
			if (p->consume_totals[current_id] == p->max_iterations) {
				p->cond_stop = 1;
			}
			p->who_is_eating[current_id] = 1;


			print_whos_eating_with_me(current_id, p->who_is_eating, p->control_whos_is_eating, p->philosopher_number);

			p->who_is_eating[current_id] = 0;

			//Larga garfo
			leave_fork(p, right_fork, left_fork);


			pthread_cond_signal(&p->philCond[left_person]);
			pthread_cond_signal(&p->philCond[rigth_person]);

			if (p->cond_stop == 0) {
				pthread_cond_wait(&p->philCond[current_id], &p->fork[current_id]);
			}
			else {
				//Acordando todos os filosofos
				for (int i = 0; i < p->philosopher_number; i++) {
					pthread_cond_signal(&p->philCond[i]);
				}
			}
		}
		else {
			pthread_cond_wait(&p->philCond[current_id], &p->fork[current_id]);
		}

	}

	//Acordando todos os filosofos
	for (int i = 0; i < p->philosopher_number; i++) {
		pthread_cond_signal(&p->philCond[i]);
	}
	return NULL;
}

void think() {
	Sleep(TIME_THINKING);
}

void eat() {
	Sleep(TIME_EATING);
}

/* END - METHODS */

int main(int argc, char* argv[]) {


	/* BEGIN - Coletando argumentos em linha de comando */

	int philosophers_number = atoi(argv[1]);
	int max_iterations = atoi(argv[2]);

	/* END - Coletando argumentos em linha de comando */


	/* BEGIN - Secao de inicializacao das variaveis */

	int i;


	Philosopher p;


	p.philosopher_number = philosophers_number;
	p.max_iterations = max_iterations;

	pthread_t* thread_philosopher = malloc(p.philosopher_number * sizeof(pthread_t));

	p.fork = malloc(p.philosopher_number * sizeof(pthread_mutex_t));
	p.forkCond = malloc(p.philosopher_number * sizeof(pthread_cond_t));
	p.philCond = malloc(p.philosopher_number * sizeof(pthread_cond_t));
	p.who_is_eating = malloc(p.philosopher_number * sizeof(int));
	p.consume_totals = malloc(p.philosopher_number * sizeof(int));
	p.forks_being_used = malloc(p.philosopher_number * sizeof(int));

	pthread_mutex_init(&p.id_mtx, NULL);
	pthread_mutex_init(&p.control_print, NULL);
	pthread_mutex_init(&p.control_whos_is_eating, NULL);

	for (i = 0; i < p.philosopher_number; i++) {

		pthread_cond_init(&p.philCond[i], NULL);
		pthread_mutex_init(&p.fork[i], NULL);
		
		p.consume_totals[i] = 0;
		p.who_is_eating[i] = 0;
		p.forks_being_used[i] = 0;

	}

	p.id = 0;
	p.cond_stop = 0;
	/* END - Secao de inicializacao das variaveis */

	for (i = 0; i < p.philosopher_number; i++) {
		if (pthread_create(&thread_philosopher[i], NULL, philosopher_existence, (void*)&p)) {
			printf("\n Erro na criacao da thread %d\n", i);
			exit(1);
		}
	}

	for (i = 0; i < p.philosopher_number; i++) {
		if (pthread_join(thread_philosopher[i], NULL)) {
			printf("\n Erro no join da thread %d\n", i);
			exit(1);
		}
	}

	//Construir struct de consumo para print no final
	Consume_Philosopher* t = (Consume_Philosopher *) malloc(p.philosopher_number * sizeof(Consume_Philosopher));
	for (int i = 0; i < p.philosopher_number; i++) {
		t[i] = create_consume_struct(i, p.consume_totals[i]);
	}

	//Ordenando os registros
	int j;
	Consume_Philosopher aux;
	for (i = 0; i < p.philosopher_number; ++i)
	{
		for (j = i + 1; j < p.philosopher_number; ++j)
		{
			if (t[i].total_consume > t[j].total_consume)
			{
				aux = t[i];
				t[i] = t[j];
				t[j] = aux;
			}
		}
	}

	print_consume_totals(t, p.philosopher_number);

	return 0;
}