#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <string.h>
#include <unistd.h>

 #include <execinfo.h>

int main()
{
    // All needed variables.
    int socket_listen_fd,
        portno = 1815,
        client_len,
        socket_connection_fd,
        kq,
        new_events;
    struct kevent change_event[1],
        event[1];
    struct sockaddr_in serv_addr,
        client_addr;

    // Create socket.
    if (((socket_listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0))
    {
        perror("ERROR opening socket");
        exit(1);
    }

    // Create socket structure and bind to ip address.
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(socket_listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    // Start listening.
    listen(socket_listen_fd, 3);
    client_len = sizeof(client_addr);

    // Prepare the kqueue.
    kq = kqueue();

    // Create event 'filter', these are the events we want to monitor.
    // Here we want to monitor: socket_listen_fd, for the events: EVFILT_READ 
    // (when there is data to be read on the socket), and perform the following
    // actions on this kevent: EV_ADD and EV_ENABLE (add the event to the kqueue 
    // and enable it).
    EV_SET(change_event, socket_listen_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    // Register kevent with the kqueue.
    if (kevent(kq, change_event, 1, NULL, 0, NULL) == -1)
    {
        perror("kevent");
        exit(1);
    }

    int occurred_events = 0;

    // Actual event loop.
    for (;;)
    {
        // Check for new events, but do not register new events with
        // the kqueue. Hence the 2nd and 3rd arguments are NULL, 0.
        // Only handle 1 new event per iteration in the loop; 5th
        // argument is 1.
        if ((new_events = kevent(kq, NULL, 0, event, 1, NULL)) < 0 ) {
            perror("checking for new events error");
            exit(1);
        };
    
        // if (new_events > 1) {
        //     printf("new events: %d", new_events);
        // }

        // printf("amount of new events: %d\n", new_events);
        for (int i = 0; new_events > i; i++)
        {
            int event_fd = event[i].ident;

            // When the client disconnects an EOF is sent. By closing the file
            // descriptor the event is automatically removed from the kqueue.
            if (event[i].flags & EV_EOF)
            {
                printf("Client has disconnected\n");
                printf("amount of events: %d\n", occurred_events);
                close(event_fd);
            }
            // If the new event's file descriptor is the same as the listening
            // socket's file descriptor, we are sure that a new client wants 
            // to connect to our socket.
            else if (event_fd == socket_listen_fd)
            {
                // printf("New connection coming in...\n");    

                // Incoming socket connection on the listening socket.
                // Create a new socket for the actual connection to client.
                socket_connection_fd = accept(event_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
                if (socket_connection_fd == -1)
                {
                    perror("Accept socket error");
                }

                // Put this new socket connection also as a 'filter' event
                // to watch in kqueue, so we can now watch for events on this
                // new socket.
                EV_SET(change_event, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0)
                {
                    perror("kevent error");
                }

                // char buf[] = "xxxx\n";
                // send(socket_connection_fd, buf, strlen(buf), 0);
            }

            else if (event[i].filter & EVFILT_READ)
            {
                occurred_events++;
                // Read bytes from socket
                int maxlen = 1000;
                char buf[maxlen];
                size_t bytes_read = recv(event_fd, buf, maxlen, 0);
                send(event_fd, buf, bytes_read, 0);
            }
        }
    }

    return 0;
}
