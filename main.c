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

const char* name_of(int id) {
    switch (id) {
        case 0:  return "";
        case 1:  return " (Дерево)";
        case 2:  return " (Камень)";
        case 3:  return " (Семена)";
        case 4:  return " (Яблоко)";
        case 5:  return " (Железо)";
        case 6:  return " (Уголь)";
        case 7:  return " (Пшеница)";
        case 8:  return " (Инструмент)";
        case 9:  return " (Ткань)";
        default: return " (Такого нету)";
    }
}
