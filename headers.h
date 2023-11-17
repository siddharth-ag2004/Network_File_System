#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <linux/limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include<stdint.h>

#include "trie.h"
// #include "stack.h"
#include "seek.h"
#include "socketAPI.h"
#include "errorHandler.h"
#include "communicationProtocols.h"