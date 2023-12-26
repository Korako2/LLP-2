#include "../include/parser.h"

void print_error() {
    printf("PARSE ERROR\n");
    exit(EXIT_FAILURE);
}

void check_path(char *req, int *path_length) {
    int bracket = 0;
    int bracket2 = 0;

    for (int i = 0; i < *path_length; i++) {
        if (req[i] == '{') bracket++;
        if (req[i] == '}') bracket--;
        if (req[i] == '(') bracket2++;
        if (req[i] == ')') bracket2--;
    }

    if (bracket != 0 || bracket2 != 0) {
        print_error();
    }
}

void remove_char(int *size, char **request_path) {
    (*size)--;
    (*request_path)++;
}

char *read_word(char **req, int *path_length) {
    char *word = calloc(MAX_STRING_SIZE, sizeof(char));

    int i;
    for (i = 0; i < *path_length; i++) {
        //if ((*req)[i] == '{' || (*req)[i] == '}') remove_char(path_length, req);

        int k = 0;
        while ((*req)[k] == ' ' || (*req)[i] == '{' || (*req)[i] == '}') {
            remove_char(path_length, req);
        }

        if ((*req)[i] == ' ' || (*req)[i] == ',' || (*req)[i] == '(' || (*req)[i] == ')' || (*req)[i] == '}') {
            if ((*req)[i] == ',') i--;
            break;
        }

        word[i] = (*req)[i];
    }
    i++;

    for (int j = 0; j < i; j++) {
        remove_char(path_length, req);
    }

    return word;
}

int read_attributes(char **req, int *path_length, struct attribute *attribute) {
    int flag = 0;

    if ((*req)[0] != '|' && (*req)[0] != '&') {

        char *left = read_word(req, path_length);
        char *condition = read_word(req, path_length);
        char *right = read_word(req, path_length);

        attribute->left = left;
        attribute->condition = condition;
        attribute->right = right;

        if ((*req)[0] == '{') {
            flag++;
            struct attribute *new_attribute = malloc(sizeof(struct attribute));
            flag += read_attributes(req, path_length, new_attribute);
            attribute->next_attribute = new_attribute;
        }
    }

    if ((*req)[0] == ',') {
        remove_char(path_length, req);
        remove_char(path_length, req);

        struct attribute *new_attribute = malloc(sizeof(struct attribute));
        flag += read_attributes(req, path_length, new_attribute);
        attribute->next_attribute = new_attribute;

    } else if ((*req)[0] == '|' || (*req)[0] == '&') {
        char *combined_condition = read_word(req, path_length);

        attribute->combined_condition = combined_condition;

        struct attribute *new_attribute = malloc(sizeof(struct attribute));
        flag += read_attributes(req, path_length, new_attribute);
        attribute->next_attribute = new_attribute;
    }

    remove_char(path_length, req);

    return flag;
}

enum parser_status parse_request(char *req, struct request *request) {


    int path_length = strlen(req);

    check_path(req, &path_length);

    char *operation = read_word(&req, &path_length);
    if (!(
            strcmp("add", operation) == 0 ||
            strcmp("find", operation) == 0 ||
            strcmp("delete", operation) == 0 ||
            strcmp("update", operation) == 0
    ))
        print_error();

    request->operation = operation;

    struct attribute *attribute = malloc(sizeof(struct attribute));
    request->attributes = attribute;
    int flag = read_attributes(&req, &path_length, attribute);

    struct attribute *attr = request->attributes;

    if (attr->next_attribute) {
        if (flag > 0) {
            request->parent_id = attr->right;
            request->attributes = attr->next_attribute;
        }
    }

    return PARSE_OK;
}