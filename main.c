#include <stdio.h>

int ask_int(const char *question) {
    int value;
    char c;

    while (1) {
        printf("%s", question);

        if (scanf("%d", &value) == 1) {
            while ((c = getchar()) != '\n' && c != EOF);
            return value;
        }
      
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Не думаю что это число. Попробуй ещё раз.\n");
    }
}
