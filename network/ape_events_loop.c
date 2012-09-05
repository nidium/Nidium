#include "common.h"
#include "ape_events.h"
#include "ape_socket.h"
#include "ape_timers.h"

#include <sys/time.h>
#include <stdio.h>

extern int ape_running;

void events_loop(ape_global *ape)
{
    int nfd, fd, bitev;
    long int ticks = 0, uticks = 0, lticks = 0;
    
    void *attach;
    struct timeval t_start, t_end;

    printf("Start socket loop\n");
    
    gettimeofday(&t_start, NULL);
    while(ape->is_running && ape_running) {
        int i;

        if ((nfd = events_poll(&ape->events, 1)) == -1) {
            continue;
        }
        for (i = 0; i < nfd; i++) {
			
			if ((attach  = events_get_current_fd(&ape->events, i)) == NULL) {
				continue;
			}
			
            bitev   = events_revent(&ape->events, i);
			//if (attach == NULL) printf("Will failed\n");
            fd  = ((ape_fds *)attach)->fd; /* assuming that ape_fds is the first member */
			//printf("Getting : %d on %d %d\n", i, fd, bitev);
			//if ((ape_fds *)attach)->state == ()
            switch(((ape_fds *)attach)->type) {

            case APE_SOCKET:
                if (APE_SOCKET(attach)->states.type == APE_SOCKET_TP_SERVER) {
                    if (bitev & EVENT_READ) {
                        if (APE_SOCKET(attach)->states.proto == APE_SOCKET_PT_TCP ||
							APE_SOCKET(attach)->states.proto == APE_SOCKET_PT_SSL) {
								
                            ape_socket_accept(APE_SOCKET(attach));
                        } else {
                            printf("read on UDP\n");
                        }
                    }
                } else if (APE_SOCKET(attach)->states.type == APE_SOCKET_TP_CLIENT) {

                    
                    /* unset this before READ event because read can invoke writes */
                    if (bitev & EVENT_WRITE) {
                        APE_SOCKET(attach)->states.flags &= ~APE_SOCKET_WOULD_BLOCK;
                    }

                    if (bitev & EVENT_READ &&
                        ape_socket_read(APE_SOCKET(attach)) == -1) {
                        
                        /* ape_socket is planned to be release after the for block */
                        continue;
                    }

                    if (bitev & EVENT_WRITE) {
                        if (APE_SOCKET(attach)->states.state == APE_SOCKET_ST_ONLINE &&
                                !(APE_SOCKET(attach)->states.flags & APE_SOCKET_WOULD_BLOCK)) {

                            ape_socket_do_jobs(APE_SOCKET(attach));
                            
                        } else if (APE_SOCKET(attach)->states.state == APE_SOCKET_ST_PROGRESS) {
                            int serror = 0, ret;
                            socklen_t serror_len = sizeof(serror);

                            if ((ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &serror, &serror_len)) == 0 &&
                                serror == 0) {

                                APE_SOCKET(attach)->states.state = APE_SOCKET_ST_ONLINE;

                                ape_socket_connected(APE_SOCKET(attach));
                                
                            } else {
                                printf("Failed to connect\n");
                            }
                        }
                    }
                } else if (APE_SOCKET(attach)->states.type == APE_SOCKET_TP_UNKNOWN) {

                }

                break;
            case APE_FILE:
                break;
            case APE_DELEGATE:
                ((struct _ape_fd_delegate *)attach)->on_io(fd, bitev, ape); /* punning */
                break;
            }
        }
        
		gettimeofday(&t_end, NULL);
		
		ticks = 0;
		
		uticks = 1000000L * (t_end.tv_sec - t_start.tv_sec);
		uticks += (t_end.tv_usec - t_start.tv_usec);
		t_start = t_end;
		lticks += uticks;

		while (lticks >= 1000) {
			lticks -= 1000;
			process_tick(ape);
		}        
    }
}

// vim: ts=4 sts=4 sw=4 et

