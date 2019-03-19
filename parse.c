#include "9cc.h"

static Token *add_token(Vector *tokens, int ty, char *input){
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    vec_push(tokens,token);
    return token;
}

static char *add_token_ident(Vector *tokens, char *p){
    char *name = malloc(sizeof(char) * 256);
    int i = 0;
    while ( isalnum(*p) || *p == '_')
        *(name+i++) = *p++;
    *(name+i) = '\0';
    add_token(tokens, TK_IDENT, name);
    return p;
}

// separate a string pointed by 'p' and save in 'tokens'
Vector *tokenizer(char *p) {
    Vector *vec = new_vector();
    Token *token = malloc(sizeof(Token));
    while(*p){
        // skip blanc
        if (isspace(*p)){
            p++;
            continue;
        }

        if (strchr("+-*/(){}=;!,", *p)){
            if (*p == '=' && *(p+1) == '='){
                add_token(vec, TK_EQ, p);
                p+=2;
            } else if (*p == '!' && *(p+1) == '=') {
                add_token(vec, TK_EQN, p);
                p+=2;
            } else { // one charactor operator
                add_token(vec, *p, p);
                p++;
            }
            continue;
        }

        if (isalpha(*p) || *p == '_' ){
            p = add_token_ident(vec, p);
            continue;
        }

        if (isdigit(*p)){
            token = add_token(vec, TK_NUM, p);
            token->val = strtol(p, &p, 10);
            continue;
        }

        fprintf(stderr, "cannot tokenize : %s\n", p);
        exit(1);
    }
    add_token(vec, TK_EOF, p);
    return vec;
}


