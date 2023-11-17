#include "headers.h"

int port_for_clients;
int port_for_naming_server;
sem_t portc_lock;
sem_t portnms_lock;
int close_flag = 0;
TrieNode *ssTrie = NULL;

void *naming_server_responder_worker(void *arg)
{
    printf("here i am\n");
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    // char buffer[MAX_PATH_LENGTH];
    int n;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        // exit(1);
    }
    // printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        close(server_sock);
        // exit(1);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    while (1)
    {
        if (getsockname(server_sock, (struct sockaddr *)&sin, &len) == -1)
        {
            fprintf(stderr, "couldn't extract port of socket error\n");
            continue;
        }
        else
            break;
    }
    // port_for_naming_server = ntohs(sin.sin_port);
    // printf("port extracted is %d\n",sin.sin_port);
    port_for_naming_server = sin.sin_port;
    sem_post(&portnms_lock);
    // printf("[+]Bind to the port number: %d\n", port_for_naming_server);
    if (listen(server_sock, 5) < 0)
    {
        fprintf(stderr, "[-]Storage server got disconnected from Naming Server %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    printf("listening to respond to naming server\n");
    while (1)
    {

        addr_size = sizeof(client_addr);
        // printf("1\n");
        // printf("%d\n",port_for_naming_server);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        // printf("2\n");
        if (client_sock < 0)
        {
            fprintf(stderr, "[-]Accept error: %s\n", strerror(errno));
            if (close(server_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        // else
        // {
        //     printf("Naming server pinged once\n");
        // }
        close(client_sock);
        if (close_flag == 1)
            return NULL;
    }
    return NULL;
}

void *naming_server_informer_worker(void *arg)
{
    sem_wait(&portc_lock);
    sem_wait(&portnms_lock);
    int ss_sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[MAX_PATH_LENGTH];
    int n;

    ss_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_sock < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));

        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = nms_ss_port;
    addr.sin_addr.s_addr = inet_addr(ip_address);

    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "[-]Connection time error: %s\n", strerror(errno));
        exit(1);
    }
    printf("Connected to the server.\n");

    // ssTrie = createNode("storage_server_1");
    // InsertTrie("root/dir1/dir2/dir3",ssTrie);
    // InsertTrie("root/dir1/dir5/dir6",ssTrie);
    // InsertTrie("root/dir1/dir2/dir4",ssTrie);
    // InsertTrie("root/dir1/dir5/dir7",ssTrie);
    // InsertTrie("root/dir2/dir6/dir9",ssTrie);
    // InsertTrie("root/dir2/dir6/dir10",ssTrie);

    // bzero(buffer, MAX_PATH_LENGTH);
    // sprintf(buffer,"%d,%d",port_for_clients,port_for_naming_server);
    MessageSS2NM message;
    // strcpy(message.buffer, "storage_server_1");
    TrieToString(ssTrie, message.buffer);
    message.port_for_clients = port_for_clients;
    message.port_for_naming_server = port_for_naming_server;

    printf("Sending message to server: %d %d\n", message.port_for_clients, message.port_for_naming_server);

    if (send(ss_sock, &message, sizeof(message), 0) < 0)
    {
        fprintf(stderr, "[-]Send time error: %s\n", strerror(errno));
        if (close(ss_sock) < 0)
            fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        exit(1);
    }
    return NULL;
}

void *CLientServerConnection(void *arg)
{
    int client_sock = *(int *)arg;
    MessageClient2SS message;
    bzero(message.buffer, MAX_PATH_LENGTH);

    if (recv(client_sock, &message, sizeof(message), 0) < 0)
    {
        fprintf(stderr, "[-]Receive error: %s\n", strerror(errno)); // ERROR HANDLING
        if (close(client_sock) < 0)
            fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno)); // ERROR HANDLING
        exit(1);
    }

    if (message.operation == CREATE)
    {
        int fd = open(message.buffer, O_CREAT | O_WRONLY, 0644);
        if (fd == -1)
        {
            fprintf(stderr, "\x1b[31mCould not open %s. Permission denied\n\n\x1b[0m", message.buffer); // ERROR HANDLING
            return NULL;
        }
        InsertTrie(message.buffer, ssTrie);
        close(fd);
    }

    if (message.operation == READ)
    {
        int fd = open(message.buffer, O_RDONLY);
        if (fd == -1)
        {
            fprintf(stderr, "\x1b[31mCould not open %s. Permission denied\n\n\x1b[0m", message.buffer); // ERROR HANDLING
            return NULL;
        }
        char buffer[1024];
        int bytes_read;
        while ((bytes_read = read(fd, buffer, 1024)) > 0)
        {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }
        printf("\n");
        close(fd);
    }

    if (message.operation == WRITE)
    {
        int fd = open(message.buffer, O_WRONLY);
        if (fd == -1)
        {
            fprintf(stderr, "\x1b[31mCould not open %s. Permission denied\n\n\x1b[0m", message.buffer); // ERROR HANDLING
            return NULL;
        }
        char buffer[1024];
        int bytes_read;
        while ((bytes_read = read(fd, buffer, 1024)) > 0)
        {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }
        printf("\n");
        close(fd);
    }

    if (message.operation == DELETE)
    {
        int fd = open(message.buffer, O_RDONLY);
        if (fd == -1)
        {
            fprintf(stderr, "\x1b[31mCould not open %s. Permission denied\n\n\x1b[0m", message.buffer); // ERROR HANDLING
            return NULL;
        }
        closeSocket(fd);
        if (remove(message.buffer) == 0)
        {
            printf("\x1b[32mDeleted %s successfully\n\n\x1b[0m", message.buffer);
            DeleteTrie(message.buffer, ssTrie);
        }
        else
            printf("\x1b[31mCould not delete %s\n\n\x1b[0m", message.buffer);
    }
}

void *clients_handler_worker(void *arg)
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    // char buffer[MAX_PATH_LENGTH];
    int n;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        // exit(1);
    }
    // printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        close(server_sock);
        // exit(1);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    while (1)
    {
        if (getsockname(server_sock, (struct sockaddr *)&sin, &len) == -1)
        {
            fprintf(stderr, "couldn't extract port of socket error\n");
            continue;
        }
        else
            break;
    }
    port_for_clients = sin.sin_port;
    // printf("port extracted is %d\n", port_for_clients);
    sem_post(&portc_lock);
     if (listen(server_sock, 5) < 0)
    {
        fprintf(stderr, "[-]Storage server got disconnected from Naming Server %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    printf("listening to respond to clients\n");
    // listen for new clients and make a new thread if a client is found and do ss-client communication part in the worker function of that new thread
    while (1)
    {
        addr_size = sizeof(client_addr);
        // printf("1\n");
        // printf("%d\n",port_for_naming_server);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        // printf("2\n");
        if (client_sock < 0)
        {
            fprintf(stderr, "[-]Accept error: %s\n", strerror(errno));
            if (close(server_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        pthread_t client_service_thread;
        pthread_create(&client_service_thread,NULL,CLientServerConnection,(void*)&client_sock);
    }
    return NULL;
}

void GetAccessiblePaths()
{
    printf("Enter 1 to make all paths accessible\n");
    printf("Enter 2 to make some paths accessible\n");
    int choice;
    scanf("%d", &choice);
    char current_directory[MAX_PATH_LENGTH];
    char TEMPBUFF[MAX_PATH_LENGTH];
    getcwd(current_directory, sizeof(current_directory));
    if (choice == 1)
    {
        printf("All paths are accessible\n");
        // get current directory storage server is in
        printf("Current directory: %s\n", current_directory);
        // append ss1 to cwdd
        // sprintf(current_directory, "%s/%s", current_directory, "ss1");
        char port_in_string[10];
        sprintf(port_in_string,"%d",port_for_clients);
        ssTrie = createNode(port_in_string);
        lookFor(current_directory, strlen(current_directory), ssTrie);
    }
    else if (choice == 2)
    {
        // make some paths accessible
        printf("Enter the number of directory paths you want to make accessible\n");
        int num_directory;
        scanf("%d", &num_directory);
        printf("Enter the paths for directories\n");
        char paths[num_directory][MAX_PATH_LENGTH];
        for (int i = 0; i < num_directory; i++)
        {
            scanf("%s", paths[i]);
            //handle paths being relative
            TEMPBUFF[0] = '\0';
            strcat(TEMPBUFF, current_directory);
            if (paths[i][0] != '/')
            {
                strcat(TEMPBUFF, "/");
            }
            strcat(TEMPBUFF, paths[i]);
            strcpy(paths[i], TEMPBUFF);
        }
        char port_in_string[10];
        sprintf(port_in_string,"%d",port_for_clients);
        ssTrie = createNode(port_in_string);
        for (int i = 0; i < num_directory; i++)
        {
            InsertTrie(paths[i] + strlen(current_directory), ssTrie);
            lookFor(paths[i], strlen(current_directory), ssTrie);
        }

        printf("Enter the number of files you want to make accessible\n");
        int num_files;
        scanf("%d", &num_files);
        printf("Enter the paths for files\n");
        char files[num_files][MAX_PATH_LENGTH];
        for (int i = 0; i < num_files; i++)
        {
            scanf("%s", files[i]);
        }
        for (int i = 0; i < num_files; i++)
        {
            InsertTrie(files[i], ssTrie);
        }

        // PrintTrie(ssTrie);
    }
    else
    {
        printf("Invalid choice\n"); // ERROR HANDLING
        // GetAccessiblePaths();
    }
}
int main()
{
    GetAccessiblePaths();
    pthread_t clients_handler, naming_server_informer, naming_server_responder;
    sem_init(&portc_lock, 0, 0);
    sem_init(&portnms_lock, 0, 0);
    pthread_create(&clients_handler, NULL, clients_handler_worker, NULL);
    pthread_create(&naming_server_responder, NULL, naming_server_responder_worker, NULL);
    pthread_create(&naming_server_informer, NULL, naming_server_informer_worker, NULL);
    while (1)
    {
    }
}