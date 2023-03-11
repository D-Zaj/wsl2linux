#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE (8*1024)
#define ESCAPE_CHAR '\''

static char tmp_buf[BUF_SIZE] = {0};
int tmp_size = 0;
char* end = tmp_buf;

void tmp_append_cstr(const char* str)
{
    int len = strlen(str);
    if (tmp_size + len > BUF_SIZE) {
        fprintf(stderr, "ERROR: maximum tmp buffer capacity was reached!\n");
        exit(1);
    }
    for (int i = 0; i < len; i++) {
        *end = str[i];
        end++;
    }
    tmp_size += len;
}

void tmp_append_sized(const char* str, size_t n)
{
    if (tmp_size + n > BUF_SIZE) {
        fprintf(stderr, "ERROR: maximum tmp buffer capacity was reached!\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        *end = str[i];
        end++;
    }
    tmp_size += n;
}

void tmp_append_chr(char chr)
{
    tmp_append_sized(&chr, 1);
}

char* tmp_end()
{
    return end;
}

void tmp_clean()
{
    tmp_size = 0;
    end = tmp_buf;
}

static char stdin_buf[BUF_SIZE];
char* get_filename_from_stdin()
{
    int i = 0;
    char* cursor = stdin_buf;
    char next_char;
    while ((next_char = getchar()) != -1 && next_char != '\n') {
        *cursor++ = next_char;
    }
    *cursor = '\0';
    return stdin_buf;
}

int main(int argc, char** argv)
{
    unsigned int piped = !isatty(fileno(stdin));
    char* filepath;

    /******** Figure out filepath from input ************/
    if (argc < 2) {
        if (piped) {
            filepath = get_filename_from_stdin();
        } else {
            printf("Usage: %s <path-to-convert>\n", argv[0]);
            exit(1);
        }
    } else {
        // if something is provided, assume that's the desired filepath
        filepath = argv[1];
    }

    char* result = tmp_end();

    //TODO: this is a placeholder, figure out a way of actually getting the current WSL root path
    tmp_append_cstr("\\\\wsl$\\Ubuntu");

    // Add an extra sep char if not present in start of provided path
    if (*filepath != '/') tmp_append_chr('\\');

    unsigned int contains_space;
    char* tok = strtok(filepath, "/");

    while (tok != NULL) {
        // wrap token in "s if it contains a space
        contains_space = strchr(tok, ' ') != NULL;
        if (contains_space) tmp_append_chr(ESCAPE_CHAR);
        tmp_append_cstr(tok);
        if (contains_space) tmp_append_chr(ESCAPE_CHAR);
        tmp_append_chr('\\');
        tok = strtok(NULL, "/");
    }
    tmp_append_chr('\0');
    printf("Result: %s\n", result);

    return 0;
}
