#include "9cc.h"

static Node *stmt(Function *func);
static Node *assign(Function *func);
static Node *eq(Function *func);
static Node *add(Function *func);
static Node *mul(Function *func);
static Node *term(Function *func);

static Node *new_node(int ty, Node *lhs, Node *rhs){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_term(int ty, int val, char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->val = val;
    node->name = input;
    return node;
}

static int consume(int ty){
    Token *token = (Token *)(tokens->data[pos]);
    if (token->ty != ty)
        return 0;
    pos++;
    return 1;
}

static void add_ident(Map *map, char *name){
    if (map_get(map, name) == NULL)
        map_put(map, name, (void *)(map->keys->len));
}

static Function *add_func(Vector *funcs, char *name){
    Function *func = malloc(sizeof(Function));
    func->name = name;
    func->idents = new_map();
    vec_push(funcs, (void *)func);
    return func;
}

Vector *program(){
    int i;
    Function *func;
    Vector *funcs = new_vector();
    while (((Token *)(tokens->data[pos]))->ty != TK_EOF){
        if (((Token *)(tokens->data[pos]))->ty != TK_IDENT){
            fprintf(stderr,"function name is needed");
            exit(1);
        } else {
            i = 0;
            func = add_func(funcs, ((Token *)(tokens->data[pos]))->input);
            pos++;
            consume('(');
            consume(')');
            consume('{');
            while (((Token *)(tokens->data[pos]))->ty != '}')
                func->code[i++] = stmt(func);
            func->code[i] = NULL;
            consume('}');
        }
    }
    return funcs;
}

static Node *stmt(Function *func){
    Token *token = (Token *)(tokens->data[pos]);
    Node *node = assign(func);
    if (!consume(';')){
        fprintf(stderr,"token without ';': %s", token->input);
        exit(1);
    }
    return node;
}        

static Node *assign(Function *func){
    Node *node = eq(func);
    for (;;)
        if (consume('='))
            node = new_node('=', node, assign(func));
        else
            return node;
}

static Node *eq(Function *func){
    Node *node = add(func);
    for (;;)
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, add(func));
        else if (consume(TK_EQN))
            node = new_node(ND_EQN, node, add(func));
        else
            return node;
}

static Node *add(Function *func){
    Node *node = mul(func);
    for (;;)
        if (consume('+'))
            node = new_node('+', node, mul(func));
        else if (consume('-'))
            node = new_node('-', node, mul(func));
        else
            return node;
}

static Node *mul(Function *func){
    Node *node = term(func);
    for (;;)
        if (consume('*'))
            node = new_node('*', node, term(func));
        else if (consume('/'))
            node = new_node('/', node, term(func));
        else
            return node;
}
    
static Node *term(Function *func){
    Token *token = (Token *)(tokens->data[pos]);

    if (consume('(')){
        Node *node = add(func);
        if (!consume(')')){
            fprintf(stderr,"'(' without ')': %s", 
                    token->input);
            exit(1);
        }
        return node;
    }

    if (consume(TK_NUM))
        return new_node_term(ND_NUM,token->val,token->input);
    
    if (consume(TK_IDENT)){
        if (!consume('(')){
            add_ident(func->idents, token->input);
            return new_node_term(ND_IDENT,token->val,token->input);
        }
        else {
            Node *node = new_node_term(ND_CALL,token->val,token->input);
            node->args = new_vector();
            if (!consume(')')) { // get arguments
                do{
                     vec_push(node->args, (void *)assign(func));
                }while(consume(','));
                consume(')');
            }
            return node;
        }
    }

    fprintf(stderr,"found an unknown token: %s", token->input);
    exit(1);
}
