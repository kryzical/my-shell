#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_execute(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit
};

int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args) {
    int i;
    printf("Brians Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < shell_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    return 1;
}

int shell_exit(char **args) {
    return 0;
}

void shell_loop(void){
    char *line;
    char **args;
    int status;

    printf("[-----------------------------]\n");
    printf("\033[1;31m");
    printf("|--Welcome to Brian's Shell!--|\n");
    printf("\033[0m");
    printf("|----- helpful commands ------|\n");
    printf("|    cd - change dir          |\n");
    printf("|    help - help menu         |\n");
    printf("|    exit - exit shell        |\n");
    printf("-------------------------------\n");


    do {
        printf("> ");
        line = shell_read_line(); //read a line
        args = shell_split_line(line); //split line into arguments
        status = shell_execute(args); //execute the arguments

        free(line);
        free(args);
    } while (status == 1);
}

char *shell_read_line(void){
    int buffer_size = 1024;
    int pos = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if (buffer == NULL) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        } else {
            buffer[pos] = c;
        }
        pos++;

        if (pos >= buffer_size) {
            buffer_size += 1024;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **shell_split_line(char *line) {
    int buffer_size = 64;
    int pos = 0;
    char **tokens = malloc(buffer_size * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }    

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= buffer_size) {
            buffer_size += 64;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[pos] = NULL;
    return tokens;
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("shell");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
} 

int shell_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return shell_launch(args);
}

int main(int argc, char **argv) {
   

    shell_loop();

    return EXIT_SUCCESS;
}