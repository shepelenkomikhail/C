#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define POEM_BUFFER_SIZE 100

int showMenu(){
    printf("Choose an operation:\n1. List poems.\n2. Add a new poem.\n3. Update an existing poem.\n4. Delete poem\n5. Send a boy to Ferencvaros!.\n6. Close.\n");
    int input;
    scanf("%d", &input);
    getchar();
    return input;
}

void listPoems(int f){
    char c;
    while(read(f, &c, sizeof(c))){
        printf("%c", c);
    }
    printf("\n");
}

void addPoem(int f, int count){
    off_t end_position = lseek(f, 0, SEEK_END);
    char poem[POEM_BUFFER_SIZE];

    printf("Please, write a poem:\n");
    fgets(poem, sizeof(poem), stdin);

    if (poem[strlen(poem) - 1] == '\n')
        poem[strlen(poem) - 1] = '\0';

    char newPoem[POEM_BUFFER_SIZE];
    sprintf(newPoem, "%d. %s\n", count + 1, poem);

    write(f, newPoem, strlen(newPoem));
    printf("\nNew poem added!\n");
}

void updatePoem(char* filename, int line_number) {
    FILE *file = fopen(filename, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    char line[POEM_BUFFER_SIZE];
    int current_line = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (current_line != line_number) {
            fputs(line, temp);
        } else {
            char updated_poem[POEM_BUFFER_SIZE];
            printf("Enter the updated poem:\n");
            fgets(updated_poem, sizeof(updated_poem), stdin);
            fprintf(temp, "%d. %s", line_number, updated_poem);
        }
        current_line++;
    }

    fclose(file);
    fclose(temp);

    remove(filename);
    rename("temp.txt", filename);
    printf("Poem updated!\n");
}

void deletePoem(char* filename, int line_number) {
    FILE *file = fopen(filename, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    char line[POEM_BUFFER_SIZE];
    int current_line = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (current_line != line_number) {
            if (current_line > line_number) {
                fprintf(temp, "%d. %s", current_line - 1, line + 2);
            } else {
                fputs(line, temp);
            }
        }
        current_line++;
    }

    fclose(file);
    fclose(temp);

    remove(filename);
    rename("temp.txt", filename);
    printf("Poem deleted!\n");
}

int countLines(int f){
    char content;
    int count = 0;
    while(read(f, &content, sizeof(content))){
        if (content == '\n')
            count++;
    }
    lseek(f, 0, SEEK_SET);
    return count;
}

struct msgbuf {
    long mtype; 
    char mtext[POEM_BUFFER_SIZE];
};

void handler(int signum) {
    printf("I am Mama and I've recieved a SIGNAL %i!\n", signum);
}

void deletePoem1(char* poem) {
    FILE *file = fopen("poems.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[POEM_BUFFER_SIZE];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, poem) != 0) {
            fprintf(temp, "%s\n", line);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove("poems.txt");
        rename("temp.txt", "poems.txt");
        printf("Poem deleted!\n");
    } else {
        printf("Poem not found!\n");
    }
}

//Homework2
int forkFunction(){
    //Give a random name
    char *names[] = {
    "Denes Dibusz",
    "Varga Barnabás",
    "Kenan Kodro",
    "Ibrahim Cisse"
    };

    srand(time(NULL));
    int random = rand() % 4;
    int random1;
    int random2;
    char *poem1;
    char *poem2;

    char *name = names[random];
    printf("Mamma chose a bunny: %s\n", name);
   
    //Choose a random poem
    char *poems[5] = {NULL, NULL, NULL, NULL, NULL}; 
    int totalPoems = 0;
    
    FILE *f = fopen("poems.txt", "r");
    if (f == NULL) {
        printf("Could not open file poems.txt\n");
        return 1;
    }
    else
    {
        char line[POEM_BUFFER_SIZE];
        while (fgets(line, sizeof(line), f) && totalPoems < 5) {
            poems[totalPoems] = malloc(strlen(line) + 1);
            strcpy(poems[totalPoems], line);  
            totalPoems++;
        }
        fclose(f);

        if (totalPoems < 2) {
            printf("Not enough poems in the file\n");
            return 1;
        }

        random1 = rand() % totalPoems;
        random2;
        do {
            random2 = rand() % totalPoems;
        } while (random2 == random1);

        poem1 = poems[random1];
        poem2 = poems[random2];

        printf("Mamma chose a poem: %s", poem1);
        printf("Mamma chose a poem: %s\n", poem2);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Opening pipe error!");
        exit(EXIT_FAILURE);
    }

    key_t key = ftok("poems.txt", 1);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    pid_t child = fork();
    if (child < 0) {
        perror("The fork calling was not successful\n");
        exit(1);
    }  
    if (child > 0) {
        close(pipefd[0]); // Close the reading end in the parent
        signal(SIGUSR1, handler);
        pause(); //Waits till child arrives to Ferencvaros
        sleep(2);

        write(pipefd[1], poem1, strlen(poem1));
        write(pipefd[1], poem2, strlen(poem2));
        close(pipefd[1]);

        printf("I am Mama and I am sending 2 poems to Ferencvaros through PIPE!:\n%s%s\n", poem1, poem2);
        sleep(2);

        struct msgbuf msg;
        msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0);
        printf("\nI am Mama and I received a MESSAGE with a poem to delete:\n%s\n", msg.mtext);
        deletePoem1(msg.mtext);
        sleep(2);

        wait(NULL); // Waits until the end of the child process
        printf("Parent process ended\n");
        msgctl(msgid, IPC_RMID, NULL);

    } else {
        close(pipefd[1]); // Close the writing end in the child
        sleep(2); // Traveling to Ferencvaros
        kill(getppid(), SIGUSR1);
        printf("%s SIGNALing mama about arrival to Ferencvaros!\n", name);
        sleep(2);

        char buffer[2 * POEM_BUFFER_SIZE] = {0};
        int totalRead = 0;
        int bytesRead;
        while ((bytesRead = read(pipefd[0], buffer + totalRead, sizeof(buffer) - totalRead - 1)) > 0) {
            totalRead += bytesRead;
        }
        buffer[totalRead] = '\0'; // Ensure null-termination of the string

        // Close the reading end of the pipe
        close(pipefd[0]);

        // Split the buffer into poems based on newline characters
        char *receivedPoem1 = buffer;
        char *receivedPoem2 = strchr(buffer, '\n');
        if (receivedPoem2) {
            *receivedPoem2 = '\0';  // Null-terminate the first poem
            receivedPoem2++;        // Move to the beginning of the second poem
            char *endOfPoem2 = strchr(receivedPoem2, '\n');
            if (endOfPoem2) {
                *endOfPoem2 = '\0'; // Null-terminate the second poem
            }
        }

        printf("\nI am %s and I received 2 poems from Mama from PIPE!:\n%s\n%s\n", name, receivedPoem1, receivedPoem2);
        sleep(2);

        // Randomly choose a poem to send back to parent to delete
        int chosenPoemIndex = rand() % 2;
        char *selectedPoem = chosenPoemIndex ? receivedPoem1 : receivedPoem2;
        struct msgbuf msg = {1, ""};
        strncpy(msg.mtext, selectedPoem, sizeof(msg.mtext) - 1);
        msg.mtext[sizeof(msg.mtext) - 1] = '\0';
        msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

        printf("\nI am %s and I sent a MESSAGE with a poem to delete:\n%s\n", name, selectedPoem);
        
        char *notDeletedPoem = chosenPoemIndex ? receivedPoem2 : receivedPoem1;
        printf("\nI am %s and I am watering a family with this poem:\n%s\n", name, notDeletedPoem);
        printf("Child process ended\n");
        exit(0);
    }
    return 0;
}

int main(int argc,char** argv){
    if(argc !=2){
        perror("You need to give a name of file with poems!");
        exit(1);
    }

    int input, f, count;
    input = showMenu();
    if(input != 6){ 
        f=open(argv[1], O_RDWR);
        if(f<0){
            perror("Error while opening a file!");
            exit(1);
        }
    }
    
    switch (input){
        case 1:
            listPoems(f);
            break;
        case 2:
            count = countLines(f);
            addPoem(f, count);
            break;
        case 3:
            listPoems(f);
            printf("\nNumber of poem to update: ");
            int poem_number;
            scanf("%d", &poem_number);
            getchar();
            updatePoem(argv[1], poem_number);
            break;
        case 4:
            listPoems(f);
            printf("\nNumber of poem to delete: ");
            int poem_delete;
            scanf("%d", &poem_delete);
            getchar();
            deletePoem(argv[1], poem_delete);
            break;
        case 5:
            forkFunction();
        case 6:
            printf("Goodbye!\n");
            close(f);
            exit(0);
        default:
            printf("Invalid choice.\n");
            return 0;
    }
}