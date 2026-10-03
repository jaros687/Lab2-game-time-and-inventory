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
void show_inventory(const int inv[10]) {
    for (int i = 0; i < 10; i++) {
        printf("Слот %d: [%d]%s\n", i, inv[i], name_of(inv[i]));
    }
}

void show_clock(int day, int hour) {
    printf("Сейчас День %d, %02d:00\n", day, hour);
}

void work(int *day, int *hour) {
    int h = ask_int("Сколько часов работаем? ");

    if (h <= 0) {
        printf("Здесь не должно быть таких чисел. Попробуй положительное число.\n");
        return;
    }

    int total = *hour + h;
    *day  += total / 24;
    *hour  = total % 24;

    printf("Отработали %d ч. День %d, %02d:00\n", h, *day, *hour);
}
