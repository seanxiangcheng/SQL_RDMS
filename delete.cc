// CS554 project Delete Command
// By Xiang Cheng

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


/****************************************************
 * Parse DELETE Command to table_name[], inp_conds
 ***************************************************/
int ParseDelete(char *current_pos, char *table_name, char *inp_conds)
{
    int token_type, i;
    FILE *fp;
    char *current;
    char *token = new char[MAX_TOKEN];
    current = current_pos;  // Current position pointing to the commands

    current = lex_anal_pm( current, token, &token_type );  // Get next token: TABLE
    if(token_type != TABLE)
    {
        printf("\t Error: Expected 'TABLE' instead of \"%s\" after \"DELETE\"!\n", token);
        return(0);
    }
    current = lex_anal_pm( current, token, &token_type );
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: Expected relation name instead of \"%s\" after \"TABLE\"!\n", token);
        return(0);
    }
    strcpy(table_name, token);  // copy the table name for future use;
    
    for(i=0; i < MAX_COND; i++)
    {
        *(inp_conds + i) = '\0';
    }
    
    current = lex_anal_pm( current, token, &token_type );  // Get next token: TABLE
    if(token_type == 0 && *current == '\0')
    {
        printf("\t Msg: no conditions were given; all tuples will be deleted.\n");
        return(1);
    }
    else if(token_type != WHERE || strcmp(token, "WHERE") != 0)
    {
        printf("\t Error: Expected \"WHERE\" instead of \"%s\" after Relation name!\n", token);
        return(0);
    }
    
    while(isspace(*current)) current++;
    strcpy(inp_conds, current);
    
    fp = fopen("catalog", "r");
    if(fp == 0)
    {
        printf("\t Error: Cannot open catalog file \"catalog\"!\n");
        return(0);
    }
    fclose(fp);
    fp = fopen(table_name, "r");
    if(fp == 0)
    {
        printf("\t Error: Cannot open relation file \"%s\"", table_name);
        return(0);
    }
    fclose(fp);
    delete[] token;
    return(1);
}

/******************************************************
 * Execute DELETE Command to table_name[], inp_conds
 *****************************************************/
int DoDelete(char *table_name, char *inp_conds)
{
    ScanTable *f = new ScanTable();
    int token_type;
    char token[MAX_TOKEN];
    struct Node *BE_ptr;
    int *i_ptr, tot_tup=0, del_tup=0;
    bool empty_cond = 0, not_checked_cond = 1, delete_flag=0;
    double *f_ptr;
    if ( f->Open( table_name ) == 0 )
    {
        cout << "\t Error: Relation '" << table_name << "' not found !" << endl;
        return(0);
    }

    int fd;
    char temp_table[MAX_TOKEN+5];
    strcpy(temp_table, table_name);
    strcat(temp_table, ".temp");
    fd = open(temp_table, O_APPEND|O_WRONLY|O_CREAT, 0666);
    while ( f->GetNext() != 0 )
    {
        if(f->buf[f->record_size] == 'Y')
        {
            tot_tup++;
            curr = inp_conds;
            if(not_checked_cond)
            {
                lex_anal_pm(curr, token, &token_type);
                if(token_type==0 && token[0]=='\0')
                  empty_cond=1;
                not_checked_cond = 0;
            }
            if(!empty_cond)
            {
              BE_ptr = BE(f);
              if(BE_ptr == NULL)
              {
                  printf("\t Illegal inputs; unable to decide which ones to delete!\n");
                  close(fd);
                  f->Close();
                  if(remove(temp_table) != 0)
                  {
                      printf("\t Error: buffer relation file \"%s\" cannot be removed. \n", temp_table);
                      return(0);
                  }
                  return(0);
              }
              delete_flag = BE_ptr->eval_b();
              if(BE_ptr == NULL)
              {
                  printf("\t Illegal inputs; unable to evaluate the boolean expression!\n");
                  close(fd);
                  f->Close();
                  if(remove(temp_table) != 0)
                  {
                      printf("\t Error: buffer relation file \"%s\" cannot be removed.\n", temp_table);
                      return(0);
                  }
                  return(0);
              }
            }
            //printf("BE() finished!\n"); //deleteit
            //printf("BE_ptr returns:%d\n", BE_ptr->eval_b());
            //if(delete_flag == 0 || !empty_cond)
            if(!(delete_flag==1 || empty_cond==1))
            {
              if(write(fd, f->buf, f->record_size+1) == -1)
              {
                  printf("\t Error: Cannot write to table file \"%s\"!\n", temp_table);
                  close(fd);
                  f->Close();
                  return(0);
              }
            }          //BE_ptr = BE(f);
            else
            {
                del_tup++;
            }
        }
    }
    printf("\t %d out of %d tuples has been deleted.\n", del_tup, tot_tup);
    if(remove(table_name) != 0)
    {
        printf("\t Error: old relation file \"%s\" cannot be removed. \n", table_name);
        f->Close();
        close(fd);
        return(0);
    }
        
    if(rename(temp_table, table_name) !=0)
    {
        printf("\t Error: new relation file \"%s\" cannot be renamed. \n", temp_table);
        f->Close();
        close(fd);
        return(0);
    }
    f->Close();
    close(fd);
    return(1);
}


/* Boolean Expression */
struct Node *BE(struct ScanTable *f)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    //printf("BE() is called.\n"); //deleteit
    operand1 = BT(f);
    if(operand1 == NULL) return(NULL);
    curr_temp = lex_anal_pm(curr, token, &token_type); 
    //printf("OR is parsed:%d:%s\n", token_type, token); //deleteit
    while(token_type == OR)
    {
        curr = curr_temp; // pass over the token
        op = (struct Node *) malloc(sizeof(struct Node));
        op->value.p[0] = operand1;
        op->value.p[1] = BT(f);
        if (op->value.p[1] == NULL)
        {
            printf("\t Error: T(f) returns NULL.\n");
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
struct Node *BT(struct ScanTable *f)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    //printf("BT() is called.\n"); //deleteit
    operand1 = BF(f);
    if(operand1 == NULL) return(NULL);
    curr_temp = lex_anal_pm(curr, token, &token_type); 
    while(token_type == AND)
    {
        //printf(" \"AND\" is passed\n");//deleteit
        curr = curr_temp; // pass over the token
        op = (struct Node *) malloc(sizeof(struct Node));
        op->value.p[0] = operand1;
        op->value.p[1] = BF(f);
        if (op->value.p[1] == NULL)
        {
            printf("\t Error: T(f) returns NULL.\n");
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
struct Node *BF(struct ScanTable *f)
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
        op->value.q = BF(f);
        if(op->value.q == NULL)
        {
            printf("\t Error: E(f) returns NULL.\n");
            free(op);
            return(NULL);
        }
        return(op);
    }
    else if(token_type == '(')
    {
        curr = curr_temp;
        operand1 = BE(f);
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
                    printf("\t Error: BF(): unknown relation operator \"%s\"", token);
                    return(NULL);
                }
                op->value.p[1] = E(f);
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: BF(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: BF(); p[1] need to point to string!\n");
                              free(op);
                              return(NULL);                      
                          }
                    }
                    else
                    {
                        printf("\t Error: BF(); p[0] need to point to int or float or string!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
                              free(op);
                              return(NULL); 
                          }    
                    }
                    else
                    {
                        printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                              printf("\t Error: BF(); p[1] need to point to string!\n");
                              free(op);
                              return(NULL);                      
                          }
                    }
                    else
                    {
                        printf("\t Error: E(); p[0] need to point to int or float!\n");
                        free(op);
                        return(NULL);
                    }
                      
                }
                else
                {
                    printf("\t Error: BF(): unknown relation operator \"%s\"", token);
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
        op->value.p[0] = E(f);
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
            printf("\t Error:BF():operator is not >|<|=|>=|<=|!=.\n");
            return(NULL);
        }
        op->value.p[1] = E(f);
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); string can only be compared with string!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: BF(); String can only be compared with string!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: E(); string can only be compared with string!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: BF(); '=' can only compare int or float or string!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: BF(); p[1] need to point to string!\n");
                      free(op);
                      return(NULL);                      
                  }
            }
            else
            {
                printf("\t Error: E(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }
              
        }
        else
        {
            printf("\t Error: BF(): unknown relation operator \"%s\"", token);
            return(NULL);
        }

    }
    return(op);

}
/***************************** END BF() *******************************/

/* Arithmetic Expression */
struct Node *E(struct ScanTable *f)
{
    struct Node *operand1, *op;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    char *curr_temp;
    operand1 = T(f);  // Parse a term
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
            op->value.p[1] = T(f);
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: E(f) returns NULL.\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else
            {
                printf("\t Error: E(); p[0] need to point to int or float!\n");
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
            op->value.p[1] = T(f);
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: E(f) returns NULL.\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
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
                      printf("\t Error: E(); p[0] need to point to int or float!\n");
                      free(op);
                      return(NULL); 
                  }    
            }
            else
            {
                printf("\t Error: E(); p[0] need to point to int or float!\n");
                free(op);
                return(NULL);
            }
            
            if (op->value.p[1] == NULL)
            {
                printf("\t Error: E(f) returns NULL.\n");
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
struct Node *T(struct ScanTable *f)
{
  struct Node *operand1, *op;
  int token_type, token_type_peek, index=0;
  char token[MAX_TOKEN];
  char token_peek[MAX_TOKEN];
  char *curr_temp;
  operand1 = F(f);
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
          op->value.p[1] = F(f);
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: T(f) returns NULL.\n");
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
                    printf("\t Error: T(); p[0] need to point to int or float!\n");
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
                    printf("\t Error: T(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }    
          }
          else
          {
              printf("\t Error: T(); p[0] need to point to int or float!\n");
              free(op);
              return(NULL);
          }
          
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: T(f) returns NULL.\n");
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
          op->value.p[1] = F(f);
          if (op->value.p[1] == NULL)
          {
              printf("\t Error: T(f) returns NULL.\n");
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
                    printf("\t Error: T(); p[0] need to point to int or float!\n");
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
                    printf("\t Error: T(); p[0] need to point to int or float!\n");
                    free(op);
                    return(NULL); 
                }    
          }
          else
          {
              printf("\t Error: T(); p[0] need to point to int or float!\n");
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
struct Node *F(ScanTable *f)
{
    struct Node *p;
    int token_type, token_type_peek, index=0;
    char token[MAX_TOKEN];
    char token_peek[MAX_TOKEN];
    
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
        p->value.q = F(f);
        if(p->value.q == NULL) return(NULL);
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
                printf("\t Error:F():not valid type for positive/negative operator!\n");
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
                printf("\t Error:F():not valid type for positive/negative operator!\n");
                return(NULL);
            }
        }

    }
    else if(token_type == IDENTIFIER )
    {
        lex_anal_pm(curr, token_peek, &token_type_peek);
        if(token_type_peek == '.')
        {
            if(strcmp(token, f->TableName)!=0)
            {
                printf("\t Error: table name \"%s\" not found!\n", token);
                free(p);
                return(NULL);
            }
            curr =  lex_anal_pm(curr, token, &token_type); // read and pass .
            curr =  lex_anal_pm(curr, token, &token_type); // read next token
            if(token_type != IDENTIFIER)
            {
                printf("\t Error: Identifier expected after \".\" instead of \"%s\"!\n", token);
                free(p);
                return(NULL);
            }
        }
        // now this IDENTIFIER must be the attribute
        for(index=0; index < f->n_fields; index++)// scan for the variable
        {
            if(strcmp(f->DataDes[index].fieldname, token) == 0)
            {
                p = (struct Node *)malloc(sizeof(struct Node));
                if(f->DataDes[index].fieldtype == 'I')
                {
                    p->type = INT_ATTR;
                    p->value.i_attr = (int *) (&(f->buf[f->DataDes[index].startpos])); 
                    return(p);
                }
                else if(f->DataDes[index].fieldtype == 'F')
                {
                    p->type = FLOAT_ATTR;
                    p->value.f_attr = (double *) (&(f->buf[f->DataDes[index].startpos])); 
                    return(p);

                }
                else if(f->DataDes[index].fieldtype == 'C')
                {
                    p->type = CHAR_ATTR;
                    p->value.c_attr = (char *) (&(f->buf[f->DataDes[index].startpos])); 
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
        if(index == f->n_fields)
        {
            printf("\t Error: Attribute name \"%s\" not found!\n", token);
            return(NULL);
        }
        else
        {
            printf("\t Error: Attribute name \"%s\" not found!\n\
            \t F(): didn't scan all fileds in table", token);
            return(NULL);   
        }
    }
    else if(token_type == '(')
    {
        p = E(f);
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

// temp function print a factor to test F()
int print_F(struct Node *p) // temp function
{
    if(p->type==NOT_B || p->type==EXP_IB || p->type==EXP_FB 
    || p->type==LTII || p->type==LTIF || p->type==LTFI ||p->type==LTFF
    || p->type==GTII || p->type==GTIF || p->type==GTFI ||p->type==GTFF
    || p->type==LEII || p->type==LEIF || p->type==LEFI ||p->type==LEFF
    || p->type==GEII || p->type==GEIF || p->type==GEFI ||p->type==GEFF
    || p->type==NEII || p->type==NEIF || p->type==NEFI ||p->type==NEFF
    || p->type==EQII || p->type==EQIF || p->type==EQFI ||p->type==EQFF
    || p->type==AND_B || p->type==OR_B)
    {
        printf(" %d\n", p->eval_b());
        return(1);
    }
    else if(p->type==MULFF || p->type==MULIF || p->type==MULFI
    || p->type==ADDFF || p->type==ADDIF || p->type==ADDFI
    || p->type==SUBFF || p->type==SUBIF || p->type==SUBFI
    || p->type==DIVFF || p->type==DIVIF || p->type==DIVFI
    || p->type==POSF || p->type==NEGF)
    {
        printf(" %.4f \n", p->eval_f());
        return(1);
    }
    else if(p->type==MULII || p->type==DIVII 
    || p->type==ADDII || p->type==SUBII 
    || p->type==POSI || p->type==NEGI)
    {
        printf(" %d \n", p->eval_i());
        return(1);
    }
    else if(p->type == INT_CONST)
    {
        printf(" %d\n", p->value.i);
        return(1);
    }
    else if(p->type == FLOAT_CONST)
    {
        printf(" %f\n", p->value.f);
        return(1);
    }    
    else if(p->type == STRING_CONST)
    {
        printf(" %s\n", p->value.c);
        return(1);
    }
    else if(p->type == INT_ATTR)
    {
        printf(" %d\n", *(p->value.i_attr));
        return(1);
    }
    else if(p->type == FLOAT_ATTR)
    {
        printf(" %f\n", *(p->value.f_attr));
        return(1);
    }
    else if(p->type == CHAR_ATTR)
    {
        printf(" %s\n", p->value.c_attr);
        return(1);
    }
    else
    {
        printf("\t Error: print_F():No values assigned!\n");
        return(0);
    }
    return(0);
}
