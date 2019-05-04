#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <semaphore.h>

typedef enum { false, true } bool;

const int DEBUG = 0;


// define locks
sem_t numChildrenLock;
sem_t numTeachersLock;
sem_t numParentsLock;

// define variable for teacher/student threshold
int R = 0;

// define variables for current number of children/teachers and number of teachers waiting to exit
int numChildren = 0;
int numTeachers = 0;
int numParents = 0;
int numTeachersWaitingToExit = 0;

// define variables for the appropriate sleep time for each teacher
int teachSeconds = 1;
int learnSeconds = 1;
int verifyComplianceSeconds = 1;

// add one to the teacher cound
void teacher_enter(){
	sem_wait(&numTeachersLock);
	numTeachers++;
	printf("Teacher has entered\n");
	sem_post(&numTeachersLock);
}

void teach(){
	sleep(teachSeconds);
	return;
}

// increment numTeachersWaitingToExit depending on the threshold or decrement numTeachers
// if a teacher can leave
void teacher_exit(){
	sem_wait(&numTeachersLock);
	sem_wait(&numChildrenLock);

	if(numTeachers > 1 && numChildren / (numTeachers - 1) < R){
		numTeachers--;
		printf("Teacher has exited\n");
	} else {
		numTeachersWaitingToExit++;
		printf("Teacher cannot leave yet\n");
	}

	sem_post(&numChildrenLock);
	sem_post(&numTeachersLock);
}

void* teacher(void *arg){
	// cast arg as the int value N that we need for this thread
	int N = *((int*)arg);
	int i;
	for(i = 0;i < N; i++){
		teacher_enter();
		teach();
		teacher_exit();
	}

	return arg;
}


// add one to the child cound
void child_enter(){
	sem_wait(&numChildrenLock);
	numChildren++;
	printf("Child %d has entered\n", numChildren);
	sem_post(&numChildrenLock);
}

void learn(){
	sleep(learnSeconds);
	return;
}

// child leaves and lets a teacher leave if its now possible
void child_exit(){
	sem_wait(&numTeachersLock);
	sem_wait(&numChildrenLock);
	printf("Child has exitted\n");
	numChildren--;

	if(numChildren == 0 && numTeachersWaitingToExit > 0){
		numTeachers = numTeachers - numTeachersWaitingToExit;
		numTeachersWaitingToExit = 0;
		printf("All Waiting teachers have exitted\n");
	}

	if(numTeachers > 1 && (numChildren / (numTeachers - 1) < R) && (numTeachersWaitingToExit > 0)){
		numTeachers--;
		numTeachersWaitingToExit--;
		printf("Teacher waiting to exit has left\n");
	}

	sem_post(&numChildrenLock);
	sem_post(&numTeachersLock);
}

void* child(void *arg){
	int N = *((int*)arg);
	int i;
	for(i = 0;i < N; i++){
		child_enter();
		learn();
		child_exit();
	}

	return arg;
}


// add one to the parents count
void parent_enter(){
	sem_wait(&numParentsLock);
	numParents++;
	printf("Parent %d has entered\n", numParents);
	sem_post(&numParentsLock);
}

void verify_compliance(){
	sleep(verifyComplianceSeconds);
	sem_wait(&numTeachersLock);
	sem_wait(&numChildrenLock);
	sem_wait(&numParentsLock);
	
	if(numTeachers > 1 && (numChildren / numTeachers) < R){
		printf("Parent %d finds that the teacher/child ratio is compliant\n", numParents);
	} else {
		printf("Parent %d finds that the teacher/child ratio is NOT compliant\n", numParents);
	}
	

	sem_post(&numChildrenLock);
	sem_post(&numTeachersLock);
	sem_post(&numParentsLock);
	return;
}

// parent leaves
void parent_exit(){
	sem_wait(&numParentsLock);
	numParents--;
	printf("Parent has exitted\n");
	sem_post(&numParentsLock);
}


void* parent(void *arg){
	int N = *((int*)arg);
	int i;
	for(i = 0;i < N; i++){
		parent_enter();
		verify_compliance();
		parent_exit();
	}

	return arg;
}

int main(int argc, char *argv[]){
    //check if there are correct # of argument inputs
	if(argc != 5){
		printf("FATAL ERROR: INCORRECT NUMBER OF ARGUMENTS\nUSE ARGS N N1 N2 R (N = num people, N1 = num teachers, N2 = num children\n");
		return 1;
	}

	// initialize N, N1, N2, R (R is a global variable)
	int N, N1, N2;
	N = atoi(argv[1]);
	N1 = atoi(argv[2]);
	N2 = atoi(argv[3]);
	R = atoi(argv[4]);

	// if N is less than the sum of N1 and N2, have an error
	if(N < (N1 + N2)){
		printf("FATAL ERROR: N > N1 + N2\n");
		return 1;
	}

	int N3 = N - N1 - N2;

	// initialize tids, mutexes and execute threads
	pthread_t teachersTid; 
	pthread_t childrenTid;
	pthread_t parentsTid;

	sem_init(&numChildrenLock, 0, 1);
	sem_init(&numTeachersLock, 0, 1);
	sem_init(&numParentsLock, 0, 1);

	pthread_create(&teachersTid, NULL, teacher, (void *)&N1);
	pthread_create(&childrenTid, NULL, child, (void *)&N2);
	pthread_create(&parentsTid, NULL, parent, (void *)&N3);

	pthread_join(teachersTid, NULL);
	pthread_join(childrenTid, NULL);
	pthread_join(parentsTid, NULL);

	sem_destroy(&numChildrenLock);
	sem_destroy(&numTeachersLock);
	sem_destroy(&numParentsLock);
    return 0;
}


