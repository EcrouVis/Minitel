#ifndef M12_THREADS_H
#define M12_THREADS_H
#include "thread_messaging.h"
void thread_circuit_main(Mailbox* ,Mailbox* ,Mailbox* ,Mailbox* ,GlobalState*);
void thread_video_main(Mailbox* ,Mailbox* ,GlobalState*);
void thread_audio_main(Mailbox* ,Mailbox* ,GlobalState*);
void thread_log_main(Mailbox* ,GlobalState*);
#endif