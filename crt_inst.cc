// CS554 project CREATE & INSERT Command
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
#include <fcntl.h>
using namespace std;


/* Create Table based on the Input */
int ParseCreate(char *current_pos, char *table_name, char *attributes, char *data_types, int *var_lengths)
{   
    char *token = new char[MAX_TOKEN];  // to store single token
    int  token_type;        // token_type or token_code defined in lexanal.cc
    char *current;
    int i=0;
    char *p_at, *p_dt;      // pointer to attributes and data_types;
    int *p_vl;              // pointer to var_lengths;
    p_at = attributes;      
    p_dt = data_types;
    p_vl = var_lengths;
    current = current_pos;
    for (i=0; i<MAX_TOKEN; i++)
        table_name[i] = '\0';
    for (i = 0; i < MAX_ATTRIBUTE; i++)
    {
        attributes[i] = '\0';
    }
    current = lex_anal( current, token, &token_type );  // Get next token: TABLE
    if(token_type != TABLE)
    {
        printf("\t Error: \"%s\", keyword TABLE expected.\n", token);
        return(0);
    }
    
    current = lex_anal( current, token, &token_type );  // Get next token: IDENTIFIER
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: \"%s\", table name expected.\n", token);
        return(0);
    }
    strcpy(table_name, token);                          // save table name
    //cout << "table name: " << table_name << endl;
    current = lex_anal( current, token, &token_type );  // Get next token: '('
    if(token_type != '(')
    {
        printf("\t Error: \"%s\", '(' expected.\n", token);
        return(0);
    }
    
    do
    {
        current = lex_anal( current, token, &token_type );  // Get next token: IDENTIFIER, table name
        if(token_type != IDENTIFIER)
        { 
            printf("\t Error: \"%s\", attribute name expected.\n", token);
            return(0);
        }
        i=0;
        while(token[i] != '\0')
        {
            *p_at++ = token[i++];
        }
        *p_at++ = ' ';                                      // add a space bettween each attribute name
        //cout << " attribute: " << token << endl;
        
        current = lex_anal( current, token, &token_type );  // Get next token: IDENTIFIER, table name
        if(token_type == INT)
        {
            *p_dt++ = 'I';
            *p_vl++ = sizeof(int);
        }
        else if(token_type == FLOAT)
        {
            *p_dt++ = 'F';
            *p_vl++ = sizeof(double);
        }
        else if(token_type == CHAR)
        {
            *p_dt++ = 'C';
            
            current = lex_anal( current, token, &token_type );  // Get next token
            if (token_type != '(')
            {
                printf("\t Error: \"%s\", '(' expected.\n", token);
                return(0);
            }

            current = lex_anal( current, token, &token_type );  // Get next token
            if (token_type != INTEGER_NUMBER)
            {
                printf("\t Error: \"%s\", integer size expected.\n", token);
                return(0);
            }
            *p_vl++ = atoi(token);
            //cout << " length :" << atoi(token) << endl;
            current = lex_anal( current, token, &token_type );  // Get next token
            if (token_type != ')')
            {
                printf("\t Error: \"%s\", ')' expected.\n", token);
                return(0);
            }
        }
        
        current = lex_anal( current, token, &token_type );  // Get next token
    }while(token_type == ',');
    if(token_type != ')')
    {
        printf("\t Error: \"%s\", ')' expected.\n", token);
        return(0);
    }
    *p_at = '\0';   // the attributes end with '\0';
    *p_dt = '\0';   // the data types end with '\0';
    *p_vl = -1;     // the variables lengths end with -1;
    
    delete[] token;
    return(1);
}

/* Do Create Table */
int DoCreate(char *table_name, char *attributes, char *data_types, int *var_lengths)
{
    char *attribute = new char[MAX_TOKEN];
    string line_in_cat ;//= new char[MAX_TOKEN*8];
    char *p_attr = attributes;
    int i, start_pos=0, length=0;
    bool non_space_flag=0;
    int at_index=0;     // attribute index, which attribute is printing
    int len_table_name = strlen(table_name);
    char *exist_table_name = new char[len_table_name];
    /*
    if(strcmp(table_name, "rel_name")==0)
    {
        printf("\t Error: rel_name is one of the heads of the catalog!\n  Please pick another table name!\n");
        return(0);
    }
    */
    FILE *fp;
    for (i=0; i< MAX_TOKEN; i++)
        attribute[i] = '\0';
    fp = fopen(CATALOG_FILE, "r");  // try to open the file
    if (fp == NULL)  // if not successful, create one 
    {
        fp = fopen(CATALOG_FILE, "w");
        /*
        //fprintf(fp, "%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", "rel_name",\
         "attr_name", "data_type", "start_pos", "length");
        */
    }
    else  // if successful, write data at the end
    {
        fclose(fp);
        ifstream ifs (CATALOG_FILE);
        while(getline(ifs, line_in_cat))
        {
            non_space_flag=0;
            for(i = 0; i < len_table_name; i++)
            {
                if(isspace(line_in_cat[i]) == 0)
                    non_space_flag = 1;
                if(isspace(line_in_cat[i]) && non_space_flag==1)
                {
                    break;
                }
                if(isspace(line_in_cat[i])==0)
                    exist_table_name[i] =  line_in_cat[i];
            }
            if(strcmp(exist_table_name, table_name)==0)
            {
                ifs.close();
                printf("\t Error: table \"%s\" already exist.\n", table_name);
                return(0);
            }
        }
        fp = fopen(CATALOG_FILE, "a");
    }
    while ( *p_attr != '\0')
    {
        i=0;
        while(*p_attr != ' ')
            attribute[i++] = *p_attr++;
        attribute[i] = '\0';
        p_attr++;
        
        if(*(data_types + at_index) == 'I')
        {
            while(start_pos % 4 !=0)
                start_pos++;
            length = var_lengths[at_index];
        }
        else if(*(data_types + at_index) == 'F')
        {
            while(start_pos % 8 !=0)
                start_pos++;   
            length = var_lengths[at_index];
        }
        else if(*(data_types + at_index) == 'C')
        {
            length = var_lengths[at_index] + 1;
        }
        else
        {
            printf("\t Error: Data types can only be 'I'/'F'/'C'!\n");
            return(0);
        }
        fprintf(fp, "%-8s\t%-8s\t%-3c\t%-4d\t%-4d\n", table_name, attribute, *(data_types+at_index), start_pos, length );
        start_pos += length;
        at_index++;
    }
    fclose(fp);
    fp = fopen(table_name,"w");
    fclose(fp);
    delete[] exist_table_name;
    delete[] attribute;
    return(1);
}

/* Drop Table based on the Input */
int ParseDrop(char *current_pos, char *table_name)
{
    char *token = new char[MAX_TOKEN];  // to store single token
    int  token_type;        // token_type or token_code defined in lexanal.cc
    char *current;
    int i=0;
    current = current_pos;
    for(i=0; i<MAX_TOKEN; i++)
        table_name[i] = '\0';
    
    current = lex_anal( current, token, &token_type );  // Get next token: TABLE
    if(token_type != TABLE)
    {
        printf("\t Error: \"%s\", keyword TABLE expected.\n", token);
        return(0);
    }
    
    current = lex_anal( current, token, &token_type );  // Get next token: IDENTIFIER
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: \"%s\", table name expected.\n", token);
        return(0);
    }
    strcpy(table_name, token);                          // save table name
    
    delete[] token;
    return(1);
}
/* Do Drop Table*/
int DoDrop(char *table_name)
{
    ifstream ifs;
    ofstream ofs;
    string line;
    bool table_found_or_not = 0, non_space_flag=0;
    int i;
    char *exist_table_name = new char[MAX_TOKEN];
    /*
    if(strcmp(table_name, "rel_name")==0)
    {
        printf("\t Error: rel_name is one of the heads of the catalog!\n  Please pick another table name!\n");
        return(0);
    }
    */
    ifs.open(CATALOG_FILE);
    ofs.open("catalog.temp");
    while(getline(ifs, line))
    {   
        non_space_flag = 0;
        for(i=0; i<MAX_TOKEN; i++)
        {
            if(isspace(line[i]) == 0)
                non_space_flag = 1;
            if(isspace(line[i]) && non_space_flag == 1)
                break;
            else
                exist_table_name[i] = line[i];
        }
        exist_table_name[i] = '\0';
        if(strcmp(exist_table_name, table_name) != 0) //if not the table
            ofs << line << endl;        // keep the info
        else
            table_found_or_not = 1;     // if it is the table, found (1)
    }
    delete[] exist_table_name;
    if(table_found_or_not == 0) // if table not found
    {
        printf("\t Error: table name \"%s\" not found!\n", table_name);
        ifs.close();
        ofs.close();
        remove("catalog.temp");
        return(0);
    }
    else // if table found
    {
        ifs.close();
        ofs.close();
        if(remove(CATALOG_FILE) != 0)
        {
            printf("\t Error: old catalog file \"%s\" cannot be removed. \n", CATALOG_FILE);
            return(0);
        }
        
        if(rename("catalog.temp", CATALOG_FILE) !=0)
        {
            printf("\t Error: new catalog file \"%s\" cannot be renamed. \n", "catalog.temp");
            return(0);
        }
        
        if(remove(table_name) != 0 )
        {
            printf("\tTable info has been removed from 'catalog'\n");
            printf("\t Error: table file \"%s\" cannot be removed.\n", table_name);
            return(0);
        }
    }
    return(1);
}

/************************************************
 * Parse Insert Command to data_des, buf, etc
 ************************************************/
int ParseInsert(char *current_pos, char *table_name, DataDescription *data_des, char *buf, int *record_size)
{   
    int  n_fields, n_in_fields, token_type;
    int int_num;
    double float_num;
    FILE *fp;
    char *current;
    char *token = new char[MAX_TOKEN];
    current = current_pos;  // Current position pointing to the commands
    *record_size = 0;
    current = lex_anal( current, token, &token_type );  // Get next token: TABLE
    if(token_type != INTO)
    {
        printf("\t Error: Expected 'INTO' instead of \"%s\" after 'Insert'!\n", token);
        return(0);
    }
    current = lex_anal( current, token, &token_type );
    if(token_type != TABLE)
    {
        printf("\t Error: Expected 'TABLE' instead of \"%s\" after 'INTO'!\n", token);
        return(0);
    }
    current = lex_anal( current, token, &token_type );
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: Expected relation name instead of \"%s\" after 'TABLE'!\n", token);
        return(0);
    }
    strcpy(table_name, token);  // copy the table name for future use;
    /*
    if(strcmp(table_name, "rel_name")==0)
    {
        printf("\t Error: 'rel_name' is one of the heads of the catalog!\n\
        \t\tPlease pick another table name, if possible.\n");
        return(0);
    }
    */
    current = lex_anal( current, token, &token_type );
    if(token_type != VALUES)
    {
        printf("\t Error: Expected 'VALUES' instead of \"%s\" after relation name!\n", token);
        return(0);
    }
    
    current = lex_anal( current, token, &token_type );
    if(token_type != (int)'(')
    {
        printf("\t Error: Expected '(' instead of \"%s\" after 'VALUES'!\n", token);
        return(0);
    }
    
    fp = fopen("catalog", "r");
    n_fields = 0;
    while ( fscanf(fp, "%s %s %c %d %d", 
		data_des[n_fields].relname,
		data_des[n_fields].fieldname,
		&data_des[n_fields].fieldtype, 
		&data_des[n_fields].startpos,
		&data_des[n_fields].fieldsize) > 0 )
    {
        if ( strcmp( data_des[n_fields].relname, table_name ) == 0 )
        {
            if ( data_des[n_fields].startpos + data_des[n_fields].fieldsize >
                 *record_size )
            {
                *record_size = data_des[n_fields].startpos 
                            + data_des[n_fields].fieldsize;
            }
            n_fields++;
        }
    }
    if ( n_fields == 0 )
    {
        printf("\t Error: Relation \"%s\" is not found!\n", table_name);
        return(0);
    }
    else if( n_fields > MAX_FIELD)
    {
        printf("\t Error: This software only supports a max attribute number of %d!\n", MAX_FIELD);
        return(0);
    }
    n_in_fields = 0;
    do
    {
        if(n_in_fields >= n_fields)
        {
            printf("\t Error: Too many values! Only %d values expected! \n", n_fields);
            return(0);
        }
        current = lex_anal( current, token, &token_type );  // Get next token: IDENTIFIER, table name
        if ( data_des[n_in_fields].fieldtype == 'I')
        {
            if(token_type == INTEGER_NUMBER)
            {
                int_num = atoi(token);
                memcpy(&buf[data_des[n_in_fields].startpos], &int_num, sizeof(int));
            }
            else
            {
                printf("\t Error: \"%s\"; integer number expected!\n", token);
                return(0);
            }
        }
        else if (data_des[n_in_fields].fieldtype == 'F')
        {
            if(token_type == FLOAT_NUMBER || token_type == INTEGER_NUMBER)
            {
                float_num = atof(token);
                memcpy(&buf[data_des[n_in_fields].startpos], &float_num, sizeof(double));
            }
            else
            {
                printf("\t Error: \"%s\"; float number expected!\n", token);
                return(0);
            }
        }
        else if (data_des[n_in_fields].fieldtype == 'C')
        {
            if(token_type == STRING)
            {
                if( (int)strlen(token) > (data_des[n_in_fields].fieldsize - 1))
                {
                    printf("\t Error: \"%s\" is too long! String length <%d is expected!\n", 
                    token, data_des[n_in_fields].fieldsize - 1);
                    return(0);
                }
                memcpy(&buf[data_des[n_in_fields].startpos], token, data_des[n_in_fields].fieldsize);
            }
            else
            {
                printf("\t Error: \"%s\"; string expected!\n", token);
                return(0);
            }
        }
        else
        {
            printf("\t Error: The data_des[%d].fieldtype is not 'I', 'F', or 'C' but '%c'\n", n_in_fields, data_des[n_in_fields].fieldtype);
            return(0);
        }
        n_in_fields++;
        current = lex_anal( current, token, &token_type );  // Get next token
        
    } while(token_type == ',');
    
    if(n_in_fields < n_fields)
    {
        printf("\t Error: Too few values!\n\t%d values expected! Only %d values entered!\n", n_fields, n_in_fields);
        return(0);
    }
    if(token_type != ')')
    {
        printf("\t Error: \"%s\", ')' OR ',' expected.\n", token);
        return(0);
    }
    buf[*record_size] = 'Y';
    delete[] token;
    return(1);
}

/************************************************
 * Parse Insert Command to data_des, buf, etc
 ************************************************/
int DoInsert(char *table_name, char *buf, int record_size)
{
    int fd;
    fd = open(table_name, O_APPEND|O_WRONLY|O_CREAT, 0666);
    if(fd == -1)
    {
        printf("\t Error: Table file \"%s\" cannot be open!\n", table_name);
        close(fd);
        return(0);
    }
    if(buf[record_size] != 'Y')
    {
        printf("\t Error: tuple-chunck is not valid!\n");
        close(fd);
        return(0);
    }
    
    if(write(fd, buf, record_size+1) == -1)
    {
        printf("\t Error: Cannot write to table file \"%s\"!\n", table_name);
        close(fd);
        return(0);
    }
    close(fd);
    return(1);
}

