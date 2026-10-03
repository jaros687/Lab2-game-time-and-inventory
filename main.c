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
void put_item(int inv[10]) {
    int slot = ask_int("В какой слот (0-9)? ");

    if (slot < 0 || slot >= 10) {
        printf("Такого слота нет. Есть только 0..9.\n");
        return;
    }

    int id = ask_int("Какой ID предмета (0-9)? ");
    if (id < 0 || id > 9) {
        printf("ID бывает от 0 до 9. Я больше не сделал.\n");
        return;
    }

    inv[slot] = id;
    printf("Ок, в слот %d положили [%d]%s\n", slot, id, name_of(id));
}

void drop_item(int inv[10]) {
    int slot = ask_int("Какой слот очистить (0-9)? ");

    if (slot < 0 || slot >= 10) {
        printf("Нет такого слота.\n");
        return;
    }

    inv[slot] = 0;
    printf("Слот %d теперь пустой.\n", slot);
}

void sort_inventory(int inv[10]) {
    printf("\nБыло:\n");
    show_inventory(inv);

    int write = 0;
    for (int read = 0; read < 10; read++) {
        if (inv[read] != 0) {
            inv[write] = inv[read];
            write++;
        }
    }

    for (int i = write; i < 10; i++) {
        inv[i] = 0;
    }

    printf("\nСтало:\n");
    show_inventory(inv);
}
void reverse_sort_inventory(int inv[10]) {
    printf("\nБыло:\n");
    show_inventory(inv);

    int write = 0;
    for (int read = 9; read >= 0; read--) {
        if (inv[read] != 0) {
            inv[write] = inv[read];
            write++;
        }
    }

    for (int i = write; i < 10; i++) {
        inv[i] = 0;
    }

    printf("\nСтало (сжато и перевёрнуто):\n");
    show_inventory(inv);
}

void print_menu(void) {
    printf("\n===== Весёлый фермер =====\n");
    printf("0 — Выйти\n");
    printf("1 — Посмотреть на часы\n");
    printf("2 — Поработать\n");
    printf("3 — Заглянуть в рюкзак\n");
    printf("4 — Положить предмет\n");
    printf("5 — Выбросить предмет\n");
    printf("6 — Убраться в рюкзаке (вариант)\n");
    printf("7 — перевернуть рюкзак\n"); 
    printf("===========================\n");
}

int main(void) {
    int day  = 1;
    int hour = 8;

    int inv[10] = { 0, 2, 0, 4, 0, 5, 0, 6, 0, 0 };

    int choice;

    do {
        print_menu();
        choice = ask_int(" ");

        switch (choice) {
            case 0:
                printf("Прощай и удачного урожая.\n");
                break;

            case 1:
                show_clock(day, hour);
                break;

            case 2:
                work(&day, &hour);
                break;

            case 3:
                show_inventory(inv);
                break;

            case 4:
                put_item(inv);
                break;

            case 5:
                drop_item(inv);
                break;

            case 6:
                sort_inventory(inv);
                break;
                
            case 7:
                reverse_sort_inventory(inv);
                break;
                
            default:
                printf("Такого пункта нет. Выбери из 0-7.\n");
                break;
        }

    } while (choice != 0);

    return 0;
}
