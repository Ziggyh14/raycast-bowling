#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "sample_hash_table.h"

#ifndef SAMPLE_FUNC_H
#define SAMPLE_FUNC_H


#define DEFAULT_CHANNEL_NO 16

void init_Sample_Playback();

void close_Sample_Playback();

/*
- Plays a sample from a given file,
- If it has not already been played it will allocate memory
- If it has already been used it will NOT reallocate new memory
- will play sample (loops+1) times
- will cut off sample after mtime ms, if sample length is longer
- RETURNS 0 on success, 1 on failure
*/
extern int play_Sample_Timed(const char* file, int loops, int mtime);

#define play_Sample(file,loops) play_Sample_Timed(file,loops,-1);

typedef struct SampleInfo{
    const char* file;
    int mtime;
    struct SampleInfo* next;
} SampleInfo;

typedef struct SampleQueue{
    SampleInfo* head;
    SampleInfo* tail;
    int length;
    int channel;
} SampleQueue;

int queue_Count; // Amount of queues initialised

/*
Initialse a SampleQueue and return a pointer to it
- samples in the queue are played on thier own channel, meaning other samples can be played without disturbing the queue
*/
SampleQueue* init_Queue();

/*
push sample to queue, plays after all samples pushed before it.
- file: file location of the sample
- mtime: max time a sample will play until it is cut off. 
- queue to push to (array of string constants)
sample may end before mtime has elapsed if it (and its loops) are shorter than mtime
  -1 will let sample play out entirely.
- returns queue position on sucess, -1 on failure.
*/
int enqueue_Sample(const char* file, int mtime, SampleQueue* sq);

void dequeue_Sample(SampleQueue* sq);

/*
Handles playing from a given SampleQueue
- Will play the next sample in the queue 
if the queue is not empty nothing the queue is playing is playing
- Then dequeue it
*/
void handle_Queue(SampleQueue* sq);

void printf_Q(SampleQueue* sq);
#endif
