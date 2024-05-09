#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int showMenu(){
    printf("Choose an operation:\n1. List poems.\n2. Add a new poem.\n3. Update an existing poem.\n4. Delete poem\n5. Close.\n");
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
    char poem[100];

    printf("Please, write a poem:\n");
    fgets(poem, sizeof(poem), stdin);

    if (poem[strlen(poem) - 1] == '\n')
        poem[strlen(poem) - 1] = '\0';

    char newPoem[100];
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

    char line[100];
    int current_line = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (current_line != line_number) {
            fputs(line, temp);
        } else {
            char updated_poem[100];
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

    char line[100];
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

int main(int argc,char** argv){
    if(argc !=2){
        perror("You need to give a name of file with poems!");
        exit(1);
    }

    int input, f, count;
    input = showMenu();
    if(input != 5){ 
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
            printf("Goodbye!\n");
            close(f);
            exit(0);
        default:
            printf("Invalid choice.\n");
            return 0;
    }
}