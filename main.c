#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ITEMS       10
#define MAX_NAME_LEN    33 
#define MAX_ITEM_NAME   32
#define INV_SIZE        10
#define MENU_MIN        0
#define MENU_MAX        11

#define ITEMS_FILE  "items.txt"
#define DIARY_FILE  "diary.txt"
#define INPUT_FILE  "input.txt"
#define OUTPUT_FILE "output.txt"

#define MAX_SUPPLIES    256
#define MAX_SUPPLY_NAME 64

char item_names[MAX_ITEMS][MAX_NAME_LEN];

void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int ask_int(const char *question) {
    int value;
    while (1) {
        printf("%s", question);
        if (scanf("%d", &value) == 1) {
            clear_input();
            return value;
        }
        if (feof(stdin)) {
            printf("\nКонец сессии. Выход.\n");
            exit(0);
        }
        clear_input();
        printf("Не думаю что это число. Попробуй ещё раз.\n");
    }
}

void ask_string(const char *question, char *buf, int size) {
    if (size <= 1) { if (size == 1) buf[0] = '\0'; return; }

    printf("%s", question);
    if (fgets(buf, size, stdin) == NULL) {
        printf("\nВвод закрыт. Выход.\n");
        exit(0);
    }

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        clear_input();
    }
}

void ask_farmer_name(char *name, int size) {
    while (1) {
        ask_string("Введите имя фермера (до 32 символов): ", name, size);
        if (strlen(name) == 0) {
            printf("Имя не может быть пустым. Попробуйте снова.\n");
            continue;
        }
        return;
    }
}

void set_default_item_names(void) {
    const char *defaults[MAX_ITEMS] = {
        "Пусто", "Дерево", "Камень", "Семена_пшеницы", "Яблоко",
        "Железо", "Уголь", "Пшеница", "Инструмент", "Ткань"
    };
    for (int i = 0; i < MAX_ITEMS; i++) {
        strncpy(item_names[i], defaults[i], MAX_NAME_LEN - 1);
        item_names[i][MAX_NAME_LEN - 1] = '\0';
    }
}
int load_item_names_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[MAX_NAME_LEN];
    int idx = 0;
    while (idx < MAX_ITEMS && fgets(line, sizeof(line), f)) {
        /* убираем \n и \r */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;

        strncpy(item_names[idx], line, MAX_NAME_LEN - 1);
        item_names[idx][MAX_NAME_LEN - 1] = '\0';
        idx++;
    }
    fclose(f);

    if (idx < MAX_ITEMS) {
        char backup[MAX_ITEMS][MAX_NAME_LEN];
        memcpy(backup, item_names, sizeof(backup));
        set_default_item_names();
        for (int i = 0; i < idx; i++) {
            strncpy(item_names[i], backup[i], MAX_NAME_LEN - 1);
            item_names[i][MAX_NAME_LEN - 1] = '\0';
        }
    }
    return 0;
}

void load_catalog(char *farmer_name) {
    set_default_item_names();
    if (load_item_names_from_file(ITEMS_FILE) != 0) {
        printf("\n[!] Файл '%s' не найден. Использую стандарт.\n",
               ITEMS_FILE);
    } else {
        printf("\n[+] Предметы загружены из '%s'.\n", ITEMS_FILE);
    }
    (void)farmer_name;
}


const char* name_of(int id) {
    if (id < 0 || id >= MAX_ITEMS) return "Неизвестно";
    return item_names[id];
}

int find_id_by_name(const char *name) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(item_names[i], name) == 0) return i;
    }
    return -1;
}

const char* name_of(int id) {
    if (id < 0 || id >= MAX_ITEMS) return "Неизвестно";
    return item_names[id];
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

void reverse_inventory(int inv[10]) {
    printf("\nБыло:\n");
    show_inventory(inv);

    for (int i = 0; i < 10 / 2; i++) {
        int tmp      = inv[i];
        inv[i]       = inv[9 - i];
        inv[9 - i]   = tmp;
    }

    printf("\nСтало:\n");
    show_inventory(inv);
}


void print_menu(const char *farmer_name) {
    printf("\n===== Весёлый фермер: %s =====\n", farmer_name);
    printf(" 0 — Выйти\n");
    printf(" 1 — Посмотреть на часы\n");
    printf(" 2 — Поработать\n");
    printf(" 3 — Заглянуть в рюкзак\n");
    printf(" 4 — Положить предмет\n");
    printf(" 5 — Выбросить предмет\n");
    printf(" 6 — Убраться в рюкзаке\n");
    printf(" 7 — Перевернуть рюкзак\n");
    printf(" 8 — Поиск предмета\n");
    printf(" 9 — Записать в дневник\n");
    printf("10 — Расшифровать поставки\n");
    printf("11 — Сменить имя\n");
    printf("===========================\n");
}

/* ============ MAIN ============ */
int main(void) {
    char farmer_name[MAX_NAME_LEN] = "Безымянный";
    int  day  = 1;
    int  hour = 8;

    int inv[INV_SIZE] = { 0, 2, 0, 4, 0, 5, 0, 6, 0, 0 };

    printf("=== Приветсвую в игре 'Весёлый фермер' ===\n");
    load_catalog(farmer_name);
    ask_farmer_name(farmer_name, sizeof(farmer_name));
    printf("Привет, %s! День %d, %02d:00.\n",
           farmer_name, day, hour);

    int choice;
    do {
        print_menu(farmer_name);
        choice = ask_int("Ваш выбор: ");

        switch (choice) {
            case 0:
                printf("Прощай, %s! Удачного урожая.\n", farmer_name);
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
                reverse_inventory(inv);
                break;
            case 8:
                find_item_by_name(inv);
                break;
            case 9:
                write_diary(farmer_name, day, hour, inv);
                break;
            case 10:
                process_supplies();
                break;
            case 11:
                ask_farmer_name(farmer_name, sizeof(farmer_name));
                break;
            default:
                printf("Такого пункта нет. Выбери из %d-%d.\n",
                       MENU_MIN, MENU_MAX);
                break;
        }
    } while (choice != 0);

    return 0;
}
