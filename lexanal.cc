/*
     THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING            
     A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Xiang Cheng (1938871)
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexanal.h"
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

/* read all characters until "go" is detected;
   returns the number of characters in the end */
int read_command(char *input) 
{
    // to temporily store the input line
    char* line = new char[MAX_LINE];         // Max character per line 256
    int len_ch = 0, index=0;
    bool break_flag = false;
    for(int i=0; i<MAX_INPUT; i++)
      input[i] = '\0';
    while(1)
    {
      cin.getline(line, MAX_LINE);
      len_ch = strlen(line);
      if(len_ch>=2) // detect "go"
      {
        for(int i=0; i<len_ch-1; i++)
        {
            if(line[i] != 'g' && !isspace(line[i]))
                break;
            else if(line[i] == 'g' && line[i+1] == 'o' && (isspace(line[i+2]) || line[i+2]=='\0'))
            {    
                break_flag = true;
                break;
            }
        }
        for(int i=0; i<len_ch-1; i++)
        {
            if(line[i] != 'G' && !isspace(line[i]))
                break;
            else if(line[i] == 'G' && (line[i+1] == 'O' || line[i+1]=='o') && (isspace(line[i+2]) || line[i+2]=='\0'))
            {    
                break_flag = true;
                break;
            }
        }
      }
      if(len_ch>=4) // detect "exit"
      {
        for(int i=0; i<len_ch-3; i++)
        {
            if(line[i] != 'e' && !isspace(line[i]))
                break;
            else if(line[i] == 'e' && line[i+1] == 'x' && line[i+2] == 'i' && line[i+3] =='t' && (isspace(line[i+4]) || line[i+4]=='\0'))
            {    
                exit(1);
            }
        }          
      }
      if(break_flag) break;
      if(index>0) input[index++] = '\n';
      for(int i=0; i<len_ch; i++)
      {
          input[index++] = line[i];
      }
      cout << "      ...>> ";

    }
    delete[] line;
    return index;
}


/* lexicographical analyzer: break the input SQL command into its lexicographical tokens*/
char *lex_anal(char *current_pos, char *token, int *token_type)      
{   
    int i, j=0;
    char *orig_token_copy = new char[MAX_TOKEN];
    char single_tokens[] = ",()[]:*;/<>=\"";
    bool dot_yes = false;
    for(i=0; i<MAX_TOKEN; i++)
    {
        token[i]='\0';
        orig_token_copy[i] = '\0';
    }
    *token_type = 0;
    if(isspace(*current_pos)) // if the first char is whitespace
    {
        while(isspace(*(current_pos)) && *current_pos != '\0')
        {
            current_pos++;
            j++;
        }
    }
    
    if(*(current_pos) == '\0') // if it is the end the commands
    {
        //cout << "\t Error: Nothing is entered! \n\t OR Already the end of the commands!" << endl;
        return (char*)(current_pos);
    }
    else if (isalpha(*current_pos))  // if first char is letter
    {
        token[0] = (char)toupper(*current_pos);
        orig_token_copy[0] = *current_pos;
        for(i=1; i<MAX_TOKEN-j; i++)
        {
            if(isalpha(*(current_pos+i)) || isdigit(*(current_pos+i)) || *(current_pos+i)=='_')
            {
                token[i] = (char)toupper(*(current_pos+i));
                orig_token_copy[i] = *(current_pos+i);
            }
            else
                break;
        }
    }
    else if (isdigit(*current_pos)) // if first char is digit
    {
        token[0] = *(current_pos);
        for (i = 1; i < MAX_TOKEN-j; i++)
        {
            if(isdigit(*(current_pos+i)))
                token[i] = *(current_pos+i);
            else if (dot_yes==false && *(current_pos+i)=='.')
            {
                dot_yes = true;
                token[i] = *(current_pos+i);
            }
            else
            {
                if(dot_yes==false)
                {
                    *token_type = INTEGER_NUMBER;
                    return (char *)(current_pos+i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos+i);
                }
            }
        }
    }
    else if (*current_pos=='.') // if first char is '.'
    {
        token[0] = '.';
        if(!isdigit(*(current_pos+1)))
        {
            *token_type = (int)'.';
            return (char *)(current_pos+1);
        }
        else
        {
            token[1] = *(current_pos+1);
            for (i = 2; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos + i)))
                {
                    token[i] = *(current_pos + i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos + i);
                }
            }
        }
    }
    else if (*current_pos=='+' || *current_pos=='-') // if first char is '+' or '-'
    {
        token[0] = *current_pos;
        if(isdigit(*(current_pos+1))) // if the 2nd char is number
        {
            token[1] = *(current_pos);
            for (i = 2; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos+i)))
                    token[i] = *(current_pos+i);
                else if (dot_yes==false && *(current_pos+i)=='.')
                {
                    dot_yes = true;
                    token[i] = *(current_pos+i);
                }
                else
                {
                    if(dot_yes==false)
                    {
                        *token_type = INTEGER_NUMBER;
                        return (char *)(current_pos+i);
                    }
                    else
                    {
                        *token_type = FLOAT_NUMBER;
                        return (char *)(current_pos+i);
                    }
                }
            }
        }
        else if (*(current_pos+1)=='.' && isdigit(*(current_pos+2))) 
        // if the 2nd char is '.' && the 3rd is number 
        {
            token[1] = '.';
            token[2] = *(current_pos + 2);
            for (i = 3; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos + i)))
                {
                    token[i] = *(current_pos + i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos + i);
                }
            }
        }
        else
        {
            *token_type = (int)(*current_pos);
            return (char *)(current_pos + 1);
        }
    }
    else if (*current_pos == '<' || *current_pos == '>' || *current_pos == '!')
    // if first char is '<' or '>' or '!'
    {
        token[0] = *current_pos;
        if(*(current_pos+1)== '=')
        {
            token[1] = '=';
            i = 2;
            // the token_type is determined later, 
            // and the pos+i is returned latter
        }
        else
        {
            *token_type = (int)(*current_pos);
            return (char *)(current_pos+1);
        }
    }
    else if (*current_pos == '\'') // if first char is ' which means string
    {
        for (i = 0; i < MAX_TOKEN - j; i++)
        {
            if(*(current_pos + i + 1) == '\'') 
            {
                *token_type = STRING;
                return (char *)(current_pos+i+2);
            }
            else if (*(current_pos + i + 1) == '\0')
            {
                *token_type = STRING;
                return (char *)(current_pos+i);
            }
            else
            {
                token[i] = *(current_pos + i + 1);
            }
        }
        
    }
    else if (strchr(single_tokens, *current_pos) != NULL) // if single
    {
        token[0] = *current_pos;
        *token_type = (int)(*current_pos);
        return (char *)(current_pos+1);
    }
    else // all other cases will stop at whitespace
    {   
        token[0] = *current_pos;
        for (i = 1; i < MAX_TOKEN - j; i++)
        {
            if(isspace(*(current_pos+i)))
                break;
            else
                token[i] = *(current_pos + i);
        }
    }
    
    if(strcmp(token, "<=")==0)
        *token_type = LESSOREQ;
    else if(strcmp(token, ">=")==0)
        *token_type =  GREATEROREQ;
    else if(strcmp(token, "!=")==0) 
        *token_type = NOTEQ;
    else if(strcmp(token, "ALL")==0) 
        *token_type = ALL;
    else if(strcmp( token, "AND" ) == 0 )
        *token_type = AND;
    else if(strcmp( token, "ANY" ) == 0 )
        *token_type = ANY;
    else if(strcmp( token, "AS" ) == 0 )
        *token_type = AS;
    else if(strcmp( token, "AVG") == 0 )
        *token_type = AVG;
    else if(strcmp(token, "BETWEEN")==0)
        *token_type = BETWEEN; 
    else if (strcmp( token, "BY" ) == 0 )
        *token_type = BY;
    else if (strcmp( token, "CHAR" ) == 0 )
       *token_type = CHAR;
    else if (strcmp( token, "CHECK" ) == 0 )
        *token_type = CHECK; 
    else if (strcmp( token, "CLOSE" ) == 0 )
        *token_type = CLOSE; 
    else if (strcmp( token, "COMMIT" ) == 0 )
        *token_type = COMMIT;
    else if (strcmp( token, "COUNT" ) == 0 )
        *token_type = COUNT;
    else if (strcmp( token, "CREATE" ) == 0 )
        *token_type = CREATE; 
    else if (strcmp( token, "DECIMAL" ) == 0 )
        *token_type = DECIMAL; 
    else if (strcmp( token, "DELETE" ) == 0 )
        *token_type = DELETE;
    else if (strcmp( token, "DISTINCT" ) == 0 )
        *token_type = DISTINCT; 
    else if (strcmp( token, "DOUBLE" ) == 0 )
        *token_type = DOUBLE; 
    else if (strcmp( token, "DROP" ) == 0 )
        *token_type = DROP; 
    else if (strcmp( token, "EXISTS" ) == 0 )
        *token_type = EXISTS; 
    else if (strcmp( token, "FLOAT" ) == 0 )
        *token_type = FLOAT;
    else if (strcmp( token, "FROM" ) == 0 )
        *token_type = FROM; 
    else if (strcmp( token, "GO" ) == 0 )
        *token_type = GO; 
    else if (strcmp( token, "GROUP" ) == 0 )
        *token_type = GROUP; 
    else if (strcmp( token, "HAVING" ) == 0 )
        *token_type = HAVING; 
    else if (strcmp( token, "IN" ) == 0 )
        *token_type = IN; 
    else if (strcmp( token, "INSERT" ) == 0 )
        *token_type = INSERT; 
    else if (strcmp( token, "INT" ) == 0 )
        *token_type = INT; 
    else if (strcmp( token, "INTO" ) == 0 )
        *token_type = INTO; 
    else if (strcmp( token, "IS" ) == 0 )
        *token_type = IS; 
    else if (strcmp( token, "LIKE" ) == 0 )
        *token_type = LIKE; 
    else if (strcmp( token, "MAX" ) == 0 )
        *token_type = MAX; 
    else if (strcmp( token, "MIN" ) == 0 )
        *token_type = MIN; 
    else if (strcmp( token, "NOT" ) == 0 )
        *token_type = NOT; 
    else if (strcmp( token, "NULL" ) == 0 )
        *token_type = NULL0; 
    else if (strcmp( token, "NUMERIC" ) == 0 )
        *token_type = NUMERIC; 
    else if (strcmp( token, "OF" ) == 0 )
        *token_type = OF; 
    else if (strcmp( token, "ON" ) == 0 )
        *token_type = ON; 
    else if (strcmp( token, "OR" ) == 0 )
        *token_type = OR; 
    else if (strcmp( token, "ORDER" ) == 0 )
        *token_type = ORDER; 
    else if (strcmp( token, "PRIMARY" ) == 0 )
        *token_type = PRIMARY; 
    else if (strcmp( token, "REAL" ) == 0 )
        *token_type = REAL; 
    else if (strcmp( token, "SCHEMA" ) == 0 )
        *token_type = SCHEMA; 
    else if (strcmp( token, "SELECT" ) == 0 )
        *token_type = SELECT; 
    else if (strcmp( token, "SET" ) == 0 )
        *token_type = SET; 
    else if (strcmp( token, "SOME" ) == 0 )
        *token_type = SOME; 
    else if (strcmp( token, "SUM" ) == 0 )
        *token_type = SUM; 
    else if (strcmp( token, "TABLE" ) == 0 )
        *token_type = TABLE; 
    else if (strcmp( token, "TO" ) == 0 )
        *token_type = TO; 
    else if (strcmp( token, "UNION" ) == 0 )
        *token_type = UNION; 
    else if (strcmp( token, "UNIQUE" ) == 0 )
        *token_type = UNIQUE; 
    else if (strcmp( token, "UPDATE" ) == 0 )
        *token_type = UPDATE; 
    else if (strcmp( token, "USER" ) == 0 )
        *token_type = USER; 
    else if (strcmp( token, "VALUES" ) == 0 )
        *token_type = VALUES; 
    else if (strcmp( token, "VIEW" ) == 0 )
        *token_type = VIEW; 
    else if (strcmp( token, "WHERE" ) == 0 )
        *token_type = WHERE; 
    else if (strcmp( token, "WITH" ) == 0 )
        *token_type = WITH; 
    else if(isalpha(token[0]))
    {
        *token_type = IDENTIFIER;
        for(j=0; j<MAX_TOKEN; j++)
        {
            if(token[j] == '\0') break;
            token[j] = orig_token_copy[j];
        }
    }
    else
        *token_type = 0;
    delete[] orig_token_copy;
    return (char*)(current_pos+i);
}


/* lexicographical analyzer: break the input SQL command into its lexicographical tokens*/
/* + and - will be send back separately no matter whether there is a number after it*/
char *lex_anal_pm(char *current_pos, char *token, int *token_type)      
{   
    int i, j=0;
    char *orig_token_copy = new char[MAX_TOKEN];
    char single_tokens[] = ",()[]:*;/<>=\"";
    bool dot_yes = false;
    for(i=0; i<MAX_TOKEN; i++)
    {
        token[i]='\0';
        orig_token_copy[i] = '\0';
    }
    *token_type = 0;
    if(isspace(*current_pos)) // if the first char is whitespace
    {
        while(isspace(*(current_pos)) && *current_pos != '\0')
        {
            current_pos++;
            j++;
        }
    }
    
    if(*(current_pos) == '\0') // if it is the end the commands
    {
        //cout << "\t Error: Nothing is entered! \n\t OR Already the end of the commands!" << endl;
        return (char*)(current_pos);
    }
    else if (isalpha(*current_pos))  // if first char is letter
    {
        token[0] = (char)toupper(*current_pos);
        orig_token_copy[0] = *current_pos;
        for(i=1; i<MAX_TOKEN-j; i++)
        {
            if(isalpha(*(current_pos+i)) || isdigit(*(current_pos+i)) || *(current_pos+i)=='_')
            {
                token[i] = (char)toupper(*(current_pos+i));
                orig_token_copy[i] = *(current_pos+i);
            }
            else
                break;
        }
    }
    else if (isdigit(*current_pos)) // if first char is digit
    {
        token[0] = *(current_pos);
        for (i = 1; i < MAX_TOKEN-j; i++)
        {
            if(isdigit(*(current_pos+i)))
                token[i] = *(current_pos+i);
            else if (dot_yes==false && *(current_pos+i)=='.')
            {
                dot_yes = true;
                token[i] = *(current_pos+i);
            }
            else
            {
                if(dot_yes==false)
                {
                    *token_type = INTEGER_NUMBER;
                    return (char *)(current_pos+i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos+i);
                }
            }
        }
    }
    else if (*current_pos=='.') // if first char is '.'
    {
        token[0] = '.';
        if(!isdigit(*(current_pos+1)))
        {
            *token_type = (int)'.';
            return (char *)(current_pos+1);
        }
        else
        {
            token[1] = *(current_pos+1);
            for (i = 2; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos + i)))
                {
                    token[i] = *(current_pos + i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos + i);
                }
            }
        }
    }
    else if (*current_pos=='+' || *current_pos=='-') // if first char is '+' or '-'
    {
        token[0] = *current_pos;
        *token_type = (int)(*current_pos);
        return (char *)(current_pos + 1);
        /*
        if(isdigit(*(current_pos+1))) // if the 2nd char is number
        {
            token[1] = *(current_pos);
            for (i = 2; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos+i)))
                    token[i] = *(current_pos+i);
                else if (dot_yes==false && *(current_pos+i)=='.')
                {
                    dot_yes = true;
                    token[i] = *(current_pos+i);
                }
                else
                {
                    if(dot_yes==false)
                    {
                        *token_type = INTEGER_NUMBER;
                        return (char *)(current_pos+i);
                    }
                    else
                    {
                        *token_type = FLOAT_NUMBER;
                        return (char *)(current_pos+i);
                    }
                }
            }
        }
        else if (*(current_pos+1)=='.' && isdigit(*(current_pos+2))) 
        // if the 2nd char is '.' && the 3rd is number 
        {
            token[1] = '.';
            token[2] = *(current_pos + 2);
            for (i = 3; i < MAX_TOKEN-j; i++)
            {
                if(isdigit(*(current_pos + i)))
                {
                    token[i] = *(current_pos + i);
                }
                else
                {
                    *token_type = FLOAT_NUMBER;
                    return (char *)(current_pos + i);
                }
            }
        }
        else
        {
            *token_type = (int)(*current_pos);
            return (char *)(current_pos + 1);
        }
        */
    }
    else if (*current_pos == '<' || *current_pos == '>' || *current_pos == '!')
    // if first char is '<' or '>' or '!'
    {
        token[0] = *current_pos;
        if(*(current_pos+1)== '=')
        {
            token[1] = '=';
            i = 2;
            // the token_type is determined later, 
            // and the pos+i is returned latter
        }
        else
        {
            *token_type = (int)(*current_pos);
            return (char *)(current_pos+1);
        }
    }
    else if (*current_pos == '\'') // if first char is ' which means string
    {
        for (i = 0; i < MAX_TOKEN - j; i++)
        {
            if(*(current_pos + i + 1) == '\'') 
            {
                *token_type = STRING;
                return (char *)(current_pos+i+2);
            }
            else if (*(current_pos + i + 1) == '\0')
            {
                *token_type = STRING;
                return (char *)(current_pos+i);
            }
            else
            {
                token[i] = *(current_pos + i + 1);
            }
        }
        
    }
    else if (strchr(single_tokens, *current_pos) != NULL) // if single
    {
        token[0] = *current_pos;
        *token_type = (int)(*current_pos);
        return (char *)(current_pos+1);
    }
    else // all other cases will stop at whitespace
    {   
        token[0] = *current_pos;
        for (i = 1; i < MAX_TOKEN - j; i++)
        {
            if(isspace(*(current_pos+i)))
                break;
            else
                token[i] = *(current_pos + i);
        }
    }
    
    if(strcmp(token, "<=")==0)
        *token_type = LESSOREQ;
    else if(strcmp(token, ">=")==0)
        *token_type =  GREATEROREQ;
    else if(strcmp(token, "!=")==0) 
        *token_type = NOTEQ;
    else if(strcmp(token, "ALL")==0) 
        *token_type = ALL;
    else if(strcmp( token, "AND" ) == 0 )
        *token_type = AND;
    else if(strcmp( token, "ANY" ) == 0 )
        *token_type = ANY;
    else if(strcmp( token, "AS" ) == 0 )
        *token_type = AS;
    else if(strcmp( token, "AVG") == 0 )
        *token_type = AVG;
    else if(strcmp(token, "BETWEEN")==0)
        *token_type = BETWEEN; 
    else if (strcmp( token, "BY" ) == 0 )
        *token_type = BY;
    else if (strcmp( token, "CHAR" ) == 0 )
       *token_type = CHAR;
    else if (strcmp( token, "CHECK" ) == 0 )
        *token_type = CHECK; 
    else if (strcmp( token, "CLOSE" ) == 0 )
        *token_type = CLOSE; 
    else if (strcmp( token, "COMMIT" ) == 0 )
        *token_type = COMMIT;
    else if (strcmp( token, "COUNT" ) == 0 )
        *token_type = COUNT;
    else if (strcmp( token, "CREATE" ) == 0 )
        *token_type = CREATE; 
    else if (strcmp( token, "DECIMAL" ) == 0 )
        *token_type = DECIMAL; 
    else if (strcmp( token, "DELETE" ) == 0 )
        *token_type = DELETE;
    else if (strcmp( token, "DISTINCT" ) == 0 )
        *token_type = DISTINCT; 
    else if (strcmp( token, "DOUBLE" ) == 0 )
        *token_type = DOUBLE; 
    else if (strcmp( token, "DROP" ) == 0 )
        *token_type = DROP; 
    else if (strcmp( token, "EXISTS" ) == 0 )
        *token_type = EXISTS; 
    else if (strcmp( token, "FLOAT" ) == 0 )
        *token_type = FLOAT;
    else if (strcmp( token, "FROM" ) == 0 )
        *token_type = FROM; 
    else if (strcmp( token, "GO" ) == 0 )
        *token_type = GO; 
    else if (strcmp( token, "GROUP" ) == 0 )
        *token_type = GROUP; 
    else if (strcmp( token, "HAVING" ) == 0 )
        *token_type = HAVING; 
    else if (strcmp( token, "IN" ) == 0 )
        *token_type = IN; 
    else if (strcmp( token, "INSERT" ) == 0 )
        *token_type = INSERT; 
    else if (strcmp( token, "INT" ) == 0 )
        *token_type = INT; 
    else if (strcmp( token, "INTO" ) == 0 )
        *token_type = INTO; 
    else if (strcmp( token, "IS" ) == 0 )
        *token_type = IS; 
    else if (strcmp( token, "LIKE" ) == 0 )
        *token_type = LIKE; 
    else if (strcmp( token, "MAX" ) == 0 )
        *token_type = MAX; 
    else if (strcmp( token, "MIN" ) == 0 )
        *token_type = MIN; 
    else if (strcmp( token, "NOT" ) == 0 )
        *token_type = NOT; 
    else if (strcmp( token, "NULL" ) == 0 )
        *token_type = NULL0; 
    else if (strcmp( token, "NUMERIC" ) == 0 )
        *token_type = NUMERIC; 
    else if (strcmp( token, "OF" ) == 0 )
        *token_type = OF; 
    else if (strcmp( token, "ON" ) == 0 )
        *token_type = ON; 
    else if (strcmp( token, "OR" ) == 0 )
        *token_type = OR; 
    else if (strcmp( token, "ORDER" ) == 0 )
        *token_type = ORDER; 
    else if (strcmp( token, "PRIMARY" ) == 0 )
        *token_type = PRIMARY; 
    else if (strcmp( token, "REAL" ) == 0 )
        *token_type = REAL; 
    else if (strcmp( token, "SCHEMA" ) == 0 )
        *token_type = SCHEMA; 
    else if (strcmp( token, "SELECT" ) == 0 )
        *token_type = SELECT; 
    else if (strcmp( token, "SET" ) == 0 )
        *token_type = SET; 
    else if (strcmp( token, "SOME" ) == 0 )
        *token_type = SOME; 
    else if (strcmp( token, "SUM" ) == 0 )
        *token_type = SUM; 
    else if (strcmp( token, "TABLE" ) == 0 )
        *token_type = TABLE; 
    else if (strcmp( token, "TO" ) == 0 )
        *token_type = TO; 
    else if (strcmp( token, "UNION" ) == 0 )
        *token_type = UNION; 
    else if (strcmp( token, "UNIQUE" ) == 0 )
        *token_type = UNIQUE; 
    else if (strcmp( token, "UPDATE" ) == 0 )
        *token_type = UPDATE; 
    else if (strcmp( token, "USER" ) == 0 )
        *token_type = USER; 
    else if (strcmp( token, "VALUES" ) == 0 )
        *token_type = VALUES; 
    else if (strcmp( token, "VIEW" ) == 0 )
        *token_type = VIEW; 
    else if (strcmp( token, "WHERE" ) == 0 )
        *token_type = WHERE; 
    else if (strcmp( token, "WITH" ) == 0 )
        *token_type = WITH; 
    else if(isalpha(token[0]))
    {
        *token_type = IDENTIFIER;
        for(j=0; j<MAX_TOKEN; j++)
        {
            if(token[j] == '\0') break;
            token[j] = orig_token_copy[j];
        }
    }
    else
        *token_type = 0;
    delete[] orig_token_copy;
    return (char*)(current_pos+i);
}


int print_table(char *relname, char *current_pos)
{
    char token[MAX_TOKEN];
    int token_type, printed_tuple_num=0, want_tuple_num=100;
    ScanTable *f = new ScanTable();
    lex_anal( current_pos, token, &token_type );    // Get next token
    want_tuple_num = 100;
    if(token_type == INTEGER_NUMBER)
    {
        want_tuple_num = atoi(token);
    }
    
    if ( f->Open( relname ) == 0 )
    {
        cout << "\t Relation '" << relname << "' not found !" << endl;
        return(0);
    }

    f->PrintRelationInfo();


    while ( f->GetNext() != 0 && printed_tuple_num < want_tuple_num)
    {
      /* ------------------
         Print it...
         ------------------ */
        f->PrintTuple();
        printed_tuple_num++;
    }

    f->Close();

    printf("\n");
}







