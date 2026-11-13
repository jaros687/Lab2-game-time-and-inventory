#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ITEMS       10
#define MAX_NAME_LEN    33
#define MAX_ITEM_NAME   32
#define INV_SIZE        10
#define MENU_MIN        0
#define MENU_MAX        12

#define ITEMS_FILE  "items.txt"
#define DIARY_FILE  "diary.txt"
#define INPUT_FILE  "input.txt"
#define OUTPUT_FILE "output.txt"

#define MAX_SUPPLIES    256
#define MAX_SUPPLY_NAME 64

#define MAP_MIN_SIZE       5
#define MAP_MAX_SIZE       30
#define CH_WALL            '#'
#define CH_FLOOR           '.'
#define CH_TREE            'T'
#define CH_ROCK            'R'
#define CH_HERO            '@'
#define CH_TRAP_SHOWN      ','

#define OBSTACLE_CHANCE_MIN  15
#define OBSTACLE_CHANCE_MAX  30
#define TRAP_CHANCE_MIN       5
#define TRAP_CHANCE_MAX      10
#define TRAP_COLLAPSE_PCT    50

enum {
    MOVE_UNKNOWN  = -1,
    MOVE_BLOCKED  =  0,
    MOVE_OK       =  1,
    MOVE_COLLAPSE =  2
};

char item_names[MAX_ITEMS][MAX_NAME_LEN];

void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int ask_int(const char *question) {
    int value;
    while (1) {
        printf("%s", question);
        int r = scanf("%d", &value);
        if (r == EOF) {
            printf("\nКонец сессии. Выход.\n");
            exit(0);
        }
        if (r == 1) {
            clear_input();
            return value;
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

char ask_dir(void) {
    char buf[16];
    ask_string("Шаг (W/A/S/D, Q — домой): ", buf, sizeof(buf));
    if (buf[0] == '\0') return '?';
    return buf[0];
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

void load_catalog(void) {
    set_default_item_names();
    if (load_item_names_from_file(ITEMS_FILE) != 0) {
        printf("\n[!] Файл '%s' не найден. Использую стандарт.\n", ITEMS_FILE);
    } else {
        printf("\n[+] Список предметов из '%s'.\n", ITEMS_FILE);
    }
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

void show_inventory(const int inv[INV_SIZE]) {
    printf("\n--- Инвентарь ---\n");
    for (int i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n", i, inv[i], name_of(inv[i]));
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
    if (h > 24 * 365) {
        printf("Слишком много часов за раз. Нельзя больше чем %d.\n", 24 * 365);
        return;
    }

    int total = *hour + h;
    *day  += total / 24;
    *hour  = total % 24;
    printf("Отработали %d ч. День %d, %02d:00\n", h, *day, *hour);
}

void put_item(int inv[INV_SIZE]) {
    int slot = ask_int("В какой слот (0-9)? ");
    if (slot < 0 || slot >= INV_SIZE) {
        printf("Такого слота нет. Есть только 0..%d.\n", INV_SIZE - 1);
        return;
    }
    int id = ask_int("Какой ID предмета (0-9)? ");
    if (id < 0 || id >= MAX_ITEMS) {
        printf("ID бывает от 0 до %d.\n", MAX_ITEMS - 1);
        return;
    }
    inv[slot] = id;
    printf("Ок, в слот %d положили [%d] - %s\n", slot, id, name_of(id));
}

void drop_item(int inv[INV_SIZE]) {
    int slot = ask_int("Какой слот очистить (0-9)? ");
    if (slot < 0 || slot >= INV_SIZE) {
        printf("Нет такого слота.\n");
        return;
    }
    inv[slot] = 0;
    printf("Слот %d теперь пустой.\n", slot);
}

void sort_inventory(int inv[INV_SIZE]) {
    printf("\nБыло:\n");
    show_inventory(inv);

    int write = 0;
    for (int read = 0; read < INV_SIZE; read++) {
        if (inv[read] != 0) inv[write++] = inv[read];
    }
    for (int i = write; i < INV_SIZE; i++) inv[i] = 0;

    printf("\nСтало:\n");
    show_inventory(inv);
}

void reverse_inventory(int inv[INV_SIZE]) {
    printf("\nБыло:\n");
    show_inventory(inv);

    for (int i = 0; i < INV_SIZE / 2; i++) {
        int tmp = inv[i];
        inv[i] = inv[INV_SIZE - 1 - i];
        inv[INV_SIZE - 1 - i] = tmp;
    }

    printf("\nСтало:\n");
    show_inventory(inv);
}

void find_item_by_name(const int inv[INV_SIZE]) {
    char query[MAX_NAME_LEN];
    ask_string("Введите название предмета для поиска: ", query, sizeof(query));

    if (strlen(query) == 0) {
        printf("Пустая строка — искать нечего.\n");
        return;
    }

    int id = find_id_by_name(query);
    if (id < 0) {
        printf("Предмет '%s' не найден в каталоге.\n", query);
        return;
    }

    printf("Предмет '%s' имеет ID %d. Ищу в рюкзаке\n", query, id);

    int found = 0;
    for (int i = 0; i < INV_SIZE; i++) {
        if (inv[i] == id) {
            printf("  Найдено в слоте %d\n", i);
            found = 1;
        }
    }
    if (!found) printf("  В рюкзаке такого предмета нет.\n");
}

void write_diary(const char *farmer_name, int day, int hour,
                 const int inv[INV_SIZE]) {
    FILE *f = fopen(DIARY_FILE, "a");
    if (!f) {
        printf("[!] Не удалось открыть '%s' для записи.\n", DIARY_FILE);
        return;
    }

    fprintf(f, "========================================\n");
    fprintf(f, "Фермер: %s\n", farmer_name);
    fprintf(f, "День %d, %02d:00\n", day, hour);
    fprintf(f, "Инвентарь:\n");
    for (int i = 0; i < INV_SIZE; i++) {
        fprintf(f, "  Слот %d: [%d] - %s\n", i, inv[i], name_of(inv[i]));
    }
    fprintf(f, "========================================\n\n");
    fclose(f);
    printf("[+] Состояние записано в '%s'.\n", DIARY_FILE);
}

int process_supplies(void) {
    FILE *in = fopen(INPUT_FILE, "r");
    if (!in) {
        printf("[!] Файл '%s' не найден. Создайте его и повторите.\n", INPUT_FILE);
        return -1;
    }

    char  names[MAX_SUPPLIES][MAX_SUPPLY_NAME];
    long  counts[MAX_SUPPLIES];
    int   total = 0;

    char line[256];
    int line_no = 0;

    while (fgets(line, sizeof(line), in)) {
        line_no++;

        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        char name[MAX_SUPPLY_NAME];
        long amount;
        char extra;
        int parsed = sscanf(p, "%63s %ld %c", name, &amount, &extra);

        if (parsed != 2 || amount < 0) {
            printf("[!] Строка %d пропущена (неверный формат): %s",
                   line_no, line);
            continue;
        }

        if (total >= MAX_SUPPLIES) {
            printf("[!] Достигнут лимит поставок (%d). Остальные пропущены.\n",
                   MAX_SUPPLIES);
            break;
        }

        int found = -1;
        for (int i = 0; i < total; i++) {
            if (strcmp(names[i], name) == 0) { found = i; break; }
        }

        if (found >= 0) {
            counts[found] += amount;
        } else {
            strncpy(names[total], name, MAX_SUPPLY_NAME - 1);
            names[total][MAX_SUPPLY_NAME - 1] = '\0';
            counts[total] = amount;
            total++;
        }
    }
    fclose(in);

    printf("\n--- Сводная ведомость поставок ---\n");
    for (int i = 0; i < total; i++) {
        printf("%s: %ld шт.\n", names[i], counts[i]);
    }
    if (total == 0) printf("(нет данных)\n");

    FILE *out = fopen(OUTPUT_FILE, "w");
    if (!out) {
        printf("[!] Не удалось открыть '%s' для записи.\n", OUTPUT_FILE);
        return -1;
    }
    fprintf(out, "Сводная ведомость поставок за неделю\n");
    fprintf(out, "------------------------------------\n");
    for (int i = 0; i < total; i++) {
        fprintf(out, "%s: %ld шт.\n", names[i], counts[i]);
    }
    fclose(out);
    printf("[+] Результат записан в '%s'.\n", OUTPUT_FILE);
    return 0;
}

static int rand_in(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

char **create_map(int height, int width) {
    char **map = malloc((size_t)height * sizeof(char *));
    if (!map) return NULL;
    for (int i = 0; i < height; i++) {
        map[i] = malloc((size_t)width * sizeof(char));
        if (!map[i]) {
            for (int j = 0; j < i; j++) free(map[j]);
            free(map);
            return NULL;
        }
    }
    return map;
}

void free_map(char **map, int height) {
    if (!map) return;
    for (int i = 0; i < height; i++) free(map[i]);
    free(map);
}

bool **create_traps(int height, int width) {
    bool **t = malloc((size_t)height * sizeof(bool *));
    if (!t) return NULL;
    for (int i = 0; i < height; i++) {
        t[i] = calloc((size_t)width, sizeof(bool));
        if (!t[i]) {
            for (int j = 0; j < i; j++) free(t[j]);
            free(t);
            return NULL;
        }
    }
    return t;
}

void free_traps(bool **t, int height) {
    if (!t) return;
    for (int i = 0; i < height; i++) free(t[i]);
    free(t);
}

char **generate_map(int *out_h, int *out_w) {
    int h = rand_in(MAP_MIN_SIZE, MAP_MAX_SIZE);
    int w = rand_in(MAP_MIN_SIZE, MAP_MAX_SIZE);

    char **map = create_map(h, w);
    if (!map) return NULL;

    int chance = rand_in(OBSTACLE_CHANCE_MIN, OBSTACLE_CHANCE_MAX);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int border = (y == 0 || y == h - 1 || x == 0 || x == w - 1);
            if (border) {
                map[y][x] = CH_WALL;
            } else if (rand_in(1, 100) <= chance) {
                map[y][x] = (rand() % 2) ? CH_TREE : CH_ROCK;
            } else {
                map[y][x] = CH_FLOOR;
            }
        }
    }

    *out_h = h;
    *out_w = w;
    return map;
}

void place_traps(char **map, bool **traps, int h, int w) {
    int floors = 0;
    for (int y = 1; y < h - 1; y++)
        for (int x = 1; x < w - 1; x++)
            if (map[y][x] == CH_FLOOR) floors++;

    if (floors == 0) return;

    int pct   = rand_in(TRAP_CHANCE_MIN, TRAP_CHANCE_MAX);
    int count = floors * pct / 100;
    if (count == 0) count = 1;

    int placed = 0, guard = 0;
    int max_guard = floors * 10 + 50;
    while (placed < count && guard++ < max_guard) {
        int x = rand_in(1, w - 2);
        int y = rand_in(1, h - 2);
        if (map[y][x] != CH_FLOOR) continue;
        if (traps[y][x]) continue;
        traps[y][x] = true;
        placed++;
    }
}

int find_free_cell(char **map, int h, int w, int *hx, int *hy) {
    for (int attempt = 0; attempt < 200; attempt++) {
        int x = rand_in(1, w - 2);
        int y = rand_in(1, h - 2);
        if (map[y][x] == CH_FLOOR) { *hx = x; *hy = y; return 1; }
    }
    for (int y = 1; y < h - 1; y++)
        for (int x = 1; x < w - 1; x++)
            if (map[y][x] == CH_FLOOR) { *hx = x; *hy = y; return 1; }
    return 0;
}

void draw_map(char **map, int h, int w, int hx, int hy) {
    printf("\n");
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            putchar((x == hx && y == hy) ? CH_HERO : map[y][x]);
        }
        putchar('\n');
    }
    printf("Герой на (%d, %d). Размер %dx%d.\n", hx, hy, w, h);
}

static int cell_passable(char **map, int h, int w, int x, int y) {
    if (x < 0 || x >= w || y < 0 || y >= h) return 0;
    return map[y][x] == CH_FLOOR || map[y][x] == CH_TRAP_SHOWN;
}

int try_move(char **map, bool **traps, int h, int w,
             int *hx, int *hy, char dir)
{
    int nx = *hx, ny = *hy;
    switch (dir) {
        case 'w': case 'W': ny--; break;
        case 's': case 'S': ny++; break;
        case 'a': case 'A': nx--; break;
        case 'd': case 'D': nx++; break;
        default: return MOVE_UNKNOWN;
    }

    if (!cell_passable(map, h, w, nx, ny)) return MOVE_BLOCKED;

    if (traps[ny][nx]) {
        if (map[ny][nx] == CH_FLOOR) {
            map[ny][nx] = CH_TRAP_SHOWN;
            printf("Осторожно, ипучий пол!\n");
        } else {
            if (rand_in(1, 100) <= TRAP_COLLAPSE_PCT) {
                printf("Пол с треском ломается! Падаешь на этаж ниже...(Все заново, наверное)\n");
                return MOVE_COLLAPSE;
            }
            printf("Пол скрипит!\n");
        }
    }

    *hx = nx;
    *hy = ny;
    return MOVE_OK;
}

typedef struct {
    char **map;
    bool **traps;
    int    h, w;
    int    hx, hy;
} Floor;

static int floor_init(Floor *f) {
    f->map = generate_map(&f->h, &f->w);
    if (!f->map) return 0;

    f->traps = create_traps(f->h, f->w);
    if (!f->traps) {
        free_map(f->map, f->h);
        f->map = NULL;
        return 0;
    }

    place_traps(f->map, f->traps, f->h, f->w);

    if (!find_free_cell(f->map, f->h, f->w, &f->hx, &f->hy)) {
        free_traps(f->traps, f->h);
        free_map(f->map, f->h);
        f->map = NULL;
        f->traps = NULL;
        return 0;
    }
    return 1;
}

static void floor_destroy(Floor *f) {
    free_traps(f->traps, f->h);
    free_map(f->map, f->h);
    f->traps = NULL;
    f->map = NULL;
}

void adventure(void) {
    Floor f = {0};
    if (!floor_init(&f)) {
        printf("[!] Не хватило памяти на этаж.\n");
        return;
    }

    int depth = 1;
    printf("\n--- Приключение (этаж %d) ---\n", depth);
    printf("Управление: W/A/S/D — шаг, Q — вернуться домой.\n");

    for (;;) {
        draw_map(f.map, f.h, f.w, f.hx, f.hy);
        printf("Этаж: %d\n", depth);

        char dir = ask_dir();
        if (dir == 'q' || dir == 'Q') {
            printf("Возвращаемся домой.\n");
            break;
        }

        int r = try_move(f.map, f.traps, f.h, f.w, &f.hx, &f.hy, dir);

        if (r == MOVE_UNKNOWN) {
            printf("Неизвестное направление. Используй W/A/S/D или Q.\n");
            continue;
        }
        if (r == MOVE_BLOCKED) {
            printf("Туда нельзя: стена или препятствие.\n");
            continue;
        }
        if (r == MOVE_COLLAPSE) {
            floor_destroy(&f);
            depth++;
            if (!floor_init(&f)) {
                printf("[!] Не хватило памяти на новый этаж. Возврат домой.\n");
                return;
            }
            printf("\n--- Этаж %d ---\n", depth);
        }
    }

    floor_destroy(&f);
    printf("[+] Память этажа освобождена.\n");
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
    printf("10 — Отправиться в приключение\n");
    printf("11 — Расшифровать поставки\n");
    printf("12 — Сменить имя\n");
    printf("===========================\n");
}

int main(void) {
    srand((unsigned)time(NULL));

    char farmer_name[MAX_NAME_LEN] = "Безымянный";
    int  day  = 1;
    int  hour = 8;

    int inv[INV_SIZE] = { 0, 2, 0, 4, 0, 5, 0, 6, 0, 0 };

    printf("=== Приветствую в игре 'Весёлый фермер' ===\n");
    load_catalog();
    ask_farmer_name(farmer_name, sizeof(farmer_name));
    printf("Привет, %s! День %d, %02d:00.\n", farmer_name, day, hour);

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
                adventure();
                break;
            case 11:
                process_supplies();
                break;
            case 12:
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
