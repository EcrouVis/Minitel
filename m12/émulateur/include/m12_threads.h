#ifndef M12_THREADS_H
#define M12_THREADS_H
#include "thread_messaging.h"
void thread_circuit_main(thread_mailbox* ,thread_mailbox* ,thread_mailbox* ,thread_mailbox* ,GlobalState*);
void thread_video_main(thread_mailbox* ,thread_mailbox* ,GlobalState*);
void thread_audio_main(thread_mailbox* ,thread_mailbox* ,GlobalState*);
void thread_log_main(thread_mailbox* ,GlobalState*);
#endif