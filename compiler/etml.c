#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// トークンの種類
typedef enum
{
    TK_COMMAND, // コマンド
    TK_ARG,     // コマンドの引数
    TK_TEXT,    // 文章
    TK_EOF,     // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    char *str;      // トークン文字列
    int len;        // トークン文字列の長さ
};

// 現在着目しているトークン
Token *token;

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    printf("New token %p has been created.\n", cur->next);
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    bool is_command = false;
    bool is_first_command = false;
    int etml_command_length = 0;
    bool is_argument = false;
    bool is_first_argument = false;
    int etml_argument_length = 0;
    bool is_text = false;
    bool is_first_text = true;
    int etml_text_length = 0;

    while (*p)
    {
        printf("tokenizing \"%c\", memory address: %p.\n", *p, p);
        // 空白文字をスキップ
        if (!is_argument && isspace(*p))
        {
            printf("Skipping tokenize \"%c\" because it's space, memory address: %p.\n", *p, p);
            p++;
            continue;
        }

        if (*p == '\\' && !is_argument)
        {
            printf("\"%c\" is a backslash so it will not be tokenized. Memory address: %p.\n", *p, p);
            is_command = true;
            is_first_command = true;
            if (is_text)
            {
                is_text = false;
                is_first_text = true;
                printf("Token length: %d\n", etml_text_length);
                cur->len = etml_text_length;
                etml_text_length = 0;
            }
            p++;
            continue;
        }

        if (*p == '{' && is_command)
        {
            printf("\"%c\" is a curly brackets so it will not be tokenized. Memory address: %p.\n", *p, p);
            is_command = false;
            is_argument = true;
            is_first_argument = true;
            printf("Token length: %d.\n", etml_command_length);
            cur->len = etml_command_length;
            etml_command_length = 0;
            p++;
            continue;
        }

        if (*p == '}' && is_argument)
        {
            printf("\"%c\" is a curly brackets so it will not be tokenized. Memory address: %p.\n", *p, p);
            is_argument = false;
            printf("Token length: %d.\n", etml_argument_length);
            cur->len = etml_argument_length;
            etml_argument_length = 0;
            p++;
            continue;
        }

        if (is_command && is_first_command)
        {
            printf("\"%c\" is a type of command. Memory address: %p.\n", *p, p);
            cur = new_token(TK_COMMAND, cur, p++);
            is_first_command = false;
            etml_command_length = 1;
            continue;
        }

        if (is_command)
        {
            printf("\"%c\" is a type of command. Memory address: %p.\n", *p, p);
            etml_command_length++;
            p++;
            continue;
        }

        if (is_argument && is_first_argument)
        {
            printf("\"%c\" is a type of argument. Memory address: %p.\n", *p, p);
            cur = new_token(TK_ARG, cur, p++);
            is_first_argument = false;
            etml_argument_length = 1;
            continue;
        }

        if (is_argument)
        {
            printf("\"%c\" is a type of argument. Memory address: %p.\n", *p, p);
            etml_argument_length++;
            p++;
            continue;
        }

        if (is_first_text)
        {
            printf("\"%c\" is a type of text. Memory address: %p.\n", *p, p);
            cur = new_token(TK_TEXT, cur, p++);
            is_text = true;
            is_first_text = false;
            etml_text_length = 1;
            continue;
        }

        printf("\"%c\" is a type of text. Memory address: %p.\n", *p, p);
        etml_text_length++;
        p++;
    }

    if (is_text)
    {
        is_text = false;
        is_first_text = true;
        printf("Token length: %d.\n", etml_text_length);
        cur->len = etml_text_length;
        etml_text_length = 0;
    }

    printf("Memory address %p is the end of user-input argument.\n", p);
    new_token(TK_EOF, cur, p);
    return head.next;
}

char *read_token_str(char *str, size_t len)
{
    printf("Reading address %p with length %zu.\n", str, len);
    char *token_str = calloc(len + 1, sizeof(char));
    if (token_str == NULL)
    {
        fprintf(stderr, "Failed to allocate the memory. Insufficient memory.\n");
        exit(1);
    }
    memcpy(token_str, str, len);
    token_str[len] = '\0';
    printf("Readed token string: \"%s\".\n", token_str);

    return token_str;
}

char *append_string(char *dest, char *src)
{
    size_t dest_len = (dest != NULL) ? strlen(dest) : 0;
    size_t src_len = strlen(src);
    size_t new_size = dest_len + src_len + 1;
    printf("Previous variable length: %zu.\n", dest_len);
    printf("String length: %zu.\n", src_len);
    printf("New variable length: %zu.\n", new_size);

    // 領域を拡大（dest が NULL のときは malloc と同等の動作）
    char *temp = realloc(dest, new_size);
    if (temp == NULL)
    {
        fprintf(stderr, "Failed to allocate the memory. Insufficient memory.\n");
        exit(1);
    }
    printf("Temp veriable address: %p.\n", &temp);
    dest = temp;

    // 初回の場合は空文字で初期化しておく
    if (dest_len == 0)
    {
        dest[0] = '\0';
    }

    // 末尾に追加
    memcpy(dest + dest_len, src, src_len + 1);
    printf("New memory address: %p.\n", &dest);
    return dest;
}

void reset_var(char **str_ptr)
{
    if (str_ptr == NULL || *str_ptr == NULL)
    {
        fprintf(stderr, "Invalid pointer.\n"); // 無効なポインタ
        exit(1);
    }

    // 1バイト ('\0' 用) にサイズを縮小
    char *temp = realloc(*str_ptr, 1);
    if (temp == NULL)
    {
        fprintf(stderr, "Failed to allocate the memory. Insufficient memory.\n");
        exit(1);
    }

    // 新しいアドレス（移動している可能性を考慮）を反映
    *str_ptr = temp;
    printf("New memory address: %p.\n", &*str_ptr);

    // 先頭を終端文字にして空文字列にする
    (*str_ptr)[0] = '\0';

    return;
}

char *html_lang = NULL;
char *html_title = NULL;
char *html_description = NULL;
char *html_viewport = NULL;

char *etml_command = NULL;
char *etml_arg = NULL;

bool is_str_same(char *str1, char *str2)
{
    if (strcmp(str1, str2) == 0)
    {
        return true;
    }

    return false;
}

void command_lookup()
{
    if (is_str_same(etml_command, "lang"))
    {
        reset_var(&html_lang);
        html_lang = append_string(html_lang, etml_arg);
    }

    if (is_str_same(etml_command, "title"))
    {
        reset_var(&html_title);
        html_title = append_string(html_title, etml_arg);
    }

    if (is_str_same(etml_command, "description"))
    {
        reset_var(&html_description);
        html_description = append_string(html_description, etml_arg);
    }

    if (is_str_same(etml_command, "viewport"))
    {
        reset_var(&html_viewport);
        html_viewport = append_string(html_viewport, etml_arg);
    }

    reset_var(&etml_command);
    reset_var(&etml_arg);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "The number of argument is incorrect.\n");
        return 1;
    }

    // トークナイズする
    token = tokenize(argv[1]);

    html_lang = append_string(html_lang, "en");
    html_title = append_string(html_title, "");
    html_description = append_string(html_description, "");
    html_viewport = append_string(html_viewport, "width=device-width,initial-scale=1");

    while (!at_eof())
    {
        if (token->kind == TK_COMMAND)
        {
            printf("Reading the token type of TK_COMMAND. Token length: %d.\n", token->len);
            printf("Adding string \"%s\".\n", read_token_str(token->str, token->len));
            etml_command = append_string(etml_command, read_token_str(token->str, token->len));
            token = token->next;

            continue;
        }

        if (token->kind == TK_ARG)
        {
            printf("Reading the token type of TK_ARG. Token length: %d.\n", token->len);
            printf("Adding string \"%s\".\n", read_token_str(token->str, token->len));
            etml_arg = append_string(etml_arg, read_token_str(token->str, token->len));
            command_lookup();
            token = token->next;

            continue;
        }

        token = token->next;
    }

    printf("<!DOCTYPE html>\n");
    printf("<html lang=\"%s\">\n", html_lang);
    printf("\n");
    printf("<head>\n");
    printf("    <meta charset=\"utf-8\">\n");
    printf("    <title>%s</title>\n", html_title);
    printf("    <meta name=\"description\" content=\"%s\">\n", html_description);
    printf("    <meta name=\"viewport\" content=\"%s\">\n", html_viewport);
    printf("</head>\n");
    printf("\n");
    printf("<body>\n");
    printf("    <header>\n");
    printf("\n");
    printf("    </header>\n");
    printf("    <main>\n");
    printf("\n");
    printf("    </main>\n");
    printf("    <footer>\n");
    printf("\n");
    printf("    </footer>\n");
    printf("</body>\n");
    printf("\n");
    printf("</html>\n");

    return 0;
}