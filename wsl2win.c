#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE (8*1024)
#define WSL_PREFIX "\\\\wsl$\\"
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

static char* tokens[BUF_SIZE] = {0};
int token_count = 0;

void append_token(char* tok)
{
    tokens[token_count++] = tok;
}

void pop_token()
{
    token_count--;
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

char* abspath(char* filepath)
{
    char cwd_buf[BUF_SIZE];
    char *cwd_path = getcwd(cwd_buf, BUF_SIZE);
    if (cwd_path == NULL) {
        fprintf(stderr, "ERROR: pathname of cwd exceeds provided buffer size.\n");
        exit(1);
    }
    char* fullpath;

    // Check if provided path is already absolute
    if (filepath[0] != '/') {
        fullpath = tmp_end();
        tmp_append_cstr(cwd_path); tmp_append_chr('/');
        tmp_append_cstr(filepath); tmp_append_chr('\0');
        printf("Full path: %s\n", fullpath);
    } else {
        fullpath = filepath;
    }

    // normalize path
    char* tok = strtok(fullpath, "/");
    size_t tok_len = strlen(tok);
    while (tok != NULL) {
        if (strncmp(tok, "..", 2) == 0 && tok_len == 2 && token_count > 0) {
            pop_token();
        } else if (*tok == '.' && tok_len == 1) {
            // Ignore single dots
        } else {
            append_token(tok);
        }
        tok = strtok(NULL, "/");
    }

    char* path = tmp_end();
    // collect normalized tokens into a path string
    for (int i = 0; i < token_count; i++) {
        tmp_append_chr('\\');
        tmp_append_cstr(tokens[i]);
    }
    tmp_append_chr('\0');

    size_t path_len = tmp_end() - path;
    char* result = malloc(path_len);
    return memcpy(result, path, path_len);
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

    char* fullpath = abspath(filepath);

    char* result = tmp_end();

    char* wsl_distro_name = getenv("WSL_DISTRO_NAME");
    tmp_append_cstr(WSL_PREFIX);
    tmp_append_cstr(wsl_distro_name);

    unsigned int contains_space = strchr(fullpath, ' ') != NULL;
    if (contains_space) tmp_append_chr(ESCAPE_CHAR);
    tmp_append_cstr(fullpath);
    if (contains_space) tmp_append_chr(ESCAPE_CHAR);
    tmp_append_chr('\0');
    printf("Result: %s\n", result);
    free(fullpath);

    return 0;
}
