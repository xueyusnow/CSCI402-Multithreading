#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "cs402.h"
#include "my402list.h"
#include <signal.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_t thread[5];
sigset_t set;
typedef int bool;
#define true 1
#define false 0
struct timeval initialtime;
struct timeval currenttime;
bool showt;
bool finish1;
bool finish2;
bool tokenfinish;
bool shutdown;
bool dropfinish;
double now;
double tokentime;
double arrive_time;
double inter_arrival_sum;
double service_time_sum;
double system_time_sum;
double system_square_time;
double Q1_time_sum;
double Q2_time_sum;
double S1_time_sum;
double S2_time_sum;
My402List list1,list2;	
int tokens;
int endpocket;
int droptoken;
int pockets;
int pocketspass;
int droppocket;
int B;
double lambda;
double mu;	
double r;
int P;
int num;
int currenttokens;
char *filename;
typedef struct tagPocket {
	int No;
	double arrival_time;
	double inQ1;
	double outQ1;
	double inQ2;
	double outQ2;
	int serNo;
	double inserver;
	double outserver;
	int reqNo;
	double inter_arrival_time;
	int reqsertime;
} Pocket;
void minus(struct timeval tx,struct timeval tx1, struct timeval difftime);
double computetime(struct timeval tx);
void *Q11(){
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
	for(; pockets<num;){	
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		if(now - arrive_time < 1000.0/lambda){
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
			usleep(arrive_time *1000.0 + 1000000.0/lambda - 1000.0*now);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
		}	
		Pocket *p = (Pocket*)malloc(sizeof(Pocket));
		pthread_mutex_lock(&m);
		if(shutdown == true){
			free(p);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}
		pockets++;
		p->No = pockets;
		p->reqsertime = (int)(1000.0/mu+0.5);
		p->reqNo =P;
		if(p->No == 1){
			p->inter_arrival_time = now;
		}else{
			p->inter_arrival_time = now - arrive_time;
		}
		p->arrival_time = now;
		arrive_time = now;
		inter_arrival_sum+=p->inter_arrival_time;
		
		if(p->reqNo > B){
			droppocket++;
			fprintf(stdout, "%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3lfms, dropped\n", now, p->No, p->reqNo, p->inter_arrival_time);
			free(p);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}
		fprintf(stdout, "%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3lfms\n", p->arrival_time, p->No, p->reqNo, p->inter_arrival_time);
		if(My402ListEmpty(&list1)){
			My402ListAppend(&list1, p);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			p->inQ1 = now;
			fprintf(stdout, "%012.3lfms: p%d enters Q1\n", p->inQ1, p->No);
			My402ListElem *elem = My402ListFirst(&list1);
			Pocket *pp = elem->obj;
			if(currenttokens >= pp->reqNo){	
				currenttokens = currenttokens - pp->reqNo;
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
				pp->outQ1 = now;
				double Q1duringtime = pp->outQ1-pp->inQ1;
				Q1_time_sum +=Q1duringtime;
				My402ListUnlink(&list1, elem);
				fprintf(stdout, "%012.3lfms: p%d leaves Q1, time in Q1 = %.3lfms, token bucket now has %d token\n", pp->outQ1, pp->No, Q1duringtime, currenttokens);
				My402ListAppend(&list2, pp);
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
				pp->inQ2 = now;
				fprintf(stdout, "%012.3lfms: p%d enters Q2\n", pp->inQ2, pp->No);
			}
		}else{
			My402ListAppend(&list1, p);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			p->inQ1 = now;
			fprintf(stdout, "%012.3lfms: p%d enters Q1\n", p->inQ1, p->No);

		}
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
	}
	pthread_exit(NULL);
	return(0);
}

void *Q12(void *filename){
	FILE *fp = fopen(filename, "r");
	char input[1026];
	memset(input, 0, sizeof(input));
	fgets(input, sizeof(input), fp);
	num = atoi(input);
	int req[num][3];
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
	for(; pockets<num;){	
		memset(input, 0, sizeof(input));
		fgets(input, sizeof(input), fp);
		char temp[1026];
		int k = 0;
		memset(temp, 0, sizeof(temp));
		while(input[k]>='0'&&input[k]<='9'){
				temp[k] = input[k];
				k++;
		}
		req[pockets][0] = atoi(temp);
		memset(temp, 0, sizeof(temp));
		while(input[k] ==' '||input[k]=='\t'){
				k++;
		}
		int j = k;
		while(input[k]>='0'&&input[k]<='9'){
				temp[k-j] = input[k];
				k++;
		}
		req[pockets][1] = atoi(temp);
		while(input[k] ==' '||input[k]=='\t'){
				k++;
		}
		memset(temp, 0, sizeof(temp));
		j = k;
		while(input[k]>='0'&&input[k]<='9'){
				temp[k-j] = input[k];
				k++;
		}
		req[pockets][2] = atoi(temp);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		//usleep(100000);
		if(now - arrive_time < req[pockets][0]){
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
			usleep(req[pockets][0]*1000.0 + arrive_time *1000.0 - 1000.0*now);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
		}
		Pocket *p = (Pocket*)malloc(sizeof(Pocket));
		pthread_mutex_lock(&m);
		if(shutdown == true){
			free(p);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}
		pockets++;
		p->No = pockets;
		p->reqsertime = req[pockets - 1][2];
		p->reqNo = req[pockets - 1][1];	
		if(p->No == 1){
			p->inter_arrival_time = now;
		}else{
			p->inter_arrival_time = now - arrive_time;
		}
		p->arrival_time = now;
		arrive_time = now;
		inter_arrival_sum+=p->inter_arrival_time;
		if(req[pockets - 1][1] > B){
			droppocket++;
			fprintf(stdout, "%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3lfms, dropped\n", now, p->No, p->reqNo, p->inter_arrival_time);
			free(p);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}

		fprintf(stdout, "%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3lfms\n", p->arrival_time, p->No, p->reqNo, p->inter_arrival_time);
		if(My402ListEmpty(&list1)){
			My402ListAppend(&list1, p);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			p->inQ1 = now;
			fprintf(stdout, "%012.3lfms: p%d enters Q1\n", p->inQ1, p->No);
			My402ListElem *elem = My402ListFirst(&list1);
			Pocket *pp = elem->obj;
			if(currenttokens >= pp->reqNo){
				currenttokens = currenttokens - pp->reqNo;
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
				pp->outQ1 = now;
				double Q1duringtime = pp->outQ1-pp->inQ1;
				Q1_time_sum +=Q1duringtime;
				My402ListUnlink(&list1, elem);
				fprintf(stdout, "%012.3lfms: p%d leaves Q1, time in Q1 = %.3lfms, token bucket now has %d token\n", pp->outQ1, pp->No, Q1duringtime, currenttokens);
				My402ListAppend(&list2, pp);
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
				pp->inQ2 = now;
				fprintf(stdout, "%012.3lfms: p%d enters Q2\n", pp->inQ2, pp->No);
			}
		}else{
			My402ListAppend(&list1, p);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			p->inQ1 = now;
			fprintf(stdout, "%012.3lfms: p%d enters Q1\n", p->inQ1, p->No);
		}
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
	}
	fclose(fp);
	return(0);
}



void *Tokens(){
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
	for(;;){	
		if(My402ListEmpty(&list1)&&pockets==num){
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			break;
		}
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		if(now - tokentime < 1000.0/r){
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
			usleep(1000*(tokentime + 1000.0/r - now));
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
		}
		tokens++;
		pthread_mutex_lock(&m);
		
		if(shutdown == true){
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}
		if(currenttokens >= B){
			droptoken++;
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			tokentime=now;
			fprintf(stdout, "%012.3lfms: token t%d arrives, dropped\n", now, tokens);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			continue;
		}else{
			currenttokens++;
		}
		tokentime = now;
		fprintf(stdout, "%012.3lfms: token t%d arrives, token bucket now has %d token\n", tokentime, tokens, currenttokens);
		
			if(!My402ListEmpty(&list1)) {
				My402ListElem *elem = My402ListFirst(&list1);
				Pocket *pp = elem->obj;
				if(currenttokens >= pp->reqNo){
					currenttokens = currenttokens - pp->reqNo;
					gettimeofday(&currenttime,0);
					now = computetime(currenttime);
					pp->outQ1 = now;
					double Q1duringtime = pp->outQ1-pp->inQ1;
					Q1_time_sum+=Q1duringtime;
					My402ListUnlink(&list1, elem);
					fprintf(stdout, "%012.3lfms: p%d leaves Q1, time in Q1 = %.3lfms, token bucket now has %d token\n", pp->outQ1, pp->No, Q1duringtime, currenttokens);
					My402ListAppend(&list2, pp);
					gettimeofday(&currenttime,0);
					now = computetime(currenttime);
					pp->inQ2 = now;
					fprintf(stdout, "%012.3lfms: p%d enters Q2\n", pp->inQ2, pp->No);
				}
			}
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
	}
	pthread_exit(NULL);
	return(0);
}

void *s1(){
	for (;;) {
		pthread_mutex_lock(&m);
		while (My402ListEmpty(&list2)&&shutdown == false&&!(My402ListEmpty(&list1)&&My402ListEmpty(&list2)&&(pocketspass+droppocket == num))) {
			
		    	pthread_cond_wait(&cv, &m);
		}
		if(My402ListEmpty(&list1)&&My402ListEmpty(&list2)&&(pocketspass+droppocket == num)){
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&m);
				return(0);
			}
		if(shutdown == true){
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			break;
		}
		My402ListElem *elem = My402ListFirst(&list2);
		Pocket *pp = elem->obj;
		My402ListUnlink(&list2, elem);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		pp->outQ2 = now;
		Q2_time_sum+= pp->outQ2-pp->inQ2;
		fprintf(stdout, "%012.3lfms: p%d leaves Q2, time in Q2 = %.3lfms\n", pp->outQ2, pp->No, pp->outQ2-pp->inQ2);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		pp->serNo = 1;
		pp->inserver = now;
		fprintf(stdout, "%012.3lfms: p%d begins service at S1, requesting %dms of service\n", pp->inserver, pp->No, pp->reqsertime);
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		if(now - pp->inserver < 1000.0/pp->reqsertime){
				usleep(1000*(pp->inserver + pp->reqsertime - now));
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
		}
		pp->outserver = now;
		service_time_sum+=pp->outserver-pp->inserver;
		S1_time_sum+=pp->outserver-pp->inserver;
		system_time_sum+=(pp->outserver-pp->arrival_time)/1000.0;
		system_square_time+=(pp->outserver-pp->arrival_time)*(pp->outserver-pp->arrival_time)/1000000.0;
		fprintf(stdout, "%012.3lfms: p%d departs from S1, service time = %.3lfms, time in system = %.3lfms\n", pp->outserver, pp->No,pp->outserver-pp->inserver,pp->outserver-pp->arrival_time);
		free(pp);
		pocketspass++;
	}
	return(0);
}

void *s2(){
	for (;;) {
		pthread_mutex_lock(&m);
		while (My402ListEmpty(&list2)&&shutdown == false&&!(My402ListEmpty(&list1)&&My402ListEmpty(&list2)&&(pocketspass+droppocket == num))) {
			
		    	pthread_cond_wait(&cv, &m);
		}
		if(My402ListEmpty(&list1)&&My402ListEmpty(&list2)&&(pocketspass+droppocket == num)){
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&m);
				return(0);
			}
		if(shutdown == true){
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			break;
		}
		My402ListElem *elem = My402ListFirst(&list2);
		Pocket *pp = elem->obj;
		My402ListUnlink(&list2, elem);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		pp->outQ2 = now;
		Q2_time_sum+= pp->outQ2-pp->inQ2;
		fprintf(stdout, "%012.3lfms: p%d leaves Q2, time in Q2 = %.3lfms\n", pp->outQ2, pp->No, pp->outQ2-pp->inQ2);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		pp->serNo = 2;
		pp->inserver = now;
		fprintf(stdout, "%012.3lfms: p%d begins service at S2, requesting %dms of service\n", pp->inserver, pp->No, pp->reqsertime);
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		if(now - pp->inserver < pp->reqsertime){
				usleep(1000*(pp->inserver + pp->reqsertime - now));
				gettimeofday(&currenttime,0);
				now = computetime(currenttime);
		}
		pp->outserver = now;
		service_time_sum+=pp->outserver-pp->inserver;
		S2_time_sum+=pp->outserver-pp->inserver;
		system_time_sum+=(pp->outserver-pp->arrival_time)/1000.0;
		system_square_time+=(pp->outserver-pp->arrival_time)*(pp->outserver-pp->arrival_time)/1000000.0;
		fprintf(stdout, "%012.3lfms: p%d departs from S2, service time = %.3lfms, time in system = %.3lfms\n", pp->outserver, pp->No,pp->outserver-pp->inserver,pp->outserver-pp->arrival_time);
		free(pp);
		pocketspass++;
    	}
	return(0);
}
double computetime(struct timeval tx){
	struct timeval difftime;
	difftime.tv_sec = 0;
	difftime.tv_usec = tx.tv_sec*1000000 + tx.tv_usec - initialtime.tv_usec - initialtime.tv_sec*1000000;
	while(difftime.tv_usec - 1000000 >= 0){
		difftime.tv_usec -= 1000000;
		difftime.tv_sec++;
	}
	return ((int)(difftime.tv_sec)*1000000.0 + (int)(difftime.tv_usec))/1000.0;
}

static void statistics(double endtime){
	fprintf(stdout,"Statistics:\n\n");
	fprintf(stdout,"    average packet inter-arrival time = %.6g\n", inter_arrival_sum/(double)pockets/1000.0);
	if(pocketspass == 0){
		fprintf(stdout,"    average packet service time = (N/A:no packet arrived at servers)\n\n");
	}else{
		fprintf(stdout,"    average packet service time = %.6g\n\n", service_time_sum/(double)pocketspass/1000.0);
	}
	fprintf(stdout,"    average number of packets in Q1 = %.6g\n", Q1_time_sum/endtime);
	fprintf(stdout,"    average number of packets in Q2 = %.6g\n", Q2_time_sum/endtime);
	fprintf(stdout,"    average number of packets in S1 = %.6g\n", S1_time_sum/endtime);
	fprintf(stdout,"    average number of packets in S2 = %.6g\n\n", S2_time_sum/endtime);
	if(pocketspass == 0){
		fprintf(stdout,"    average time a packet spent in system = (N/A:no packet passed the system)\n");
		fprintf(stdout,"    standard deviation for time spent in system = (N/A:no packet passed the system)\n\n");
	}else{
		double EX = system_time_sum/(double)pocketspass;
		fprintf(stdout,"    average time a packet spent in system = %.6g\n", EX);
		double EX2 = system_square_time/(double)pocketspass;
		double var = EX2-pow(EX,2.0);
		fprintf(stdout,"    standard deviation for time spent in system = %.6g\n\n", sqrt(var));
	}
	fprintf(stdout,"    token drop probability = %.6g\n", droptoken/(double)tokens);
	fprintf(stdout,"    packet drop probability = %.6g\n", droppocket/(double)pockets);
}


void *monitor(){
	int sig;
	//while(1){
		sigwait(&set, &sig);
		pthread_mutex_lock(&m);
		shutdown = true;
		pthread_cancel(thread[0]);
		pthread_cancel(thread[1]);
		gettimeofday(&currenttime,0);
		now = computetime(currenttime);
		fprintf(stdout, "\n%012.3lfms: SIGINT caught, no new packets or tokens will be allowed\n", now);
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&m);
		//break;
	//}
	return(0);

}
int main(int argc, char *argv[])
{
	memset(&list1, 0, sizeof(My402List));
	(void)My402ListInit(&list1);
	memset(&list2, 0, sizeof(My402List));
	(void)My402ListInit(&list2);
	showt = false;
	finish1 = false;
	finish2 = false;
	tokenfinish = false;
	dropfinish = false;
	shutdown = false;
	now = 0;
	arrive_time = 0;
	inter_arrival_sum = 0;
	service_time_sum = 0;
	system_time_sum = 0;
	system_square_time = 0;
	Q1_time_sum = 0;
	Q2_time_sum = 0;
	S1_time_sum = 0;
	S2_time_sum = 0;
	tokens = 0;
	endpocket = num;
	droptoken = 0;
	pockets = 0;
	pocketspass = 0;
	droppocket = 0;
	B = 10;
	lambda = 2;
	mu = 0.35;	
	r = 1.5;
	P = 3;
	num = 20;
	currenttokens = 0;
	filename = NULL;
	if(argc%2 == 0){
		fprintf(stderr, "Wrong parameters number\n");
		return 0;
	}
	for(int i = 1; i < argc; i=i+2){
		if(strcmp(argv[i], "-lambda")==0){
			lambda = atof(argv[i+1]);
			if(1/lambda > 10){
				lambda = 0.1;
			}
		}
		else if(strcmp(argv[i], "-mu")==0){
			mu = atof(argv[i+1]);
			if(1/mu > 10){
				mu = 0.1;
			}
		}
		else if(strcmp(argv[i], "-r")==0){
			r = atof(argv[i+1]);
			if(1/r > 10){
				r = 0.1;
			}
		}
		else if(strcmp(argv[i], "-B")==0){
			B = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-P")==0){
			P = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-n")==0){
			num = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-t")==0){
			showt = true;
			//filename = getFileName(argv[i+1]);
			filename = argv[i+1];
			FILE *fp = fopen(filename, "r");
			if (fp == NULL) {
		        			perror(argv[2]);
		        			return 0;
			}
			char input[1026];
			memset(input, 0, sizeof(input));
			fgets(input, sizeof(input), fp);
			if(input[0] == 0){
				fprintf(stderr, "Wrong input - the number in line 1 is 0\n");
				return 0;
			}
			int k = 0;
			while (input[k] != '\n'){
				if(input[k] < '0'||input[k] > '9'){
					fprintf(stderr, "Wrong input - line 1 is not just a number\n");
					return 0;
				}else{
					k++;
				}
			}
			num = atoi(input);
			int i=0;
			while(fgets(input,sizeof(input), fp)!=NULL){
				i++;
				int k = 0;
				while (input[k] != '\n'){
					if((input[k]!=' ' && input[k]!='\t')&&(input[k] < '0'||input[k] > '9')){
						fprintf(stderr, "Wrong input - line %d has invalid characters\n",i+1);
						return 0;
					}else{
						k++;
					}
				}
			}
			if(i!=num){
				fprintf(stderr, "Wrong input - the number of lines doesn't match the num\n");
				exit(0);
			}
			fclose(fp);
		}else{
			fprintf(stderr, "Wrong command : %s\n", argv[i]);
			return 0;
		}
	}
	memset(&list1, 0, sizeof(My402List));
	memset(&list2, 0, sizeof(My402List));
	gettimeofday(&initialtime,NULL);
	tokentime = computetime(initialtime);
	fprintf(stdout,"Emulation Parameters:\n");
	fprintf(stdout,"    number to arrive = %d\n",num);
	if(showt == false){
		fprintf(stdout,"    lambda = %g\n",lambda);
		fprintf(stdout,"    mu = %g\n",mu);
	}
	fprintf(stdout,"    r = %g\n",r);
	fprintf(stdout,"    B = %d\n",B);
	if(showt == false){
		fprintf(stdout,"    P = %d\n",P);
	}
	if(showt == true){
		fprintf(stdout,"    tsfile = %s\n",filename);
	}
	fprintf(stdout,"\n");
	double starttime = 00000000.000;
	fprintf(stdout,"%012.3lfms: emulation begins\n",starttime);
	pthread_mutex_init(&m,NULL);
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigprocmask(SIG_BLOCK,&set,0);

	pthread_create(&thread[0], 0, Tokens, NULL);
	if(showt == true)
		pthread_create(&thread[1], 0, Q12, filename);
	else
		pthread_create(&thread[1], 0, Q11, NULL);
	pthread_create(&thread[2], 0, s1, NULL);
	pthread_create(&thread[3], 0, s2, NULL);
	pthread_create(&thread[4], 0, monitor, 0);
	//long_running_proc();
	pthread_join(thread[0],NULL);
	pthread_join(thread[1],NULL);
	pthread_join(thread[2],NULL);
	pthread_join(thread[3],NULL);
	//pthread_join(thread[4],NULL);
	if(shutdown == false){
		(void)My402ListUnlinkAll(&list1);
		(void)My402ListUnlinkAll(&list2);
	}else{
		while(!My402ListEmpty(&list1)){
			My402ListElem *elem = My402ListFirst(&list1);
			Pocket *ppp = elem->obj;
			(void)My402ListUnlink(&list1,elem);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			fprintf(stdout, "%012.3lfms: p%d removed from Q1\n", now, ppp->No);
		}
		while(!My402ListEmpty(&list2)){
			My402ListElem *elem = My402ListFirst(&list2);
			Pocket *ppp = elem->obj;
			(void)My402ListUnlink(&list2,elem);
			gettimeofday(&currenttime,0);
			now = computetime(currenttime);
			fprintf(stdout, "%012.3lfms: p%d removed from Q2\n", now, ppp->No);
		}
	}
	gettimeofday(&currenttime,NULL);

	double endtime = computetime(currenttime);
	fprintf(stdout,"%012.3lfms: emulation ends\n\n",endtime);

	statistics(endtime);	
	
	return 0;

}
