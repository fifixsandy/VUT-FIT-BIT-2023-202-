/**
 * @file proj2.c
 * 
 * @brief Program for project 2 from course IOS on BUT FIT focused on synchronisation using semaphores.
 * 
 * This program solves modified The Senate Bus Problem from Little Book Of Semaphores by Allan B. Downey.
 *  THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT

 * @author xnovakf00
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>

#define ALL_OK 0
#define ERR_ARG_COUNT 1
#define ERR_L 2
#define ERR_Z 3
#define ERR_K 4
#define ERR_TL 5
#define ERR_TB 6
#define ERR_FORK_FAIL 7
#define ERR_FILE_OPEN 8
#define ERR_CONVERT 9
#define ERR_MMAP 10
#define ERR_SEM_INIT 11
#define ERR_MUNMAP 12
#define ERR_FILE 13
#define ERR_MALLOC 14


#define MAX_L 20000
#define MIN_L 0
#define MAX_Z 10
#define MIN_Z 0
#define MAX_K 100
#define MIN_K 10
#define MAX_TL 10000
#define MIN_TL 0
#define MAX_TB 1000
#define MIN_TB 0
 

int L, Z, K, TK, TL, TB; // variables which represent argumets on intput

typedef struct shared
{
    bool rlyBoarded;
    int processNum, currZ, doneL, busFill; 
    int stops[MAX_Z + 1]; 
    FILE *output;
   
} Shared; // Structure of all shared variables which will need to be controlled by semaphores

sem_t *print, *boarded, *exited, *mutex, *busArrival, *finalStop; // semaphores for synchronisation

Shared *shared;

pid_t *idsL = NULL; // Pointer to store skier process IDs
int runningL = 0; // Number of currently active skier processes

/******************************************
 * * * * * * HELPER FUNCTIONS * * * * * *
******************************************/

/**
 * @brief Minimum of two integers
 * @param a First integer
 * @param b Second integer
 * @return Minimum of two integers
*/
int min(int a, int b)
{
    return (a < b) ? a : b;
}

/**
 * @brief Takes two integers and returns random number from the interval
 * @param min Minimal value of the interval
 * @param max Maximal value of the interval
 * @return Random integer
*/
int randFromRange(int min, int max)
{
    return ((rand() % (max - min + 1)) + min);
}

/******************************************
 * * * * MESSAGE PRINTING FUNCTIONS * * * *
******************************************/

/**
 * @brief Prints error message to stderr
 * @param errCode Identifies error based on which the message will be printed
*/
void errPrint(int errCode) 
{
    switch (errCode) 
    {
        case ERR_ARG_COUNT:
            fprintf(stderr, "Error: Incorrect number of arguments\n");
            break;
        case ERR_L:
            fprintf(stderr, "Error: Invalid value for L\n");
            break;
        case ERR_Z:
            fprintf(stderr, "Error: Invalid value for Z\n");
            break;
        case ERR_K:
            fprintf(stderr, "Error: Invalid value for K\n");
            break;
        case ERR_TL:
            fprintf(stderr, "Error: Invalid value for TL\n");
            break;
        case ERR_TB:
            fprintf(stderr, "Error: Invalid value for TB\n");
            break;
        case ERR_FORK_FAIL:
            fprintf(stderr, "Error: Fork failed\n");
            break;
        case ERR_FILE_OPEN:
            fprintf(stderr, "Error: Unable to open file\n");
            break;
        case ERR_CONVERT:
            fprintf(stderr, "Error: Unable to convert argument into integer\n");
            break;
        case ERR_MMAP:
            fprintf(stderr, "Error: Mmap failed\n");
            break;
        case ERR_SEM_INIT:
            fprintf(stderr, "Error: Initialisation of semaphore failed\n");
            break;
        case ERR_MUNMAP:
            fprintf(stderr, "Error: Munmap failed\n");
            break;
        case ERR_FILE:
            fprintf(stderr, "Error: File opening failed\n");
            break;
        case ERR_MALLOC:
            fprintf(stderr, "Error: Malloc failed\n");
            break;
        default:
            fprintf(stderr, "Unknown error\n");
    }
}

/**
 * @brief Prints process message with variable number of arguments to output file 
 * @param file File where the output should go
 * @param message Message to be printed
*/
void processPrint(FILE *file, const char *message, ...)
{
    va_list args;
    va_start (args, message);
    vfprintf (file, message, args);
    fflush(file); // to avoid output buffering
    va_end (args);
}


/******************************************
 * * * * INPUT PROCESSING FUNCTIONS * * * *
******************************************/

/**
 * @brief Checks whether given arguments are in the correct form, when not, exits the whole program
 * @param argc Number of arguments given in cmdline
 * @param argv Array of string arguments
*/
void argCheck(int argc, char* argv[])
{
    if(argc != 6)
    {
        errPrint(ERR_ARG_COUNT);
        exit(1);
    }

    int convertErr = sscanf(argv[1], "%d", &L);
    if(!convertErr)
    {
        errPrint(ERR_CONVERT);
        exit(1);
    }
    convertErr = sscanf(argv[2], "%d", &Z);
    if(!convertErr)
    {
        errPrint(ERR_CONVERT);
        exit(1);
    }
    convertErr = sscanf(argv[3], "%d", &K);
    if(!convertErr)
    {
        errPrint(ERR_CONVERT);
        exit(1);
    }
    convertErr = sscanf(argv[4], "%d", &TL);
    if(!convertErr)
    {
        errPrint(ERR_CONVERT);
        exit(1);
    }
    convertErr = sscanf(argv[5], "%d", &TB);
    if(!convertErr)
    {
        errPrint(ERR_CONVERT);
        exit(1);
    }

    if(L < MIN_L || L >= MAX_L)
    {
        errPrint(ERR_L);
        exit(1);
    }
    if(Z <= MIN_Z || Z > MAX_Z)
    {
        errPrint(ERR_Z);
        exit(1);
    }
    if(K < MIN_K || K > MAX_K)
    {
        errPrint(ERR_K);
        exit(1);
    }
    if(TL < MIN_TL || TL > MAX_TL)
    {
        errPrint(ERR_TL);
        exit(1);
    }
    if(TB < MIN_TB || TB > MAX_TB)
    {
        errPrint(ERR_TB);
        exit(1);
    }
    return;
}

/******************************************
 * * * * * * CLEANUP FUNCTIONS * * * * * *
******************************************/


/**
 * @brief Function kills all running skier processes
*/
void killSkiers()
{
    for(int i = 0; i < runningL; i++)
    {
        kill(idsL[i], SIGTERM);
    }
    free(idsL);
}


/**
 * @brief Closes opened file, unmaps shared variables and semaphores, if error occurs, ends the whole program with 1
 * @param shared Struct of shared variables to be unmapped
*/
void clean(Shared *shared)
{
    if(shared->output != NULL)
    {
        fclose(shared->output);
    }

    if(shared != NULL)
    {
        if(munmap(shared, sizeof(Shared)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(print != NULL)
    {
        if(munmap(print, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(boarded != NULL)
    {
        if(munmap(boarded, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(exited != NULL)
    {
        if(munmap(exited, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(mutex != NULL)
    {
        if(munmap(mutex, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(busArrival != NULL)
    {
        if(munmap(busArrival, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(finalStop != NULL)
    {
        if(munmap(finalStop, sizeof(sem_t)) == -1)
        {
            errPrint(ERR_MUNMAP);
            exit(1);
        }
    }
    if(idsL != NULL)
    {
        free(idsL);
    }

    return;
}

/******************************************
 * * * * INITIALISATION FUNCTIONS * * * *
******************************************/

/**
 * @brief Maps all shared variables needed for synchronisation, if error occurs, the whole program will end with 1
 * @param topMap Pointer to a pointer of shared structure to map
*/
 // THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT

void map(Shared **toMap)
{
    *toMap = mmap(NULL, sizeof(Shared), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0); // map structure of shared variables
    if(toMap == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    print = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(print == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    
    boarded = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(boarded == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    exited = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(exited == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(mutex == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    busArrival = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(busArrival == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    finalStop = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(finalStop == MAP_FAILED)
    {
        errPrint(ERR_MMAP);
        clean(*toMap);
        exit(1);
    }
    return;
}

/**
 * @brief Initialises shared variables and semaphores, if error occurs, exits whole program with 1
 * @param shared Pointer to the struct of shared variables
*/
void init(Shared *shared)
{
    shared->rlyBoarded = false;
    shared->processNum = 1;
    shared->currZ = 1;
    shared->doneL = 0;
    shared->busFill = 0;
    shared->output = NULL;
    for (int i = 0; i < (MAX_Z + 1); i++) 
    {
        shared->stops[i] = 0;
    }

    if(sem_init(print, 1, 1) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }
    if(sem_init(boarded, 1, 0) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }
    if(sem_init(exited, 1, 0) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }
    if(sem_init(mutex, 1, 1) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }
    if(sem_init(busArrival, 1, 0) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }
    if(sem_init(finalStop, 1, 0) == -1)
    {
        errPrint(ERR_SEM_INIT);
        clean(shared);
        exit(1);
    }

    idsL = (pid_t *)malloc(L * sizeof(pid_t));
    if(idsL == NULL)
    {
        errPrint(ERR_MALLOC);
        exit(1);
    }
    return;
}

/**
 * @brief Opens file for output, if fails, exits program with 1
 * @param file Pointer which should point to opened file
*/
void fileOpen(FILE **file)
{
    if((*file = fopen("proj2.out", "w")) == NULL)
    {
        errPrint(ERR_FILE);
        clean(shared);
        exit(1);
    }
}

/******************************************
 * * * * * FUNCTIONS FOR SKIERS * * * * *
******************************************/

/**
 * @brief Performs all actions needed for skier when boarding
 * @param idL ID number of skier
 * @param idZ ID number of stop from where the skier boards
*/
void board(int idL, int idZ)
{
    sem_wait(mutex);
    shared->busFill++; // when the skier enters the bus, occupied seats increment
    shared->stops[idZ]--; // skiers waiting on stop decrement
    shared->rlyBoarded = true; // yes, Mr. Driver, I really boarded
    sem_post(mutex);
    sem_wait(print);
    processPrint(shared->output, "%d: L %d: boarding\n", shared->processNum++, idL);
    sem_post(print);
}

/**
 * @brief Performs all actions needed for skier when exiting the bus
 * @param idL ID number of the skier
*/
void exitBus(int idL)
{
    sem_wait(mutex);
    shared->doneL++; // when skier exits bus, he finishes his journey and goes skiing wooohooo
    shared->busFill--; // occupancy decrements
    sem_post(mutex); 
    sem_wait(print);
    processPrint(shared->output, "%d: L %d: going to ski\n", shared->processNum++, idL);
    sem_post(exited);
    sem_post(print);
}

/**
 * @brief Performs all actions for the skier process
 * @param idL ID number of the skier
 * @param idL ID number of the stop where the skier waits for the bus
*/
void skier(int idL, int idZ)
{
    sem_wait(print);
    processPrint(shared->output, "%d: L %d: started\n", shared->processNum++, idL);
    sem_post(print);
    srand(time(NULL)^getpid()); // sets new seed for random
    usleep(randFromRange(0,TL)); // skier goes to stop (random time)
    sem_wait(mutex); 
    sem_wait(print);
    processPrint(shared->output, "%d: L %d: arrived to %d\n", shared->processNum++, idL, idZ);
    shared->stops[idZ]++; // people waiting on stop increments
    sem_post(print);
    sem_post(mutex);

    while(1)
    {
        sem_wait(busArrival);
        sem_wait(mutex);
        if(shared->currZ == idZ) // if the bus is at skiers stop, he can board
        {
            sem_post(mutex);
            board(idL, idZ);
            sem_post(boarded);
            sem_wait(finalStop); // waiting for bus to come to final stop
            exitBus(idL); // going skiing, exiting the bus
            break; // when exited, break the whole while cycle
        }
        else // the bus is not at skiers stop :(
        {
            sem_post(boarded); // so that bus can move on and signal another skier 
            shared->rlyBoarded = false; // no Mr. Driver, I did not really board, you have to check it
            sem_post(mutex);
            continue; // skier did not tell the truth, he has to go to the end of line again as a punishment 
        } 
    }
}

/******************************************
 * * * * * * FUNCTIONS FOR BUS * * * * * *
******************************************/

/**
 * @brief Performs all actions needed for when bus leaves the stop
*/
void busLeaving()
{
    sem_wait(mutex);
    sem_wait(print);
    processPrint(shared->output, "%d: BUS: leaving %d\n", shared->processNum++, shared->currZ);
    shared->currZ++; // moving on to another stop
    sem_post(mutex);
    sem_post(print);
}

/**
 * @brief Performs all actions for the process bus
*/
void bus()
{
    sem_wait(print);
    processPrint(shared->output, "%d: BUS: started\n", shared->processNum++);
    sem_post(print);
    while(1)
    {
        srand(time(NULL)^getpid()); // new seed for random
        usleep(randFromRange(0, TB)); // bus drives to stop for a random time
        sem_wait(mutex);
        sem_wait(print);
        processPrint(shared->output, "%d: BUS: arrived to %d\n", shared->processNum++, shared->currZ);
        sem_post(print);
        int n = min(shared->stops[shared->currZ], K); // how many skiers can the bus take (takes skiers only ALREADY present at the stop)
        sem_post(mutex);
        for(int i = n; i > 0; i--)
        {
            sem_post(busArrival); // signal one skier that he can board
            sem_wait(boarded); // wait for skier to board
            sem_wait(mutex);
            if(!(shared->rlyBoarded)) // check whether he really boarded
            {
                i++; // if not, bus has to signal one more skier
            }
            sem_post(mutex);
        }
        busLeaving();
        if(shared->currZ == Z+1) // when going to final stop
        {
            srand(time(NULL)^getpid()); // new seed
            usleep(randFromRange(0, TB)); // drive to final stop for random time
            sem_wait(print);
            processPrint(shared->output, "%d: BUS: arrived to final\n", shared->processNum++);
            sem_post(print);
            sem_wait(mutex);
            while(shared->busFill != 0) // while there is still someone in the bus
            {
                sem_post(mutex);
                sem_post(finalStop); // signal one skier he can exit the bus
                sem_wait(exited); // wait for skier to exit
                sem_wait(mutex);
            }
            sem_post(mutex);
            sem_wait(print);
            processPrint(shared->output, "%d: BUS: leaving final\n", shared->processNum++);
            sem_post(print);
            if(shared->doneL == L) // when there is no skier that is not skiing
            {
                processPrint(shared->output, "%d: BUS: finish\n", shared->processNum++);
                break; // finish, break the whole while
            }
            else // when there are still some skiers not skiing
            {
                sem_wait(mutex);
                shared->currZ = 1; // go back to stop num 1
                sem_post(mutex);
                continue;
            }
        }
    }
}

 // THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT

int main(int argc, char* argv[])
{
    argCheck(argc, argv);
    map(&shared);
    init(shared);
    fileOpen(&shared->output);

    pid_t pidB = fork(); // fork for the BUS process
    if(pidB == -1) // check for fork fail
    {
        errPrint(ERR_FORK_FAIL);
        clean(shared);
        killSkiers();
        exit(1); // we are not in the bus process, exits whole program
    }
    if(pidB == 0)
    {
        bus();
        clean(shared);
        exit(0);
    }

    for(int idL = 1; idL <= L; idL++)
    {
        pid_t pidL = fork();
        if(pidL == -1)
        {
            errPrint(ERR_FORK_FAIL);
            clean(shared);
            kill(pidB, SIGKILL);
            killSkiers();
            exit(1);
        }
        if(pidL == 0)
        {
            srand(time(NULL)^getpid());
            int idZ = randFromRange(1, Z); // send the skier to a random stop
            skier(idL, idZ);
            clean(shared);
            exit(0);
        }
        else
        {
            idsL[runningL++] = pidL;
        }
    }
    while(wait(NULL) > 0);
    clean(shared);
    return 0;
}
