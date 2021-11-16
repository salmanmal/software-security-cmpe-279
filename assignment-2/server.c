// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    char *nobody_user = "nobody";

    printf("Program started\n");
    if (argc > 1)
    {
        if (strcmp(argv[0], "listen") == 0)
        {
            printf("it is child process\n");
            new_socket = atoi(argv[1]);
            valread = read(new_socket, buffer, 1024);
            printf("Read %d bytes: %s\n", valread, buffer);
            send(new_socket, hello, strlen(hello), 0);
            printf("Hello message sent\n");
            exit(EXIT_SUCCESS);
        }
    }

    printf("it is parent process\n");

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();
    if (child_pid < 0)
    {
        perror("child process creation failed");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        struct passwd *nobody = getpwnam("nobody");
        if (nobody == NULL)
        {
            perror("nobody user does not exists.");
            exit(EXIT_FAILURE);
        }

        printf("nobody uid: %u\n", nobody->pw_uid);

        if (setuid(nobody->pw_uid) < 0)
        {
            perror("Error while setting uid");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Listening for client...\n");
        }

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char socket_arg[20];
        sprintf(socket_arg, "%d", new_socket);

        char *arguments[] = {"listen", socket_arg, NULL};
        execvp(argv[0], arguments);
    }
    else
    {
        int returnStatus;
        waitpid(child_pid, &returnStatus, 0);
        printf("parent process terminated\n");
        exit(EXIT_SUCCESS);
    }

    return 0;
}
//cc -o server server.c