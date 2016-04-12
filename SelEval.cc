#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexanal.h"
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>       /* isnan, sqrt */
#include <fcntl.h>
using namespace std;


/* Boolean Expression */
struct Node *SelBE(struct SELECT_class *s)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    //printf("BE() is called.\n"); //deleteit
    operand1 = SelBT(s);
    if(operand1 == NULL) return(NULL);
    curr_temp = lex_anal_pm(curr, token, &token_type); 
    //printf("OR is parsed:%d:%s\n", token_type, token); //deleteit
    while(token_type == OR)
    {
        curr = curr_temp; // pass over the token
        op = (struct Node *) malloc(sizeof(struct Node));
        op->value.p[0] = operand1;
        op->value.p[1] = SelBT(s);
        if (op->value.p[1] == NULL)
        {
            printf("\t Error: SelT(s) returns NULL.\n");
            free(op);
            return(NULL);
        }
        op->type = OR_B;
        operand1 = op;
        curr_temp = lex_anal_pm(curr, token, &token_type); 
        //printf(" Token after AND:%d:%s", token_type, token); //deleteit
    }
    
    return(operand1);
}


/* Boolean Term */
struct Node *SelBT(struct SELECT_class *s)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    //printf("BT() is called.\n"); //deleteit
    operand1 = SelBF(s);
    if(operand1 == NULL) return(NULL);
    curr_temp = lex_anal_pm(curr, token, &token_type); 
    while(token_type == AND)
    {
        //printf(" \"AND\" is passed\n");//deleteit
        curr = curr_temp; // pass over the token
        op = (struct Node *) malloc(sizeof(struct Node));
        op->value.p[0] = operand1;
        op->value.p[1] = SelBF(s);
        if (op->value.p[1] == NULL)
        {
            printf("\t Error: SelT(s) returns NULL.\n");
            free(op);
            return(NULL);
        }
        op->type = AND_B;
        operand1 = op;
        curr_temp = lex_anal_pm(curr, token, &token_type); 
        //printf(" Token after AND:%d:%s", token_type, token); //deleteit
    }
    return(operand1);
}


/* Boolean Factor */
/***************************** Begin BF() *******************************/
struct Node *SelBF(struct SELECT_class *s)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    curr_temp = lex_anal_pm(curr, token, &token_type);   
    // peek; don't read since it may not be * or /
    //printf("BF() is called\n"); //delete it
    if(token_type == NOT)
    {
        curr = curr_temp;
        op = (struct Node *) malloc(sizeof(struct Node));
        op->type = NOT_B;
        op->value.q = SelBF(s);
        if(op->value.q == NULL)
        {
            printf("\t Error: SelE(s) returns NULL.\n");
            free(op);
            return(NULL);
        }
        return(op);
    }
    else if(token_type == '(')
    {
        curr = curr_temp;
        operand1 = SelBE(s);
        if(operand1 == NULL) return(NULL);
        curr = lex_anal_pm(curr, token, &token_type);   
        if(token_type == ')')
        {
            //added1
            if(operand1->type==INT_CONST || operand1->type==FLOAT_CONST || operand1->type==STRING_CONST
            || operand1->type==INT_ATTR || operand1->type==FLOAT_ATTR || operand1->type==CHAR_ATTR
            || operand1->type==POSI || operand1->type==POSF || operand1->type==NEGI || operand1->type==NEGF
            || operand1->type==ADDII || operand1->type==ADDIF || operand1->type==ADDFI || operand1->type==ADDFF
            || operand1->type==SUBII || operand1->type==SUBIF || operand1->type==SUBFI || operand1->type==SUBFF
            || operand1->type==MULII || operand1->type==MULIF || operand1->type==MULFI || operand1->type==MULFF
            || operand1->type==DIVII || operand1->type==DIVIF || operand1->type==DIVFI || operand1->type==DIVFF
            )
            {
                op = (struct Node *) malloc(sizeof(struct Node));
                op->value.p[0] = operand1;
                curr_temp=lex_anal_pm(curr, token, &token_type);
                if(token_type==LESSOREQ || token_type==GREATEROREQ || token_type==NOTEQ
                || token_type=='<' || token_type=='>' || token_type=='=')
                { 
                    curr = curr_temp;
                }
                else
                {
                    printf("\t Error: SelBF(): unknown relation operator \"%s\"", token);
                    return(NULL);
                }
                op->value.p[1] = SelE(s);
                if(op->value.p[1] == NULL) return(NULL);

                //curr = curr_temp; // added1
                /* < less than*/
                if(token_type=='<')
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = LTII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = LTIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = LTFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = LTFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }
                }
                /* > greater than*/
                else if(token_type=='>')
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = GTII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = GTIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = GTFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = GTFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }

                }
                /* = equal to*/
                else if(token_type=='=')
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = EQII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = EQIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = EQFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = EQFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelBF(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
                    {
                          if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                          {
                              op->type = EQSS;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelBF(); p[1] need to point to string!\n");
                              free(op);
                              return(NULL);                      
                          }
                    }
                    else
                    {
                        printf("\t Error: SelBF(); p[0] need to point to int or float or string!\n");
                        free(op);
                        return(NULL);
                    }

                }
                /* <= less equal to*/
                else if(token_type==LESSOREQ)
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = LEII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = LEIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = LEFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = LEFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }

                }
                /* >= greater equal to*/
                else if(token_type==GREATEROREQ)
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = GEII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = GEIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = GEFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = GEFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }

                }
                
                /* != not equal to*/
                else if(token_type==NOTEQ)
                {
                    if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
                    || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
                    || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
                    || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = NEII;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = NEIF;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }         
                    }
                    // else if p[0] returns float, p[0]->eval_f
                    else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
                    || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
                    || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
                    || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
                    || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
                    || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
                    {
                          if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                          || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                          || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                          || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                          {
                              op->type = NEFI;
                              return(op);
                          }
                          else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                          || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                          || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                          || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                          || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                          || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                          {
                              op->type = NEFF;
                              return(op);
                              //printf(" LTFF type assigned!\n"); // delete 
                          }
                          else
                          {
                              printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    
                    else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
                    {
                          if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                          {
                              op->type = NESS;
                              return(op);
                          }
                          else
                          {
                              printf("\t Error: SelBF(); p[1] need to point to string!\n");
                              free(op);
                              return(NULL);                      
                          }
                    }
                    else
                    {
                        printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }
                      
                }
                else
                {
                    printf("\t Error: SelBF(): unknown relation operator \"%s\"", token);
                    return(NULL);
                }                
              
            }
            //before added1
            else
              return(operand1);
        }
        else
        {
            printf("\t Syntax error: \"%s\"; ')' was expected!\n", token);
            return(NULL);
        }
    }
    else
    {
        op = (struct Node *) malloc(sizeof(struct Node));
        op->value.p[0] = SelE(s);
        if(op->value.p[0] == NULL) return(NULL);
        curr_temp = lex_anal_pm(curr, token, &token_type);
        // if it is the relation operator
        if(token_type==LESSOREQ || token_type==GREATEROREQ || token_type==NOTEQ
        || token_type=='<' || token_type=='>' || token_type=='=')
        { 
            curr = curr_temp;
        }
        /* Just expression, no relation operator */
        else if(token_type == 0 && token[0]=='\0')
        {
            curr = curr_temp;
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                op->type = EXP_IB;
                return(op);
            }
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                op->type = EXP_FB;
                return(op);
            } 
        }
        else if(token_type == ')')
        {
            operand1 = op->value.p[0]; // added1
            free(op);                  // added1
            return(operand1);          // added1
        }
        else
        {
            printf("\t Error: SelBF():operator is not >|<|=|>=|<=|!=.\n");
            return(NULL);
        }
        op->value.p[1] = SelE(s);
        if(op->value.p[1] == NULL) return(NULL);

        //curr = curr_temp; // added1
        /* < less than*/
        if(token_type=='<')
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = LTII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = LTIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = LTFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = LTFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelBF(); string can only be compared with string!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {
                      op->type = LTSS;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelBF(); String can only be compared with string!\n");
                free(op);
                return(NULL);
            }
        }
        /* > greater than*/
        else if(token_type=='>')
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = GTII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = GTIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = GTFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = GTFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {  
                      op->type = GTSS;
                      return(op);
                  }  
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelE(); string can only be compared with string!\n");
                free(op);
                return(NULL);
            }

        }
        /* = equal to*/
        else if(token_type=='=')
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = EQII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = EQIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = EQFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = EQFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {
                      op->type = EQSS;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelBF(); '=' can only compare int or float or string!\n");
                free(op);
                return(NULL);
            }

        }
        /* <= less equal to*/
        else if(token_type==LESSOREQ)
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = LEII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = LEIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = LEFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = LEFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {
                      op->type = LESS;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }

        }
        /* >= greater equal to*/
        else if(token_type==GREATEROREQ)
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = GEII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = GEIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = GEFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = GEFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {
                      op->type = GESS;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }

        }
        
        /* != not equal to*/
        else if(token_type==NOTEQ)
        {
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = NEII;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = NEIF;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = NEFI;
                      return(op);
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = NEFF;
                      return(op);
                      //printf(" LTFF type assigned!\n"); // delete 
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            
            else if(op->value.p[0]->type==STRING_CONST || op->value.p[0]->type==CHAR_ATTR )
            {
                  if(op->value.p[1]->type==STRING_CONST || op->value.p[1]->type==CHAR_ATTR)
                  {
                      op->type = NESS;
                      return(op);
                  }
                  else
                  {
                      printf("\t Error: SelBF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }
              
        }
        else
        {
            printf("\t Error: SelBF(): unknown relation operator \"%s\"", token);
            return(NULL);
        }

    }
    return(op);

}
/***************************** END BF() *******************************/

/* Arithmetic Expression */
struct Node *SelE(struct SELECT_class *s)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    operand1 = SelT(s);  // Parse a term
    if(operand1 == NULL) return(NULL);
    
    curr_temp = lex_anal_pm(curr, token, &token_type);   // peek; don't read since it may not be * or /
    //printf("token: %s, token type: %d\n", token, token_type);
    while(token_type == '+' || token_type=='-')
    {
        curr = curr_temp;
        if(token_type=='+')
        {
            //printf("ADD'+' is parsed\n"); // deleteit
            op = (struct Node *) malloc(sizeof(struct Node));
            // how to tell the type
            op->value.p[0] = operand1;
            op->value.p[1] = SelT(s);
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: SelE(s) returns NULL.\n");
                free(op);
                return(NULL);
            }
            // if p[0] return int, p[0]->eval_i
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = ADDII;
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = ADDIF;
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = ADDFI;
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = ADDFF;
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else
            {
                printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }
            
            operand1 = op; //prepare for next iteration to find more * | /            
        }
        // else if SUB
        else if(token_type=='-')
        {
            op = (struct Node *) malloc(sizeof(struct Node));
            // how to tell the type
            op->value.p[0] = operand1;
            op->value.p[1] = SelT(s);
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: SelE(s) returns NULL.\n");
                free(op);
                return(NULL);
            }
            // if p[0] return int, p[0]->eval_i
            if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
            || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
            || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
            || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI) // if int+
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = SUBII;
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = SUBIF;
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }         
            }
            // else if p[0] returns float, p[0]->eval_f
            else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
            || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
            || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
            || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
            || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
            || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
            {
                  if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                  || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                  || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                  || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                  {
                      op->type = SUBFI;
                  }
                  else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                  || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                  || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                  || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                  || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                  || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                  {
                      op->type = SUBFF;
                  }
                  else
                  {
                      printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else
            {
                printf("\t Error: SelE(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }
            
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: SelE(s) returns NULL.\n");
                free(op);
                return(NULL);
            }
            operand1 = op; //prepare for next iteration to find more * | /
        }
        curr_temp = lex_anal_pm(curr, token, &token_type); 
    }
    return(operand1);
}


/************** Arithmetic Term *****************/
struct Node *SelT(struct SELECT_class *s)
{
  struct Node *operand1, *op;
  int token_type, token_type_peek, index=0;
  char token[MAX_TOKEN];
  char token_peek[MAX_TOKEN];
  char *curr_temp;
  operand1 = SelF(s);
  if(operand1 == NULL) return(NULL);
  curr_temp = lex_anal_pm(curr, token, &token_type);   // peek; don't read since it may not be * or /
  
  while(token_type == '*' || token_type == '/')
  {
      curr = curr_temp;
      if (token_type == '*')
      {
          op = (struct Node *) malloc(sizeof(struct Node));
          // how to tell the type
          
          op->value.p[0] = operand1;
          op->value.p[1] = SelF(s);
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: SelT(s) returns NULL.\n");
              free(op);
              return(NULL);
          }
          // if p[0] return int, p[0]->eval_i
          if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
          || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
          || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
          || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI)
          {
                if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                {
                    op->type = MULII;
                }
                else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                {
                    op->type = MULIF;
                }
                else
                {
                    printf("\t Error: SelT(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }         
          }
          // else if p[0] returns float, p[0]->eval_f
          else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
          || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
          || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
          || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
          || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
          || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
          {
                if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                {
                    op->type = MULFI;
                }
                else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                {
                    op->type = MULFF;
                }
                else
                {
                    printf("\t Error: SelT(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }    
          }
          else
          {
              printf("\t Error: SelT(); p[0] need to point to int or float!\n");
              free(op);
              return(NULL);
          }
          
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: SelT(s) returns NULL.\n");
              free(op);
              return(NULL);
          }
          operand1 = op; //prepare for next iteration to find more * | /
      }
      /* if it is DIV */
      else if(token_type == '/')
      {
          op = (struct Node *) malloc(sizeof(struct Node));
          // how to tell the type
          
          op->value.p[0] = operand1;
          op->value.p[1] = SelF(s);
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: SelT(s) returns NULL.\n");
              free(op);
              return(NULL);
          }
          // if p[0] return int, p[0]->eval_i
          if(op->value.p[0]->type==INT_CONST || op->value.p[0]->type==INT_ATTR
          || op->value.p[0]->type==ADDII || op->value.p[0]->type==SUBII
          || op->value.p[0]->type==MULII || op->value.p[0]->type==DIVII
          || op->value.p[0]->type==POSI || op->value.p[0]->type==NEGI)
          {
                if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                {
                    op->type = DIVII;
                }
                else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                {
                    op->type = DIVIF;
                }
                else
                {
                    printf("\t Error: SelT(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }         
          }
          // else if p[0] returns float, p[0]->eval_f
          else if(op->value.p[0]->type==FLOAT_CONST || op->value.p[0]->type==FLOAT_ATTR
          || op->value.p[0]->type==ADDIF || op->value.p[0]->type==ADDFI || op->value.p[0]->type==ADDFF 
          || op->value.p[0]->type==SUBIF || op->value.p[0]->type==SUBFI || op->value.p[0]->type==SUBFF 
          || op->value.p[0]->type==MULIF || op->value.p[0]->type==MULFI || op->value.p[0]->type==MULFF
          || op->value.p[0]->type==DIVIF || op->value.p[0]->type==DIVFI || op->value.p[0]->type==DIVFF
          || op->value.p[0]->type==POSF || op->value.p[0]->type==NEGF)
          {
                if(op->value.p[1]->type==INT_CONST || op->value.p[1]->type==INT_ATTR
                || op->value.p[1]->type==ADDII || op->value.p[1]->type==SUBII
                || op->value.p[1]->type==MULII || op->value.p[1]->type==DIVII
                || op->value.p[1]->type==POSI || op->value.p[1]->type==NEGI)
                {
                    op->type = DIVFI;
                }
                else if(op->value.p[1]->type==FLOAT_CONST || op->value.p[1]->type==FLOAT_ATTR
                || op->value.p[1]->type==ADDIF || op->value.p[1]->type==ADDFI || op->value.p[1]->type==ADDFF 
                || op->value.p[1]->type==SUBIF || op->value.p[1]->type==SUBFI || op->value.p[1]->type==SUBFF 
                || op->value.p[1]->type==MULIF || op->value.p[1]->type==MULFI || op->value.p[1]->type==MULFF
                || op->value.p[1]->type==DIVIF || op->value.p[1]->type==DIVFI || op->value.p[1]->type==DIVFF
                || op->value.p[1]->type==POSF || op->value.p[1]->type==NEGF) 
                {
                    op->type = DIVFF;
                }
                else
                {
                    printf("\t Error: SelT(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }    
          }
          else
          {
              printf("\t Error: SelT(); p[0] need to point to int or float!\n");
              free(op);
              return(NULL);
          }
          operand1 = op; //prepare for next iteration to find more * | /

      }
      curr_temp = lex_anal_pm(curr, token, &token_type); 
      //printf("Token:%s; token_type:%d\n", token, token_type);
  }
  return(operand1);
}



/* Arithmetic Factor */
struct Node *SelF(struct SELECT_class *s)
{
    struct Node *p;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    int tb_ind = 0, at_ind = 0, i, j;
    bool tb_found=0;
    
    curr = lex_anal_pm(curr, token, &token_type);
    if(token_type == FLOAT_NUMBER)
    {
        p = (struct Node *)malloc(sizeof(struct Node));
        p->type = FLOAT_CONST;
        p->value.f = atof(token);
        return(p);
    }
    else if(token_type == INTEGER_NUMBER)
    {
        p = (struct Node *)malloc(sizeof(struct Node));
        p->type = INT_CONST;
        p->value.i = atoi(token);
        //printf("number'1' is parsed:%d\n", p->value.i); // deleteit
        return(p);
    }
    else if(token_type == STRING)
    {
        p = (struct Node *)malloc(sizeof(struct Node));
        p->type = STRING_CONST; 
        strcpy(p->value.c, token);
        return(p);
    }
    else if(token_type=='+' || token_type=='-')
    {
        p = (struct Node *)malloc(sizeof(struct Node));
        p->value.q = SelF(s);
        if(p->value.q == NULL) 
        {   
            free(p);
            return(NULL);
        }
        if(token_type=='+')
        {
            if(p->value.q->type==INT_CONST || p->value.q->type==INT_ATTR)
            {
                p->type = POSI;
                return(p);
            }
            else if(p->value.q->type==FLOAT_CONST || p->value.q->type==FLOAT_ATTR)
            {
                p->type = POSF;
                return(p);
            }
            else
            {
                printf("\t Error:SelF():not valid type for positive/negative operator!\n");
                free(p);
                return(NULL);
            }
        }
        else if(token_type=='-')
        {
            if(p->value.q->type==INT_CONST || p->value.q->type==INT_ATTR)
            {
                p->type = NEGI;
                return(p);
            }
            else if(p->value.q->type==FLOAT_CONST || p->value.q->type==FLOAT_ATTR)
            {
                p->type = NEGF;
                return(p);
            }
            else
            {
                printf("\t Error:SelF():not valid type for positive/negative operator!\n");
                free(p);
                return(NULL);
            }
        }

    }
    else if(token_type == IDENTIFIER )
    {   // major changes 1 for Select 
        /*** If IDENTIFIER, 1. find table ind; 2. find attribute ind ***/
        lex_anal_pm(curr, token_peek, &token_type_peek);
        if(token_type_peek == '.')
        {
            tb_found=0; // flag 0 for whether table is found
            for(i=0; i < s->NRels; i++)
            {
                if(s->Rel[i]->alias[0] != '\0')
                { // compare alias, ignore table name
                    if( strcmp(token, s->Rel[i]->alias)==0 )
                    {
                        if(tb_found==0)
                        { // first found
                            tb_ind = i;
                            tb_found=1;
                        }
                        else if(tb_found == 1)
                        {   // ambigous table alias
                            printf("\t Error: Ambigous alias:\"%s\"", token);
                            return(NULL);
                        }
                        else
                        { // unexpected error
                            printf("\t Error: unexpected! tb_found is %d!\n", tb_found);
                            return(NULL);
                        }
                        
                    }
                }
                else
                { // compare table name, ignore alias
                    if( strcmp(token, s->Rel[i]->TableName)==0 )
                    {
                        if(tb_found==0)
                        { // first found
                            tb_ind = i;
                            tb_found=1;
                        }
                        else if(tb_found == 1)
                        {   // ambigous table alias
                            printf("\t Error: Ambigous alias:\"%s\"", token);
                            return(NULL);
                        }
                        else
                        { // unexpected error
                            printf("\t Error: unexpected! tb_found is %d!\n", tb_found);
                            return(NULL);
                        }
                        
                    }
                }
            } // for: go through rels to find table or alias
            if(tb_found==0)
            {
                printf("\t Error: table name \"%s\" not found!\n", token);
                free(p);
                return(NULL);
            }
            curr =  lex_anal_pm(curr, token, &token_type); // read and pass .
            if(token_type != '.'){ printf("\t Error: not '.'!\n"); return(NULL);}
            curr =  lex_anal_pm(curr, token, &token_type); // read next token:attribute name
            if(token_type != IDENTIFIER)
            {
                printf("\t Error: Identifier expected after \".\" instead of \"%s\"!\n", token);
                free(p);
                return(NULL);
            }
        } //end if('.'); now the token is the identifier after '.'
        else
        { // the identifier should be an attribute name
          
            tb_found=0; // flag 0 for whether table is found
            for(i=0; i < s->NRels; i++)
            {
                for(j=0; j < s->Rel[i]->n_fields; j++)
                { // compare attribute name
                    if( strcmp(token, s->Rel[i]->DataDes[j].fieldname)==0 )
                    {
                        if(tb_found==0)
                        { // first found
                            tb_ind = i;
                            tb_found=1;
                        }
                        else if(tb_found == 1)
                        {   // ambigous attribute name
                            printf("\t Error: Ambigous alias:\"%s\"", token);
                            return(NULL);
                        }
                        else
                        { // unexpected error
                            printf("\t Error: unexpected! tb_found is %d!\n", tb_found);
                            return(NULL);
                        }
                        
                    }
                }
            } // for: go through rels to find table or alias
            if(tb_found==0)
            {
                printf("\t Error: table name \"%s\" not found!\n", token);
                free(p);
                return(NULL);
            }
        } // end else: identifier is an attribute without '.'
        // table index has been found; next find attribute and return pointer
        
        // now this IDENTIFIER must be the attribute; table_index has been found
        for(index=0; index < s->Rel[tb_ind]->n_fields; index++)// scan for the variable
        {
            if(strcmp( s->Rel[tb_ind]->DataDes[index].fieldname, token ) == 0)
            {
                p = (struct Node *)malloc(sizeof(struct Node));
                if(s->Rel[tb_ind]->DataDes[index].fieldtype == 'I')
                {
                    p->type = INT_ATTR;
                    p->value.i_attr = (int *) (&(s->Rel[tb_ind]->buf[s->Rel[tb_ind]->DataDes[index].startpos])); 
                    return(p);
                }
                else if(s->Rel[tb_ind]->DataDes[index].fieldtype == 'F')
                {
                    p->type = FLOAT_ATTR;
                    p->value.f_attr = (double *) (&(s->Rel[tb_ind]->buf[s->Rel[tb_ind]->DataDes[index].startpos])); 
                    return(p);

                }
                else if(s->Rel[tb_ind]->DataDes[index].fieldtype == 'C')
                {
                    p->type = CHAR_ATTR;
                    p->value.c_attr = (char *) (&(s->Rel[tb_ind]->buf[s->Rel[tb_ind]->DataDes[index].startpos])); 
                    return(p);
                }
                else
                {
                    printf("\t Error in catalog file! Filedtype is not 'I', 'F', or 'C'!\n");
                    free(p);
                    return(NULL);
                }
            }
        }
        if(index == s->Rel[tb_ind]->n_fields)
        {
            printf("\t Error: Attribute name \"%s\" not found!\n", token);
            return(NULL);
        }
        else
        {
            printf("\t Wired Error: Attribute name \"%s\" not found!\n\
            \t SelF(): didn't scan all fileds in table", token);
            return(NULL);   
        }
    } // if(identifier) End
    
    else if(token_type == '(')
    {
        p = SelE(s);
        if(p == NULL) return(NULL);
        curr = lex_anal_pm(curr, token, &token_type);
        if(token_type == ')') 
          return(p);
        else
        {
            printf("\t Syntax error: \"%s\"; ')' was expected!\n", token);
            return(NULL);
        }
    }
    else
    {
        printf("\t Syntax error: \"%s\"; number | string | attribute was expected!\n", token);
        return(NULL);
    }
  
}
