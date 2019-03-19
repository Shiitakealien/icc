#include "9cc.h"

int pos = 0;
Vector *tokens;

int main(int argc, char **argv){
    int i;
    Function *func;
    if (argc != 2){
        fprintf(stderr, "usage: ./9cc [code]\n");
        return 1;
    }

    if (strcmp(argv[1] ,"-test") == 0)
        runtest();
    else{
        // tokenize
        tokens = tokenizer(argv[1]);
        Vector *funcs = program();

        // write the header of assembly
        printf(".intel_syntax noprefix\n");
        printf(".global");
        for (i = 0; i < funcs->len; i++)
            printf(" %s,",((Function *)(funcs->data[i]))->name); // Maybe need to fix
        printf("\n");
        for (i = 0; i < funcs->len; i++){
            func = (Function *)(funcs->data[i]);
            printf("%s:",func->name);

            // prologue
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n", func->idents->keys->len*8);

            // generate a code from the head
            for (int j = 0; func->code[j]; j++){
                gen(func,func->code[j]);
                printf("    pop rax\n");
            }
            // We have the result at the top of the stack
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
        }
    }
    return 0;
}
