#include "9cc.h"

static void gen_lval(Function *func, Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    int offset = (int)map_get(func->idents, node->name) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

static void gen_if(Function *func, Node *node){
    gen(func, node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .Lif%d\n",node->id);
    gen(func, node->lhs);
    printf("    jmp .Lelse%d\n",node->id);
    printf(".Lif%d:\n",node->id);
    if(node->rhs != NULL){
        gen(func, node->rhs);
    }
    printf(".Lelse%d:\n",node->id);
}

static void gen_while(Function *func, Node *node){
    printf(".LwhileBegin%d:\n",node->id);
    gen(func, node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LwhileEnd%d\n",node->id);
    gen(func, node->lhs);
    printf("    jmp .LwhileBegin%d\n",node->id);
    printf(".LwhileEnd%d:\n",node->id);
}

static void gen_for(Function *func, Node *node){
    printf(".LforBegin%d:\n",node->id);
    gen(func, node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LforEnd%d\n",node->id);
    gen(func, node->lhs);
    gen(func, node->rhs);
    printf("    jmp .LforBegin%d\n",node->id);
    printf(".LforEnd%d:\n",node->id);
}

// binary operation
static void gen_bin(Node *node){
    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty){
        case ND_EQ:
            printf("    cmp rdi, rax\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_EQN:
            printf("    cmp rdi, rax\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case '>':
            printf("    cmp rdi, rax\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_GE:
            printf("    cmp rdi, rax\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case '<':
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case '+':
            printf("    add rax, rdi\n");
            break;
        case '-':
            printf("    sub rax, rdi\n");
            break;
        case '*':
            printf("    mul rdi\n");
            break;
        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
        }
    printf("    push rax\n");
}

void gen(Function *func, Node *node){
    if (node == (Node *)NULL)
        return;
    else if (node->ty == ND_IF)
        gen_if(func, node);
    else if (node->ty == ND_WHILE)
        gen_while(func, node);
    else if (node->ty == ND_FOR)
        gen_for(func, node);
    else if (node->ty == ND_RETURN){
        gen(func, node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    } else if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    } else if (node->ty == ND_IDENT){
        gen_lval(func, node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    } else if (node->ty == ND_CALL){
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = node->args->len-1; i >= 0; i--){
            gen(func, (Node *)(node->args->data[i]));
            printf("    pop %s\n",reg[i]);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    } else if (node->ty == '='){
        gen_lval(func, node->lhs);
        gen(func, node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    } else { // binary operation or nop Node
        gen(func, node->lhs);
        gen(func, node->rhs);
        if (node->ty == ND_NOP)
            return;
        gen_bin(node);
    }
}
