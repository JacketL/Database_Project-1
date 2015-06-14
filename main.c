#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>

#define ROW_BUFFER_LEN 8192
#define NOT_FOUND -1
#define IO_WRITE_END true
#define PAGE_SIZE 8192
#define READ_END true

void insert_file_by_filename();
void check_catalog();
void find_the_data_whose_keyA_euqal_valueB();
void quit();
void invalid_operation();
int get_attributes_num_and_preserve_catalog(char *row_buffer);
bool is_digit(std::string s);
void write_catalog(FILE *created_catalog);
void throw_int_to_char_arr(char *data, int num, int data_count);
int to_int(char *value);
void write_data(FILE *created_data, int *int_before_data, char *data, int data_count);
void check_IO_write(FILE *created_data, bool io_end);
int get_aid(std::string key);
bool read_rows(FILE *created_data);
void output(int *int_before_data, FILE *created_data);
void check_IO_read(FILE *created_data);

std::string catalog_key_name[PAGE_SIZE];
std::string catalog_key_type[PAGE_SIZE];
int catalog_count[PAGE_SIZE];
int _id_max = 0;
char write_buffer[2 * PAGE_SIZE];
int current_offset = 0;
int write_count = 0;
char read_buffer[2 * PAGE_SIZE];
int read_count;
int aid;
int read_offset;
int read_end;
std::string value;

int main(void) {
    std::string operation;
    while (std::cin >> operation) {
        if (operation == "insert") insert_file_by_filename();
        else if (operation == "check") check_catalog();
        else if (operation == "find") find_the_data_whose_keyA_euqal_valueB();
        else if (operation == "quit") quit();
        else invalid_operation();
    }
    return 0;
}

void insert_file_by_filename() {
    char file_name[128];
    scanf("%s", file_name);
    FILE *nobench_data_json = fopen(file_name, "r");
    FILE *created_data = fopen("created_data.data", "wb+");
    FILE *created_catalog = fopen("created_catalog.data", "w+");
    static char BUFFER[BUFSIZ];
    setbuf(created_catalog, BUFFER);
    char row_buffer[ROW_BUFFER_LEN];
    _id_max = 0;
    while (fgets(row_buffer, ROW_BUFFER_LEN, nobench_data_json)) {
        int attributes_num = get_attributes_num_and_preserve_catalog(row_buffer);
        bool key_reading = false, value_reading = false;
        char key_buffer[ROW_BUFFER_LEN], value_buffer[ROW_BUFFER_LEN];
        int key_buffer_count = 0, value_buffer_count = 0, key_id;

        int int_before_data[ROW_BUFFER_LEN], aid_count = 1, data_count = 0;
        int off_count = attributes_num + 1;
        char data[ROW_BUFFER_LEN];
        int_before_data[0] = attributes_num;
        bool is_int;

        for (size_t i = 0; i < strlen(row_buffer); i++) {
            if (row_buffer[i] == '\"' && !value_reading) {
                if (!key_reading && !value_reading) {
                    key_reading = true;
                } else if (key_reading) {
                    key_reading = false;
                    value_reading = true;
                    key_buffer_count = 0;
                    i += 2;

                    /* get key */
                    std::string key = key_buffer;
                    key_id = NOT_FOUND;
                    for (int j = 1; j <= _id_max; j++) {
                        if (key == catalog_key_name[j]) {
                            int_before_data[aid_count++] = j;
                            is_int = catalog_key_type[j] == "int" ? true : false;
                            break;
                        }
                    }

                    if (row_buffer[i + 1] == '[') {
                        while (row_buffer[i + 1] != ']') {
                            i++;
                            value_buffer[value_buffer_count++] = row_buffer[i];
                        }
                        value_buffer[value_buffer_count++] = ']';
                        i++;
                    } else if (row_buffer[i + 1] == '{') {
                        while (row_buffer[i + 1] != '}') {
                            i++;
                            value_buffer[value_buffer_count++] = row_buffer[i];
                        }
                        value_buffer[value_buffer_count++] = '}';
                        i++;
                    }
                    memset(key_buffer, 0, sizeof(key_buffer));
                }
                continue;
            } else if (row_buffer[i] == ',' || row_buffer[i] == '}') {
                if (i == (strlen(row_buffer) - 2) && row_buffer[i] == ',')
                    continue;
                value_reading = false;
                value_buffer_count = 0;

                /* get value */
                if (is_int) {
                    throw_int_to_char_arr(data, to_int(value_buffer), data_count);
                    data_count += 4;
                    int_before_data[off_count++] = 4;
                } else {
                    for (unsigned long j = 0; j < strlen(value_buffer); j++) {
                        data[data_count++] = value_buffer[j];
                    }
                    int_before_data[off_count++] = strlen(value_buffer);
                }

                std::string value = value_buffer;
                memset(value_buffer, 0, sizeof(value_buffer));
                continue;
            }
            if (key_reading) {
                key_buffer[key_buffer_count++] = row_buffer[i];
            } else if (value_reading) {
                value_buffer[value_buffer_count++] = row_buffer[i];
            }
        }
        write_data(created_data, int_before_data, data, data_count);
    }
    check_IO_write(created_data, IO_WRITE_END);
    write_catalog(created_catalog);
    fclose(nobench_data_json);
    fclose(created_data);
    fclose(created_catalog);
    printf("DONE.\n");
}

void check_catalog() {
    std::string catalog;
    std::cin >> catalog;
    FILE *created_catalog = fopen("created_catalog.data", "r");
    char c;
    while ((c = fgetc(created_catalog)) != EOF)
        printf("%c", c);
    getchar();
    fclose(created_catalog);
    return;
}

void find_the_data_whose_keyA_euqal_valueB() {
    FILE *created_data = fopen("created_data.data", "rb");
    std::string key, equal;
    std::cin >> key >> equal >> value;
    if ((aid = get_aid(key)) == NOT_FOUND)
        printf("NOT FOUND.\n"), exit(1);
    else {
        fread(read_buffer, PAGE_SIZE, 1, created_data);
        fread(&read_buffer[PAGE_SIZE], PAGE_SIZE, 1, created_data);
        read_offset = 16384;
        read_count = 0;
        read_end = -1;
        while (read_rows(created_data) != READ_END);
    }
    fclose(created_data);
    return;
}

void quit() {
    exit(0);
}

void invalid_operation() {
    printf("INVALID OPERATION.\n");
    exit(1);
}

int get_attributes_num_and_preserve_catalog(char *row_buffer) {
    bool key_reading = false, value_reading = false;
    char key_buffer[ROW_BUFFER_LEN], value_buffer[ROW_BUFFER_LEN];
    int key_buffer_count = 0, value_buffer_count = 0, attributes_num = 0, key_id;
    for (size_t i = 0; i < strlen(row_buffer); i++) {
        if (row_buffer[i] == '\"' && !value_reading) {
            if (!key_reading && !value_reading) {
                key_reading = true;
            } else if (key_reading) {
                key_reading = false;
                value_reading = true;
                key_buffer_count = 0;
                i += 2;
                attributes_num++;
                /* get key */
                std::string key = key_buffer;
                key_id = NOT_FOUND;
                for (int j = 1; j <= _id_max; j++) {
                    if (key == catalog_key_name[j]) {
                        key_id = j;
                        catalog_count[j]++;
                    }
                }
                if (key_id == NOT_FOUND) {
                    _id_max++;
                    key_id = _id_max;
                    catalog_key_name[_id_max] = key;
                    catalog_count[_id_max] = 1;
                }
                if (row_buffer[i + 1] == '[') {
                    while (row_buffer[i + 1] != ']') {
                        i++;
                        value_buffer[value_buffer_count++] = row_buffer[i];
                    }
                    value_buffer[value_buffer_count++] = ']';
                    i++;
                } else if (row_buffer[i + 1] == '{') {
                    while (row_buffer[i + 1] != '}') {
                        i++;
                        value_buffer[value_buffer_count++] = row_buffer[i];
                    }
                    value_buffer[value_buffer_count++] = '}';
                    i++;
                }
                memset(key_buffer, 0, sizeof(key_buffer));
            }
            continue;
        } else if (row_buffer[i] == ',' || row_buffer[i] == '}') {
            if (i == (strlen(row_buffer) - 2) && row_buffer[i] == ',')
                continue;
            value_reading = false;
            value_buffer_count = 0;
            /* get value */
            std::string value = value_buffer;
            if (is_digit(value)) {
                catalog_key_type[key_id] = "int";
            } else if (value[0] == '[') {
                catalog_key_type[key_id] = "array";
            } else if (value[0] == '{') {
                catalog_key_type[key_id] = "json";
            } else if (value == "true" || value == "false") {
                catalog_key_type[key_id] = "boolean";
            } else if (value[0] == '\"') {
                catalog_key_type[key_id] = "text";
            }
            memset(value_buffer, 0, sizeof(value_buffer));
            continue;
        }
        if (key_reading) {
            key_buffer[key_buffer_count++] = row_buffer[i];
        } else if (value_reading) {
            value_buffer[value_buffer_count++] = row_buffer[i];
        }
    }
    return attributes_num;
}

bool is_digit(std::string s) {
    bool result = true;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] < '0' || s[i] > '9')
            result = false;
    }
    return result;
}

void write_catalog(FILE *created_catalog) {
    for (int i = 1; i <= _id_max; i++) {
        fprintf(created_catalog, "%d %s %s %d\n", i, catalog_key_name[i].c_str(), catalog_key_type[i].c_str(), catalog_count[i]);
    }
}

void throw_int_to_char_arr(char *data, int num, int data_count) {
    memcpy(&data[data_count], &num, 4);
}

int to_int(char *value) {
    int base = 1, result = 0;
    for (int i = strlen(value) - 1; i >= 0; i--) {
        result += (value[i] - '0') * base;
        base *= 10;
    }
    return result;
}

void write_data(FILE *created_data, int *int_before_data, char *data, int data_count) {
    int attributes_num = int_before_data[0];
    int data_offset = current_offset + write_count + 4 * (2 * attributes_num + 2);
    int int_buffer[ROW_BUFFER_LEN], point = data_offset;
    memcpy(int_buffer, int_before_data, 4 * (2 * attributes_num + 2));
    int_before_data[attributes_num * 2 + 1] = int_before_data[attributes_num * 2];
    for (int i = attributes_num + 2; i < 2 * attributes_num + 1; i++) {
        point += int_buffer[i - 1];
        int_before_data[i] = point;
    }
    int_before_data[attributes_num + 1] = data_offset;
    memcpy(&write_buffer[write_count], int_before_data, (2 * attributes_num + 2) * 4);
    write_count += (2 * attributes_num + 2) * 4;
    memcpy(&write_buffer[write_count], data, data_count);
    write_count += data_count;
    check_IO_write(created_data, !IO_WRITE_END);
}

void check_IO_write(FILE *created_data, bool io_end) {
    if (io_end) {
        if (fwrite(write_buffer, PAGE_SIZE, 1, created_data) == 1)
            printf("WRITE END.\n");
    } else {
        if (write_count < PAGE_SIZE) return;
        else {
            if (fwrite(write_buffer, PAGE_SIZE, 1, created_data) == 1) {
                current_offset = ftell(created_data);
                memcpy(&write_buffer[0], &write_buffer[PAGE_SIZE], PAGE_SIZE);
                write_count -= PAGE_SIZE;
            } else {
                printf("IO_write_error\n");
                exit(1);
            }
        }
    }
}

int get_aid(std::string key) {
    int aid = NOT_FOUND;
    for (int i = 1; i <= _id_max; i++) {
        if (key == catalog_key_name[i]) {
            aid = i;
            break;
        }
    }
    return aid;
}

bool read_rows(FILE *created_data) {
    int int_before_data[ROW_BUFFER_LEN];
    char data[ROW_BUFFER_LEN];
    memcpy(&int_before_data[0], &read_buffer[read_count], 4);
    read_count += 4;
    int attributes_num = int_before_data[0];
    if (attributes_num == 0) read_end++;
    memcpy(&int_before_data[1], &read_buffer[read_count], 4 * (2 * attributes_num + 1));
    read_count += 4 * (2 * attributes_num + 1);
    for (int i = 1; i <= attributes_num; i++) {
        if (int_before_data[i] == aid) {
            int offset = int_before_data[i + attributes_num];
            int length = i < attributes_num ? (int_before_data[i + attributes_num + 1] - int_before_data[i + attributes_num]) : int_before_data[i + attributes_num + 1];
            fseek(created_data, offset, SEEK_SET);
            fread(data, length, 1, created_data);
            if (catalog_key_type[aid] == "int") {
                int result;
                memcpy(&result, data, 4);
                int v = atoi(value.c_str());
                if (result == v) output(int_before_data, created_data);
            } else {
                data[length] = '\0';
                std::string v = data;
                if (v == value) output(int_before_data, created_data);
            }
        }
    }
    read_count += int_before_data[2 * attributes_num] - int_before_data[attributes_num + 1] + int_before_data[2 * attributes_num + 1];
    check_IO_read(created_data);
    if (read_end)
        return READ_END;
    else
        return !READ_END;
}

void output(int *int_before_data, FILE *created_data) {
    int attributes_num = int_before_data[0];
    printf("{");
    for (int i = 1; i <= attributes_num; i++) {
        char data[ROW_BUFFER_LEN];
        std::string key_name = catalog_key_name[int_before_data[i]];
        int offset = int_before_data[i + attributes_num];
        int length = i < attributes_num ? (int_before_data[i + attributes_num + 1] - int_before_data[i + attributes_num]) : int_before_data[i + attributes_num + 1];
        fseek(created_data, offset, SEEK_SET);
        fread(data, length, 1, created_data);
        if (catalog_key_type[int_before_data[i]] == "int") {
            int v;
            memcpy(&v, data, 4);
            std::cout << "\"" << key_name << "\"" << ": " << v;
        } else {
            data[length] = '\0';
            std::string v = data;
            std::cout << "\"" << key_name << "\"" << ": " << v;
        }
        if (i == attributes_num) {
            printf("}\n");
        } else {
            printf(", ");
        }
    }
}

void check_IO_read(FILE *created_data) {
    if (read_count < PAGE_SIZE) return;
    else {
        memcpy(&read_buffer[0], &read_buffer[PAGE_SIZE], PAGE_SIZE);
        fseek(created_data, read_offset, SEEK_SET);
        fread(&read_buffer[PAGE_SIZE], PAGE_SIZE, 1, created_data);
        read_count -= PAGE_SIZE;
        read_offset += PAGE_SIZE;
    }
}

