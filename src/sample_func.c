#include "sample_func.h"


void init_Sample_Playback(){ //todo make init variables parameters
    
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,4096);
    Mix_AllocateChannels(DEFAULT_CHANNEL_NO+1); // Allocate channels + queue buffer
    printf("Allocated channels %d\n", Mix_AllocateChannels(DEFAULT_CHANNEL_NO+1));
    queue_Count = 0;
    Mix_ReserveChannels(1); // Reserve a buffer channel for a possible queue to go
}

void close_Sample_Playback(){

    Mix_CloseAudio();
    delete_Table(); //delete the table
}

int play_Sample_Timed(const char* file,int loops, int mtime){ 

    Entry* e = hash_lookup(file);
    if(e==NULL)
        return 1;
    
    Mix_PlayChannelTimed(-1 ,e->chunk,loops,mtime);   
    return 0;
}

SampleQueue* init_Queue(){

    /*
    Alloc space in memory
    Assign it reserved channel
    Reserve new buffer channel
    return new queue
    */
    SampleQueue* q = (SampleQueue*) malloc(sizeof(SampleQueue));
    q->channel = queue_Count++;
    q->length = 0;
    q->head = NULL;
    q->tail = NULL;
    Mix_AllocateChannels(DEFAULT_CHANNEL_NO+1+queue_Count);
    Mix_ReserveChannels(queue_Count+1);
     
    return q;

}

int enqueue_Sample(const char* file, int mtime, SampleQueue* sq){

    SampleInfo* si = (SampleInfo*)malloc(sizeof(SampleInfo));
    Entry *e = hash_lookup(file);
    if(e==NULL){
        free(si);
        return -1;
    }

    si->mtime = mtime;
    si->file = file;
    si->next = NULL;

    if(sq->head == NULL){
        sq->head = si;
        sq->tail = si;
        sq->length = 1;
        printf_Q(sq);
        return sq->length;
    }
    
    sq->tail->next = si;
    sq->tail = si;
    sq->length += 1;
    printf_Q(sq);
    return sq->length;
   
}

void dequeue_Sample(SampleQueue* sq){

    if(sq->length==0)
        return;

    SampleInfo* s = sq->head;
    sq->head = sq->head->next;
    sq->length -= 1;
    printf_Q(sq);
    free(s);
    return;
}

void handle_Queue(SampleQueue* sq){

    if(sq->length == 0)
        return;

    if(Mix_Playing(sq->channel))
        return;
    
    printf("Play sample on channel %d\n",sq->channel);
    Entry* e = hash_lookup(sq->head->file);
    Mix_PlayChannelTimed(sq->channel, e->chunk,0,sq->head->mtime);
    dequeue_Sample(sq);

    return;
}

void printf_Q(SampleQueue* sq){

    printf("QUEUE LENGTH: %d, CHANNEL: %d\n",sq->length,sq->channel);
    
    if(sq->head != NULL){
        SampleInfo* s = sq->head;
        printf("%s",s->file);
        while(s->next != NULL){
            s=s->next;
            printf(", %s",s->file);
        }
        printf("\n");
    }
}