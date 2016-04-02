#include <iostream>
#include <algorithm>
#include <cstdio>
#include <ctime>

#include <pthread.h>

#include "common/packing.h"
#include "common/log.h"

using namespace std;

PackingProblem problem;
PackingUtility utility;

struct Params
{
    PackingUtility utility;
};

Params thread_params[MaxThread];
pthread_t threads[MaxThread];

void *
tree_thread(void *p) 
{
    Params *params = (Params *)p;
    params->utility.SetPackingProblem(&problem);
    //params->utility.TreeSearch(params - thread_params);
    //params->utility.SolveSA(1.0, 0.02, 0.20, 200, false, params - thread_params);
    params->utility.MentoCarlo(2, 10, params - thread_params);
    return p;
}

int main()
{
    int	cases;
    scanf("%d", &cases);

    utility.SetPackingProblem(&problem);

    double sum = 0;
    double sum0 = 0;
    double sum1 = 0;
    double minRate = 100;
    double maxRate = 0;

    double sumTime = 0;
    double sumTime0 = 0;
    double sumTime1 = 0;
    double minTime = 1e10;
    double maxTime = 0;

    int count = 0;
    for (int index = 0; index < cases; ++index)
    {
        double	start = clock();
        problem.Input(stdin);

        reachEffort = 0;

        PackingState result;
        result.volume = 0;

        for (int i = 0; i < 2; ++i)
        {
            pthread_create(&threads[i], NULL, tree_thread, &thread_params[i]);
        }

        printf("simple blocks: %d complex blocks: %d\n", 
            problem.simpleBlockLen, problem.blockTable.size());
        Log(LogInfo, "simple blocks: %d complex blocks: %d\n", 
            problem.simpleBlockLen, problem.blockTable.size());
        for (int i = 0; i < 2; ++i) 
        {
            pthread_join(threads[i], NULL);

            PackingUtility &utility = thread_params[i].utility;
            //printf("stage%d effort: %d rate: %.4f%% time: %.2fs\n", i, utility.searchEffort, 
            //        utility.best.volume*100.0/problem.volume, utility.costTime);
            if (utility.best.volume > result.volume)
            {
                result = utility.best;
                if (i == 1)
                    ++count;
            }
        }
		
        double end = clock();
        double time = (end - start);
        if (time < 0)
            time += (unsigned(~0));
        time /= CLOCKS_PER_SEC;
        sumTime += time;

        int v = result.volume;
        double rate = v*100.0/problem.volume;
        sum += rate;

        sumTime0 += thread_params[0].utility.costTime;
        sumTime1 += thread_params[1].utility.costTime;
        double rate0 = thread_params[0].utility.best.volume * 100.0 / problem.volume;
        double rate1 = thread_params[1].utility.best.volume * 100.0 / problem.volume;
        sum0 += rate0;
        sum1 += rate1;
		
        if (rate < minRate)
            minRate = rate;
        if (rate > maxRate)
            maxRate = rate;

        if (time < minTime)
            minTime = time;
        if (time > maxTime)
            maxTime = time;

        printf("case %d: rate %.4f%% %.4f%% %.4f%% time %.2fs\n", 
            index, rate0, rate1, rate, time);
        //printf("stage0: rate: %.4f%% time: %.2fs\n", sum0/(index+1), sumTime0/(index+1));
        //printf("stage1: rate: %.4f%% time: %.2fs\n", sum1/(index+1), sumTime1/(index+1));
        printf("ave: rate: %.4f%% %.4f%% %.4f%% %d time: %.2fs\n", 
            sum0/(index+1), sum1/(index+1), sum/(index+1), count, sumTime/(index+1));
        //printf("best in stage1: %d\n", count);

        Log(LogInfo, "case %d: rate %.4f%% %.4f%% %.4f%% time %.2fs\n", 
            index, rate0, rate1, rate, time);
        //Log(LogInfo, "stage0: rate: %.4f%% time: %.2fs\n", sum0/(index+1), sumTime0/(index+1));
        //Log(LogInfo, "stage1: rate: %.4f%% time: %.2fs\n", sum1/(index+1), sumTime1/(index+1));
        Log(LogInfo, "ave: rate: %.4f%% %.4f%% %.4f%% %d time: %.2fs\n", 
            sum0/(index+1), sum1/(index+1), sum/(index+1), count, sumTime/(index+1));
        //Log(LogInfo, "best in stage1: %d\n", count);

        fflush(stdout);
    }

    printf("time: min %.2fs max %.2fs ave %.2fs\n", minTime, maxTime, sumTime/cases);
    printf("rate: min %.4f%% max %.4f%% ave %.4f%%\n", minRate, maxRate, sum/cases);

    return 0;
}
