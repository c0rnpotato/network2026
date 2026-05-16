// Wireless Network HW4(2026-1)
// 2023066980 Youngjin Kim

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#ifndef NUM_STATIONS
#define NUM_STATIONS 300
#endif

#ifndef SLOT_SIZE_MS
#define SLOT_SIZE_MS 10
#endif

#ifndef SIMULATION_SLOTS
#define SIMULATION_SLOTS 10000
#endif

#ifndef NUM_G_VALUES
#define NUM_G_VALUES 17
#endif

const double G_VALUES[NUM_G_VALUES] = {
    0.2, 0.4, 0.6, 0.8, 1.0,
    1.2, 1.4, 1.6, 1.8, 2.0,
    3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

enum ChannelState
{
    FREE,
    FULL,
    FAIL,
    GEN
};
typedef enum
{
    NON_PERSISTENT,
    P_01_PERSISTENT,
    P_05_PERSISTENT,
    ONE_PERSISTENT
} CSMA_Type;

struct channel_info
{
    int tic_per_slot;
    double p;
};
struct channel_in_tic
{
    int req;
    enum ChannelState state;
};

struct answer
{
    double G[NUM_G_VALUES];
    double S_pure[NUM_G_VALUES];
    double S_slotted[NUM_G_VALUES];
    double S_nonpersistent[NUM_G_VALUES];
    double S_01persistent[NUM_G_VALUES];
    double S_05persistent[NUM_G_VALUES];
    double S_1persistent[NUM_G_VALUES];
};

struct channel_in_tic *malloc_channel(struct channel_info ch_info)
{
    int size = (SIMULATION_SLOTS + 10) * ch_info.tic_per_slot;
    struct channel_in_tic *channel = malloc(size * sizeof(struct channel_in_tic));
    if (channel == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < size; i++)
    {
        channel[i].req = 0;
        channel[i].state = FREE;
    }
    return channel;
}
void free_channel(struct channel_in_tic *channel) { free(channel); }

bool rand_gen(double p) { return (rand() / (double)RAND_MAX) < p; }

enum ChannelState sim_1tic(struct channel_in_tic *channel, int tic, struct channel_info ch_info)
{
    for (int i = 0; i < NUM_STATIONS; i++)
        channel[tic].req += rand_gen(ch_info.p);

    if (channel[tic].req == 1)
        channel[tic].state = FULL;
    else if (channel[tic].req == 0)
        channel[tic].state = FREE;
    else
        channel[tic].state = FAIL;

    return channel[tic].state;
}

double success_rate(double G, struct channel_info ch_info)
{
    int success = 0;
    int using = 0;
    bool score = 0;
    ch_info.p = G / (double)NUM_STATIONS / (double)ch_info.tic_per_slot;
    struct channel_in_tic *channel = malloc_channel(ch_info);

    for (int i = 0; i < SIMULATION_SLOTS * ch_info.tic_per_slot; i++)
    {
        enum ChannelState tmp = sim_1tic(channel, i, ch_info);
        if (tmp == FULL)
            if (using == 0)
            {
                success++;
                score = 1;
                using = ch_info.tic_per_slot - 1;
            }
            else if (score == 0)
            {
                using = ch_info.tic_per_slot - 1;
            }
            else
            {
                success--;
                score = 0;
                using = ch_info.tic_per_slot - 1;
            }
        else if (tmp == FAIL)
            if (using == 0)
            {
                using = ch_info.tic_per_slot - 1;
            }
            else if (score == 0)
            {
                using = ch_info.tic_per_slot - 1;
            }
            else
            {
                success--;
                score = 0;
                using = ch_info.tic_per_slot - 1;
            }
        else if (tmp == FREE && using > 0)
            using--;
    }

    double rate = (double)success / SIMULATION_SLOTS;
    free_channel(channel);

    return rate;
}

double simulate_csma(double G, int tic_per_slot, CSMA_Type type)
{
    int total_tics = SIMULATION_SLOTS * tic_per_slot;
    int *tx_starts = calloc(total_tics, sizeof(int));
    if (tx_starts == NULL)
        return 0.0;

    double p = G / (double)NUM_STATIONS / (double)tic_per_slot;
    int deferred_queue = 0;
    int success_count = 0;

    double p_persist = 1.0;
    if (type == P_01_PERSISTENT)
        p_persist = 0.1;
    else if (type == P_05_PERSISTENT)
        p_persist = 0.5;

    bool prev_busy = false;

    for (int i = 0; i < total_tics; i++)
    {
        int new_arrivals = 0;
        for (int s = 0; s < NUM_STATIONS; s++)
        {
            if (rand_gen(p))
            {
                new_arrivals++;
            }
        }

        bool busy = false;
        for (int t = i - tic_per_slot + 1; t <= i - 1; t++)
        {
            if (t >= 0 && tx_starts[t] > 0)
            {
                busy = true;
                break;
            }
        }

        if (type == NON_PERSISTENT)
        {
            if (!busy)
            {
                tx_starts[i] += new_arrivals;
            }
        }
        else if (type == ONE_PERSISTENT)
        {
            if (busy)
            {
                deferred_queue += new_arrivals;
            }
            else
            {
                tx_starts[i] += (new_arrivals + deferred_queue);
                deferred_queue = 0;
            }
        }
        else if (type == P_01_PERSISTENT || type == P_05_PERSISTENT)
        {
            if (busy)
            {
                if (!prev_busy)
                {
                    deferred_queue = 0;
                }
                deferred_queue += new_arrivals;
            }
            else
            {
                int available_nodes = new_arrivals + deferred_queue;
                int transmitting = 0;
                for (int n = 0; n < available_nodes; n++)
                {
                    if (rand_gen(p_persist))
                    {
                        transmitting++;
                    }
                }
                tx_starts[i] += transmitting;
                deferred_queue = available_nodes - transmitting;
            }
        }

        if (tx_starts[i] == 1)
        {
            success_count++;
        }

        prev_busy = busy;
    }

    free(tx_starts);
    return (double)success_count / SIMULATION_SLOTS;
}

void save_log_file(struct answer ans)
{
    FILE *log_file = fopen("channel_sim_results.dat", "w");
    if (log_file == NULL)
    {
        perror("Error opening log file");
        return;
    }

    fprintf(log_file, "G S_slotted S_pure S_non S_01 S_05 S_1\n");

    for (int j = 0; j < NUM_G_VALUES; j++)
    {
        fprintf(log_file, "%f %f %f %f %f %f %f\n",
                ans.G[j],
                ans.S_slotted[j],
                ans.S_pure[j],
                ans.S_nonpersistent[j],
                ans.S_01persistent[j],
                ans.S_05persistent[j],
                ans.S_1persistent[j]);
    }

    fclose(log_file);
}

int main()
{
    srand(time(NULL));
    struct channel_info ch_info;
    struct answer ans;
    int tic_per_slot[] = {1, 100};

    for (int i = 0; i < sizeof(tic_per_slot) / sizeof(tic_per_slot[0]); i++)
    {
        ch_info.tic_per_slot = tic_per_slot[i];
        for (int j = 0; j < NUM_G_VALUES; j++)
        {
            if (i == 0)
                ans.S_slotted[j] = success_rate(G_VALUES[j], ch_info);
            else if (i == 1)
            {
                ans.S_pure[j] = success_rate(G_VALUES[j], ch_info);
                ans.G[j] = G_VALUES[j];
            }
        }
    }

    for (int j = 0; j < NUM_G_VALUES; j++)
    {
        ans.S_nonpersistent[j] = simulate_csma(G_VALUES[j], 100, NON_PERSISTENT);
        ans.S_01persistent[j] = simulate_csma(G_VALUES[j], 100, P_01_PERSISTENT);
        ans.S_05persistent[j] = simulate_csma(G_VALUES[j], 100, P_05_PERSISTENT);
        ans.S_1persistent[j] = simulate_csma(G_VALUES[j], 100, ONE_PERSISTENT);
    }

    save_log_file(ans);
    return 0;
}