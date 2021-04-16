 /* Project 2 - Multithreaded calculator */
// Name: 


#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE];
int num_ops;

/* Step 3: add mutual exclusion */
static pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;

/* Step 6: add condition flag varaibles */
struct progress_t{
	int add;
	int mult;
	int group;
} progress;

/* Step 7: use a semaphore */
static sem_t progress_lock;

/* Utiltity functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
    return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
    return isdigit(c);
}

/* End utility functions */


void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An unspecified error occured!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int timeToFinish()
{
    /* be careful: timeToFinish() also accesses buffer */
    return buffer[0] == '.';
}

/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression 
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;
	//
	char nString[50];
	int changed = 0;
	int result;

    // return NULL; /* remove this line to let the loop start*/

    while (1) {

		/* Step 3: add mutual exclusion */

	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

	pthread_mutex_lock(&buffer_lock);
	if (timeToFinish()) {
		pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	/* Step 2: implement adder */
	for (i = 0; i < bufferlen; i++) 
	{
	    // do we have value1 already?  If not, is this a "naked" number?
	    // if we do, is the next character after it a '+'?
	    // if so, is the next one after the sign a "naked" number?
		if(isNumeric(buffer[i])) 
		{ // 400 + 30
			startOffset = i;
			value1 = string2int(buffer + i); //400
			while (isNumeric(buffer[i])) {
				i++;
			}

			if(buffer[i] != '+' || !isNumeric(buffer[i+1])){
				continue;
			}
			value2 = string2int(buffer + i + 1); //30
			result = value1 + value2;

			do
			{
				i++;
			} while (isNumeric(buffer[i]));
				
			remainderOffset = i;

			int2string(result, nString);

			strcpy(buffer + startOffset, nString); // 430+30*4500

			strcpy(buffer+ startOffset + strlen(nString), buffer + remainderOffset); //430 * 4500

			//what if we have 430+4500
			bufferlen = strlen(buffer);
			//i = remainderOffset + (strlen(nString)) - 1;
			i = remainderOffset - 1;

			changed = 1;
			num_ops++;
			
		}

	    // once we have value1, value2 and start and end offsets of the
	    // expression in buffer, replace it with v1+v2
	}

	// something missing?
	/* Step 3: free the lock */
	pthread_mutex_unlock(&buffer_lock);

	/* Step 6: check progress */
	sem_wait(&progress_lock);
	progress.add = changed ? 2 : 1;
	sem_post(&progress_lock);

	/* Step 5: let others play */
	sched_yield();
    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;

	//
	char nString[50];
	int changed = 0;
	int result;
	

    // return NULL; /* remove this line */

    while (1) {
		/* Step 3: add mutual exclusion */

	startOffset = remainderOffset = -1;
	value1 = value2 = -1;


	pthread_mutex_lock(&buffer_lock);
	if (timeToFinish()) {
	pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	/* Step 2: implement multiplier */
	for (i = 0; i < bufferlen; i++) 
	{
	    // same as adder, but v1*v2
		if(isNumeric(buffer[i])) 
		{ 
			startOffset = i;
			value1 = string2int(buffer[i]); 
			while (isNumeric(buffer[i])) 
			{
				i++;
			}

			if(buffer[i] != '*' || !isNumeric(buffer[i+1]))
			{
				continue;
			}
			value2 = string2int(buffer[i+1]); 
			result = value1 * value2;

			do
			{
				i++;
			} while (isNumeric(buffer[i]));
				
			remainderOffset = i;

			int2string(result, nString);

			strcpy(buffer[startOffset], nString); 

			strcpy(buffer[startOffset+strlen(nString)], buffer[remainderOffset]); 

			//what if we have 430+4500
			bufferlen = strlen(buffer);
			i = remainderOffset - 1;

			changed = 1;
			num_ops++;
		}
	}

	// something missing?
	/* Step 3: free the lock */
	pthread_mutex_unlock(&buffer_lock);

	/* Step 6: check progress */
	sem_wait(&progress_lock);
	progress.mult = changed ? 2 : 1;
	sem_post(&progress_lock);

	/* Step 5: let others play */
	sched_yield();

   	 
	}
}


/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
    int bufferlen;
    int i;
	//
	int startOffset;
	int changed = 0;

    // return NULL; /* remove this line */

    while (1) {

		/* Step 3: add mutual exclusion */

	pthread_mutex_lock(&buffer_lock);
	if (timeToFinish()) {
	pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	/* Step 2: implement degrouper */
	for (i = 0; i < bufferlen; i++) 
	{
	    // check for '(' followed by a naked number followed by ')'
		if(buffer[i] == '(' && isNumeric(buffer[i + 1]))
		{
			startOffset = i;

			do
			{
				i++;
			} while(isNumeric(buffer[i]));

			if(buffer[i] != ')')
			{
				i--;
				continue;
			}
		}
	    // remove ')' by shifting the tail end of the expression
		strcpy(buffer[i], buffer[i + 1]);
	    // remove '(' by shifting the beginning of the expression
		strcpy(buffer[startOffset], buffer[startOffset + 1]);

		bufferlen -= 2;
		i = startOffset;
		changed = 1;
		num_ops++;
	}

	// something missing?
	/* Step 3: free the lock */
	pthread_mutex_unlock(&buffer_lock);

	/* Step 6: check progress */
	sem_wait(&progress_lock);
	progress.group = changed ? 2 : 1;
	sem_post(&progress_lock);

	/* Step 5: let others play */
	sched_yield();

    }
}


/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;

    return NULL; /* remove this line */

    while (1) {

		/* Step 3: add mutual exclusion */
		pthread_mutex_lock(&buffer_lock);
	if (timeToFinish()) {
		pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    if (buffer[i] == ';') {
		if (i == 0) {
		    printErrorAndExit("Sentinel found empty expression!");
		} else {
		    /* null terminate the string */
		    numberBuffer[i] = '\0';
		    /* print out the number we've found */
		    fprintf(stdout, "%s\n", numberBuffer);
		    /* shift the remainder of the string to the left */
		    strcpy(buffer, &buffer[i + 1]);
		    break;
		}
	    } else if (!isNumeric(buffer[i])) {
		break;
	    } else {
		numberBuffer[i] = buffer[i];
	    }
	}

	// something missing?
	/* Step 6: check for progress */
	if(buffer[0] != 0)
	{
		pthread_mutex_unlock(&buffer_lock);
		sem_wait(&progress_lock);

		if(progress.add && progress.mult && progress.group)
		{
			if(progress.add > 1 || progress.mult > 1 || progress.group > 1)
			{
				memset(&progress, 0, sizeof(struct progress_t));
			}
			else{
				printf(stdout, "No progress can be made \n");
				exit(EXIT_FAILURE);
			}
		}
		sem_post(&progress_lock);
	}

	/* Step 5: let others play, too */
	pthread_mutex_unlock(&buffer_lock);
	sched_yield();

    }
}

/* reader reads in lines of input from stdin and writes them to the
   buffer */
void *reader(void *arg)
{
    while (1) {
	char tBuffer[100];
	int currentlen;
	int newlen;
	int free;

	fgets(tBuffer, sizeof(tBuffer), stdin);

	/* Sychronization bugs in remainder of function need to be fixed */

	newlen = strlen(tBuffer);
	currentlen = strlen(buffer);

	/* if tBuffer comes back with a newline from fgets, remove it */
	if (tBuffer[newlen - 1] == '\n') {
	    /* shift null terminator left */
	    tBuffer[newlen - 1] = tBuffer[newlen];
	    newlen--;
	}

	/* -1 for null terminator, -1 for ; separator */
	free = sizeof(buffer) - currentlen - 2;

	do 
	{
		/* Step 3: add mutual exclusion */
		pthread_mutex_lock(&buffer_lock);
		currentlen = strlen(buffer);
		free = sizeof(buffer) - currentlen - 2;
		pthread_mutex_unlock(&buffer_lock);
		sched_yield();
		} while(free < newlen);
		pthread_mutex_lock(&buffer_lock);


	/* we can add another expression now */
	strcat(buffer, tBuffer);
	strcat(buffer, ";");

	/* Step 6: reset flag variables indicating progress */
	sem_wait(&progress_lock);
	memset(&progress, 0, sizeof(struct progress_t));
	sem_post(&progress_lock);
	pthread_mutex_unlock(&buffer_lock);

	/* Stop when user enters '.' */
	if (tBuffer[0] == '.') {
	    return NULL;
	}
    }
}


/* Where it all begins */
int smp3_main(int argc, char **argv)
{
    void *arg = 0;		/* dummy value */

	/* Step 7: initialize your semaphore */
	sem_init(&progress_lock, 0, 1);
    /* let's create our threads */
    if (pthread_create(&multiplierThread, NULL, multiplier, arg)
	|| pthread_create(&adderThread, NULL, adder, arg)
	|| pthread_create(&degrouperThread, NULL, degrouper, arg)
	|| pthread_create(&sentinelThread, NULL, sentinel, arg)
	|| pthread_create(&readerThread, NULL, reader, arg)) {
	printErrorAndExit("Failed trying to create threads");
    }

    /* you need to join one of these threads... but which one? */
	/* The pthread_join() function for threads is the equivalent of wait() 
	for processes. A call to pthread_join blocks the calling thread until 
	the thread with identifier equal to the first argument terminates.*/
	// If you join do not detach.
    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(degrouperThread);
    pthread_detach(sentinelThread);
    pthread_detach(readerThread);
	/* Step 1: we have to join on the ________ thread. */
	// pthread_join(____, NULL);
	pthread_join(sentinelThread, NULL);

    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);

	// TODO destroy semaphores and mutex
	pthread_mutex_destroy(&buffer_lock);
	sem_destroy(&progress_lock);

	// return
    return EXIT_SUCCESS;
}
