#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define EVAPORATION_RATE 0.05
#define ITERS_ACO 30
#define MIN_PHEROMONE 0.05
#define N_RUNS 5
#define N_INPUTS 30

typedef struct job {
    // Job informations
    int index;
    int processing_time;
    int due_date;
    int weight;
    // ================
    // Heuristic informations
    float pheromone;
    int scheduled;
    float probability;
    // ================
} Job;

// Global problem data
static int n_jobs;
static Job *jobs = NULL;
static int *best_solution = NULL;
static unsigned int best_aco_tardiness;
static unsigned int best_tardiness;
static int n_ants;
// ================

// Read the values of the input file
void readInput(const char *input_file)
{
    FILE *file = fopen(input_file, "r");

    if(file == NULL) {
        printf("Failure while reading file! Exiting...");
        exit(1);
    }

    fscanf(file, "%d", &n_jobs);
    jobs = (Job*)malloc(n_jobs * sizeof(Job));
    best_solution = (int*)malloc(n_jobs * sizeof(int));

    for(int i = 0; i < n_jobs; i++) {
        fscanf(file, "%d %d %d", &(jobs[i].processing_time), &(jobs[i].due_date), &(jobs[i].weight));
        jobs[i].index = i;
        jobs[i].scheduled = 0;
        jobs[i].pheromone = 1;
    }

    n_ants = 2 * n_jobs;
} /* readInput */

// Print the list of jobs
void printJobs(void)
{
    printf("Printing job list...\n");
    printf("PT\tDD\tTW\n");
    for(int i = 0; i < n_jobs; i++)
        printf("%d\t%d\t%d\n", jobs[i].processing_time, jobs[i].due_date, jobs[i].weight);
} /* printJobs */

// Calculate the tardiness of the job if inserted in the current position of the solution
unsigned int calcTardiness(Job job, int start_date)
{
    if((start_date + job.processing_time) > job.due_date)
        return ((start_date + job.processing_time) - job.due_date) * job.weight;
    return 0;
} /* calcTardiness */

// Calculate the probability of being chosen next by the ant for each node
void calcProbabilities()
{
    float sum = 0;
    for(int i = 0; i < n_jobs; i++)
    {
        if(jobs[i].scheduled == 0)
        {
            jobs[i].probability = jobs[i].pheromone;
            sum += jobs[i].probability;
        } else {
            jobs[i].probability = 0;
        }
    }

    for(int i = 0; i < n_jobs; i++)
        jobs[i].probability /= sum;
} /* calcProbabilities */

// Sorts a random job base in roulette wheel
int sortJob()
{
    float random = (float)rand()/RAND_MAX;
    float sum = 0;

    for(int i = 0; i < n_jobs; i++)
    {
        sum += jobs[i].probability;
        if(random < sum)
            return i;
    }

    return n_jobs - 1;
} /* sortJob */

// Print the current probabilities of each job
void printProbs()
{
    printf("\n========================\n");
    printf("Probabilities: \n");
    for(int i = 0; i < n_jobs; i++)
        printf("%.4f ", jobs[i].probability);
    printf("\n========================\n");
} /* printProbs() */

// Print the current pheromones of each job in the best solution
void printPheromones()
{
    printf("\n========================\n");
    printf("Pheromones: \n");
    for(int i = 0; i < n_jobs; i++)
        printf("%.4f ", jobs[best_solution[i]].pheromone);
    printf("\n========================\n");
} /* printPheromones */

// Calculate the total tardiness of a solution
unsigned int calcTotalTardiness(int* solution, int print)
{
    unsigned int total_tardiness = 0;
    int current_date = 0;
    
    for(int i = 0; i < n_jobs; i++)
    {
        total_tardiness += calcTardiness(jobs[solution[i]], current_date);
        if(print == 1)
            printf("\nJob: %d\nStart: %d\nProcessing Time: %d\nEnd: %d\nWeight: %d\nAdditional tardiness: %u\n", jobs[solution[i]].index, current_date, jobs[solution[i]].processing_time, current_date + jobs[solution[i]].processing_time, jobs[solution[i]].weight, calcTardiness(jobs[solution[i]], current_date));
        current_date += jobs[solution[i]].processing_time;
    }

    return total_tardiness;
} /* calcTotalTardiness */

// Increase the pheromones of a job based on its position in the best solution
void increasePheromones(int *iter_best_solution)
{
    for(int i = 0; i < n_jobs; i++)
        jobs[iter_best_solution[i]].pheromone += n_jobs / (2 * i + 2);
} /* increasePheromones */

// Evaporate the pheromones of each job based on the evaporation rate
void evaporate()
{
    for(int i = 0; i < n_jobs; i++)
    {
        jobs[i].pheromone *= (1 - EVAPORATION_RATE);
        if(jobs[i].pheromone < MIN_PHEROMONE)
            jobs[i].pheromone = MIN_PHEROMONE;
    }
} /* evaporate */

// Sets all the scheduled flags back to 0
void resetScheduleds()
{
    for(int i = 0; i < n_jobs; i++)
        jobs[i].scheduled = 0;
} /* resetScheduleds */

// Runs the ACO algorithm
unsigned int ACO(int *iter_best_solution)
{
    unsigned int iter_best_tardiness = UINT_MAX; // Best tardiness of the iteration

    int *current_solution = (int*)malloc(n_jobs * sizeof(int)); // Current ant solution
    unsigned int current_tardiness;

    unsigned int current_date; // Current time at the processing timeline
    int selected; // Selected job for insertion at next position 

    // ACO Single Iteration
    for(int i = 0; i < n_ants; i++)
    {
        resetScheduleds(); // Reset the scheduled flag for all jobs
        current_date = 0;

        // Constructing the ant solution
        for(int j = 0; j < n_jobs; j++)
        {
            calcProbabilities(); // Calculate the selection probability for each job
            selected = sortJob();
            current_solution[j] = selected;
            current_date += jobs[selected].processing_time;
            jobs[selected].scheduled = 1;      
        }

        // Checking if curreting solution is the best so far for the current iteration
        current_tardiness = calcTotalTardiness(current_solution, 0);
        if(current_tardiness < iter_best_tardiness)
        {
            iter_best_tardiness = current_tardiness;
            memcpy(iter_best_solution, current_solution, n_jobs * sizeof(int));
        }        
    } /* ACO Single Iteration */

    free(current_solution);    

    return iter_best_tardiness;
} /* ACO */

// Evaluates the quality of the swap of the jobs in the position i and j
unsigned int evaluateSwap(int *solution, int i, int j)
{
    unsigned int current_tardiness = calcTotalTardiness(solution, 0);
    unsigned int new_tardiness;
    int *new_solution = (int*)malloc(n_jobs * sizeof(int));
    int temp;

    memcpy(new_solution, solution, n_jobs * sizeof(int));
    temp = new_solution[i];
    new_solution[i] = new_solution[j];
    new_solution[j] = temp;
    new_tardiness = calcTotalTardiness(new_solution, 0);

    free(new_solution);
    return current_tardiness > new_tardiness ? current_tardiness - new_tardiness : 0;
} /* evaluateSwap */

// Inserts a job from the old position to the new position
void insertJob(int *solution, int old_position, int new_position)
{
    int temp = solution[old_position];
    int i;

    if(old_position < new_position)
    {
        for(i = old_position; i < new_position; i++)
            solution[i] = solution[i+1];
    } else {
        for(i = old_position; i > new_position; i--)
            solution[i] = solution[i-1];
    }
    solution[new_position] = temp;
} /* insertJob */

// Evaluates the insertion of a node from the position i to the position j
unsigned int evaluateInsertion(int *solution, int i, int *j)
{
    unsigned int current_tardiness = calcTotalTardiness(solution, 0);
    unsigned int new_tardiness = UINT_MAX;
    unsigned int best_insertion = UINT_MAX;
    int *new_solution = (int*)malloc(n_jobs * sizeof(int));

    memcpy(new_solution, solution, n_jobs * sizeof(int));
    for(int k = 0; i < n_jobs; i++)
    {
        if(i != k)
        {
            insertJob(new_solution, i, k);
            new_tardiness = calcTotalTardiness(new_solution, 0);
        }
        if(new_tardiness < best_insertion)
        {
            best_insertion = new_tardiness;
            (*j) = k;
        }
    }

    free(new_solution);
    return current_tardiness > best_insertion ? current_tardiness - best_insertion : 0;
} /* evaluateInsertion */

// Swap the jobs at the position index1 and index2
void swapJobs(int *solution, int index1, int index2)
{
    int temp = solution[index1];
    solution[index1] = solution[index2];
    solution[index2] = temp;
} /* swapJobs */

// Runs the local search
void LocalSearch(int *solution)
{
    int improved_swap = 1;
    int improved_insertion = 0;
    unsigned int best_improvement;
    unsigned int current_improvement;
    int index1, index2;
    int t = 0;

    while((improved_swap || improved_insertion))
    {
        t++;
        //Job Swap
        best_improvement = 0;
        for(int i = 0; i < n_jobs - 1; i++)
        {
            for(int j = i + 1; j < n_jobs; j++)
            {
                current_improvement = evaluateSwap(solution, i, j);
                if(current_improvement > best_improvement)
                {
                    best_improvement = current_improvement;
                    index1 = i;
                    index2 = j;
                }
            }
        }
        if (best_improvement > 0)
            swapJobs(solution, index1, index2);
        else
            improved_swap = 0;
        
        // Job Insertion
        best_improvement = 0;
        for(int i = 0; i < n_jobs; i++)
        {
            current_improvement = evaluateInsertion(solution, i, &index2);
            if(current_improvement > best_improvement)
            {
                best_improvement = current_improvement;
                index1 = i;
            }
        }
        if(best_improvement != 0)
            insertJob(solution, index1, index2);
        else
            improved_insertion = 0;
    }
} /* LocalSearch */

void resetPheromones()
{
    for(int i = 0; i < n_jobs; i++)
        jobs[i].pheromone = 1;
}

void LSACO()
{
    resetPheromones();
    best_tardiness = UINT_MAX;
    best_aco_tardiness = UINT_MAX;
    unsigned int iter_best_tardiness;
    int *iter_best_solution = (int*)malloc(n_jobs * sizeof(int));

    for(int t = 0; t < ITERS_ACO; t++)
    {
        ACO(iter_best_solution);
        iter_best_tardiness = calcTotalTardiness(iter_best_solution, 0);
        if(iter_best_tardiness < best_aco_tardiness)
            best_aco_tardiness = iter_best_tardiness;

        LocalSearch(iter_best_solution);
        iter_best_tardiness = calcTotalTardiness(iter_best_solution, 0);
        if(iter_best_tardiness < best_tardiness)
        {
            best_tardiness = iter_best_tardiness;
            memcpy(best_solution, iter_best_solution, n_jobs * sizeof(int));
        }

        // Updating Pheromones
        increasePheromones(iter_best_solution);
        evaporate();
        // printPheromones();
        // ===================
    }
    free(iter_best_solution);
}

int main(/*int argc, char const *argv[]*/)
{
    clock_t start, end;
    double cpu_time_used;
    
    srand(time(NULL)); // Initializing the random seed
    const char* inputs[N_INPUTS] = { 
        "instances/wt040/wt040_009.dat",
        "instances/wt040/wt040_023.dat",
        "instances/wt040/wt040_037.dat",
        "instances/wt040/wt040_045.dat",
        "instances/wt040/wt040_054.dat",
        "instances/wt040/wt040_067.dat",
        "instances/wt040/wt040_078.dat",
        "instances/wt040/wt040_086.dat",
        "instances/wt040/wt040_101.dat",
        "instances/wt040/wt040_112.dat",

        "instances/wt050/wt050_007.dat",
        "instances/wt050/wt050_019.dat",
        "instances/wt050/wt050_032.dat",
        "instances/wt050/wt050_041.dat",
        "instances/wt050/wt050_042.dat",
        "instances/wt050/wt050_045.dat",
        "instances/wt050/wt050_056.dat",
        "instances/wt050/wt050_074.dat",
        "instances/wt050/wt050_104.dat",
        "instances/wt050/wt050_110.dat",

        "instances/wt100/wt100_016.dat",
        "instances/wt100/wt100_026.dat",
        "instances/wt100/wt100_042.dat",
        "instances/wt100/wt100_051.dat",
        "instances/wt100/wt100_059.dat",
        "instances/wt100/wt100_072.dat",
        "instances/wt100/wt100_080.dat",
        "instances/wt100/wt100_089.dat",
        "instances/wt100/wt100_099.dat",
        "instances/wt100/wt100_118.dat",
     };

    // Iterates through the inputs
    for(int i = 0; i < N_INPUTS; i++)
    {
        start = clock();
        unsigned int best_of_input = UINT_MAX;
        float mean_aco = 0;

        /* Reading from the input file */
        const char *input_file = inputs[i];
        readInput(input_file);
        // ==============================

        printf("================================================\n");
        printf("FILE: %s\n", inputs[i]);
        printf("================================================\n");
        printJobs();

        for(int i = 0; i < N_RUNS; i++)
        {
            LSACO();
            printf("========================\n");
            printf("LSACO Iteration %d\n", i + 1);
            printf("========================\n");
            printf("Solution:\n");
            for(int i = 0; i < n_jobs; i++)
                printf("%d ", best_solution[i]);
            mean_aco += best_aco_tardiness;
            printf("\nBest ACO tardiness: %d\n", best_aco_tardiness);
            printf("Solution tardiness: %d\n", calcTotalTardiness(best_solution, 0));
            printf("========================\n");
            printf("========================\n\n");

            if(best_tardiness < best_of_input)
                best_of_input = best_tardiness;
        }

        mean_aco /= N_RUNS;
        printf("FILE: %s\n", inputs[i]);
        printf("Mean solution provided by ACO: %f\n", mean_aco);
        printf("Best solution found for file: %d\n", best_of_input);
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("CPU MEAN TIME: %fs\n", cpu_time_used / N_RUNS);
        printf("================================================\n");
        printf("================================================\n\n");
        
    }

    /* Freeing memory */
    free(best_solution);
    free(jobs);
    // =================

    
    return 0;
}
