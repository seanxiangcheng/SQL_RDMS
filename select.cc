// Select file
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

int Select_Exec(char *current_pos, char *inp_conds)
{
    int token_type, token_type_peek, i, index_ar, j=0; // i and index_ar are index of something
    FILE *fp;
    char *current;
    char *token = new char[MAX_TOKEN], token_peek[MAX_TOKEN];
    char new_attr_tb[MAX_TOKEN*2];
    SELECT_class *Select = new SELECT_class();
    /******* Small Checks *******/
    fp = fopen("catalog", "r"); // check catalog file
    if(fp == 0)
    {
        printf("\t Error: Cannot open catalog file \"catalog\"!\n");
        delete Select;
        return(0);
    }
    
    Init_Select(Select);
    current = current_pos;  // Current position pointing to the commands
    
    // Initialize input_conditions to be empty
    for(i=0; i < MAX_COND; i++)
    {
        *(inp_conds + i) = '\0';
    }

    current = lex_anal_pm( current, token, &token_type );  // Get next token: TABLE
    if(token_type == SELECT)
    {
          printf("\t \"SELECT\" command detected!\n");
          current = lex_anal_pm( current, token, &token_type);    // Get next token
    }
    
    /******** Get Attributes Names ********/
    index_ar = 0; // index of relnames or attrnames;
    while(token_type != FROM)
    {
        if(token_type == 0)
        {
           printf("\t Error: \"FROM\" and relation names are expected!\n");
           delete Select;
           return(0); 
        }
        i=0;
        while(token[i] != '\0')
        {
            Select->AttrName[Select->NAttrs][index_ar++] = token[i++];
        }
        current = lex_anal_pm( current, token, &token_type );  // Get next token: TABLE
        if( token_type == ',' || token_type == FROM)
        {
            Select->NAttrs += 1;
            index_ar = 0;
            if(token_type == ',')
                current = lex_anal_pm( current, token, &token_type ); 
        }
        
        if(token_type!=IDENTIFIER && token_type!=INTEGER_NUMBER && token_type!=FLOAT_NUMBER &&
        token_type!=STRING && token_type!='+' && token_type!='-' && token_type!='*' && token_type!='/' &&
        token_type!=',' && token_type!=FROM && token_type!=LESSOREQ && token_type!=GREATEROREQ && 
        token_type!=NOTEQ && token_type!=NOT && token_type!=OR && token_type!=AND && token_type != '.')
        {
            printf("\t Error: \"%s\" not expected! Token_type=%d!\n", token,token_type);
            delete Select;
            return(0);
        }
    }

    /******** Get Relation Names ********/
    current = lex_anal_pm( current, token, &token_type );  // Get next token: TABLE
    while(token_type != WHERE)
    {
        if(token[0] == '\0')
        {
           printf("\t Msg: No \"WHERE\" condition given; all tuples will be selected!\n");
           break;
        }
        else if(token_type == 0)
        {
            printf("\t Error: Illeagl input \"%s\"!\n", token);
            delete Select;
            return(0);
        }
        
        Select->Rel[Select->NRels] = new ScanTable();
        if ( Select->Rel[Select->NRels]->Open(token) == 0 )
        {
            cout << "\t Error: Relation \"" << token << "\" not found !" << endl;
            delete Select;
            return(0);
        }
        current = lex_anal_pm( current, token, &token_type ); //, where or identifier
        
        if(token_type == IDENTIFIER)
        {
            strcpy(Select->Rel[Select->NRels]->alias, token);
            Select->NRels += 1;
            current = lex_anal_pm( current, token, &token_type ); //, where or identifier
            if(token_type == ',')
                current = lex_anal_pm( current, token, &token_type );

        }
        else if(token_type == ',')
        {
            Select->NRels += 1;
            current = lex_anal_pm( current, token, &token_type ); //get next table name
            if(token_type == WHERE)
            {
                printf("\t Error: IDENTIFIER/new relation expected instead of \"%s\"\n", token);
                return(0);
            }
            else if(token_type != IDENTIFIER)
            {
                printf("\t Error: IDENTIFIER/new relation expected instead of \"%s\"\n", token);
            }
        }
        else if(token[0] == '\0' || token_type == WHERE)
        {
            Select->NRels += 1;
            break;
        }
        else
        {
            printf("\t Error: Illegal input \"%s\"!\n", token);
            delete Select;
            return(0);
        }
    }

    // test Select
    //Print_select(Select);
    
    /***** Edit Attribute name to explicitly show table name: EMP.NAME*****/
    if(Select->NAttrs==1 && Select->AttrName[0][0]=='*')
    { // if *, all attributes are selected
        Select->NAttrs = 0;
        for(i=0; i < Select->NRels ; i++)
        {
            for(j=0; j < Select->Rel[i]->n_fields; j++)
            {
                if( Select->Rel[i]->alias[0]=='\0' )
                {
                    strcpy(Select->AttrName[Select->NAttrs], Select->Rel[i]->TableName);
                }
                else
                {
                    strcpy(Select->AttrName[Select->NAttrs], Select->Rel[i]->alias);
                }
                strcat(Select->AttrName[Select->NAttrs], ".");
                strcat(Select->AttrName[Select->NAttrs], Select->Rel[i]->DataDes[j].fieldname);
                Select->NAttrs += 1;
            }
        }
    }
    else if(Select->NAttrs!=1 && Select->AttrName[0][0]=='*')
    {
        printf("\t Error:\"*\" is detected, but other more attrs are entered.\n");
        delete Select;
        return(0);
    }
    else
    { // else if explict attributes are selected
        for (i=0; i < Select->NAttrs; i++)
        {
            if(Add_TB2Attr(Select, i) == 0)
            {
                printf("\t Error: Cannot Understand AttrNames!\n");
                return(0);
            }
        }
    }
    // test Select
    //Print_select(Select);
    
    /******** Parse Input Conditions: include IN *********/
    if(token_type == WHERE) // parse input conditions
    {
        curr = inp_conds;
        current = lex_anal_pm( current, token, &token_type ); //get next table name
        while(token[0] != '\0')
        {
            if(token_type == 0)
            {
              printf("\t Error: Illeagl input \"%s\"!\n", token);
              delete Select;
              return(0);
            }
            if(token_type == IDENTIFIER)
            {
                lex_anal_pm( current, token_peek, &token_type_peek ); //get next table name
                if(token_type_peek == '.')
                {
                    j=0;
                    while(token[j] != '\0')
                    {
                        *curr = token[j++];
                        curr++;
                    }
                    *(curr++) = '.';
                    current = lex_anal_pm( current, token, &token_type ); // skip '.'
                    current = lex_anal_pm( current, token, &token_type ); // get identifier
                    if(token_type == IDENTIFIER)
                    {
                        j=0;
                        while(token[j] != '\0')
                        {
                            *curr = token[j++];
                            curr++;
                        }
                        *(curr++) = ' ';
                    }
                    else
                    {
                        printf("\t Error: Attribute name expected after \".\" instead of \"%s\"!\n", token);
                        return(0);
                    }
                    current = lex_anal_pm( current, token, &token_type ); // get identifier
                }
                else  // IDENTIFER but no '.' after it
                {
                    if(Search_TB(Select, token, new_attr_tb) == 0)
                    {
                        printf("\t Error: Attribute \"%s\" not found in tables!\n", token);
                        return(0);
                    }
                    j=0;
                    while( new_attr_tb[j] != '\0')
                    {
                        *curr = new_attr_tb[j++];
                        curr++;
                    }
                    *(curr++) = ' ';
                    current = lex_anal_pm( current, token, &token_type ); // get identifier
                }
            }
            else if(token_type == IN) // token_type is IN
            {
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token_type != '(')
                {
                    printf("\t Error: \"(\" expected after IN; not \"%s\"!\n", token);
                    return(0);
                }
                
                current = InSelect(current, Select);
                if(current == NULL)
                {
                    printf("\t Error: in \"IN (...)\"!\n");
                    return(0);
                }
                
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token_type != ')')
                {
                    printf("\t Error: \")\" expected!\n");
                    delete Select;
                    return(0);
                }
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token[0] != '\0')
                {
                    *(curr++) = ' ';
                    *(curr++) = 'A';
                    *(curr++) = 'N';
                    *(curr++) = 'D';
                    *(curr++) = ' ';
                }
            }
            else
            {  // if not identifier / not IN, just copy
                if(token_type == STRING)
                    *(curr++) = '\'';
                j=0;
                while(token[j] != '\0')
                {
                    *curr = token[j++];
                    curr++;
                }
                if(token_type == STRING)
                    *(curr++) = '\'';
                *(curr++) = ' ';
                current = lex_anal_pm( current, token, &token_type ); //get next table name
            }
            // do not forget to lex_anal_pm() in the end
        }
    }
    else
    {/* nothing need */; }
    
    //cout << "Input conditions:\n" << inp_conds << endl;
    //Print_select(Select); // test print Select

    /************* Output Select **************/
    //printf("\t *** SELECT Output ***\n");
    if(ProcessSelect(Select, inp_conds, Select->NRels, 0) == 1)
    {
        for(i=0; i < Select->NAttrs*(3+12)+1; i++)
        {
            printf("%s", "-");
        }
        printf("\n");
        return(1);
    }
    else
    {
        printf("\t Error in ProcessSelect(): SELECT cannot be properly processed!\n");
        return(0);
    }
    
    fclose(fp);
    delete []token;
    delete Select;
    return(1);
  
}
//** End Select_Exec **/


// Initialize the Select object
void Init_Select(struct SELECT_class *Select)
{
    int i, j;
    Select->NAttrs = 0;
    for(i=0; i<32; i++)
    {
        Select->Attr[i] = NULL;
        for(j=0; j<MAX_TOKEN*2; j++)
        {
            Select->AttrName[i][j] = '\0';
        }
    }
    
    Select->NRels = 0;
    for(i=0; i<10; i++)
      Select->Rel[i] = NULL;
    
    Select->where = NULL;
    Select->parent = NULL;
    Select->child = NULL;
}
// End Initialize Select


/**********************************************************************/
/*********************   Process Select    ****************************/
/**********************************************************************/
// Process the select command and print out resutls
int ProcessSelect(struct SELECT_class *s, char *inp_conds, int NRels, int currRel)
{
    int i, j;
    int token_type;
    char token[MAX_TOKEN];
    bool select_flag = 0;
    
    curr = inp_conds; // remember to reset it every time you evaluate!!!! <--- NOTE
    
    
    if(currRel < NRels)
    { // read tuples as long as it is 'Y'
      
        // reset file relation to the beginning
        if ( ( s->Rel[currRel]->fd = open(s->Rel[currRel]->TableName, O_RDONLY)) == -1 )
        {
           cout << "\t Error: canot open relation file \"" << s->Rel[currRel]->TableName 
           << "\"!"<< endl;
           return 0;
        }
        
        while( s->Rel[currRel]->GetNext() != 0 ) // while not the end of a file
        { // read and continue
            if( s->Rel[currRel]->buf[s->Rel[currRel]->record_size] == 'Y')
            {    
                if( ProcessSelect(s, inp_conds, NRels, currRel+1) == 0 )
                {
                    printf("\t Error: ProcessSelect()!\n");
                    return(0);
                }
            }
        }
    }
    else
    { // Evalute(inp_conds, curr); if yes, check attr list and print; if no, do nothing 
        /***  Decide whether to select ***/
        curr = inp_conds;
        lex_anal_pm(curr, token, &token_type);
        if(token[0] == '\0') // no conditions given;
        {
            select_flag = 1;
        }
        else
        {
            s->where = SelBE(s);
            
            if(s->where == NULL)
            {
                printf("\t Error: unable to decide which ones to select!\n");
                delete s;
                return(0);
            }
            select_flag = s->where->eval_b();
            if(select_flag ==0) return(1); // tuples not satifying conds; do nothing; return 1;
        }
        
        /*** Print out selected attributes ***/
        for(i=0; i < s->NAttrs; i++)
        {
            curr = s->AttrName[i];
            s->Attr[i] = SelE(s);
            if(s->Attr[i] == NULL)
            {
                printf("\t Error: unable to evaluate \" %s \"!\n", s->AttrName[i]);
                return(0);
            }
        }
        if(sel_print_flag == 0)
        {
            printf("\n******** SELECT Output ********\n");
            for(i=0; i < s->NAttrs; i++)
            {
                printf("| %-12s ", s->AttrName[i]);
            }
            printf("|\n");
            sel_print_flag = 1;
            for(i=0; i < s->NAttrs*(3+12)+1; i++)
            {
                printf("%s", "-");
            }
            printf("\n");
        }
        
        if( Print_Selected_Attrs(s) == 1)
        {
            return(1);
        }
        else
        {
            printf("\t Error: cannot print values for attr!\n");
        }
        
    }
    return(1);
}
/**********************************************************************/
/*********************   End Process Select    ************************/
/**********************************************************************/


// Print selected Attributes; they already constrcuted parse tree
int Print_Selected_Attrs(struct SELECT_class *s)
{
    int i;
    for ( i=0; i < s->NAttrs; i++)
    {
        
        if( s->Attr[i]->type==INT_CONST || s->Attr[i]->type==INT_ATTR
            || s->Attr[i]->type==ADDII  || s->Attr[i]->type==SUBII
            || s->Attr[i]->type==MULII  || s->Attr[i]->type==DIVII
            || s->Attr[i]->type==POSI   || s->Attr[i]->type==NEGI)
        { // if integer
            printf("| %-12d ", s->Attr[i]->eval_i());
        }
        else if( s->Attr[i]->type==FLOAT_CONST || s->Attr[i]->type==FLOAT_ATTR
                 || s->Attr[i]->type==ADDIF || s->Attr[i]->type==ADDFI || s->Attr[i]->type==ADDFF 
                 || s->Attr[i]->type==SUBIF || s->Attr[i]->type==SUBFI || s->Attr[i]->type==SUBFF 
                 || s->Attr[i]->type==MULIF || s->Attr[i]->type==MULFI || s->Attr[i]->type==MULFF
                 || s->Attr[i]->type==DIVIF || s->Attr[i]->type==DIVFI || s->Attr[i]->type==DIVFF
                 || s->Attr[i]->type==POSF  || s->Attr[i]->type==NEGF)
        { // if float
            printf("| %-12.4f ", s->Attr[i]->eval_f());
        }
        else if( s->Attr[i]->type==STRING_CONST || s->Attr[i]->type==CHAR_ATTR )
        {
            printf("| %-12s ", s->Attr[i]->eval_s());
        }
        else 
        {
            printf("| %-12s ", "Error");
            return(0);
        }
    }
    
    printf("|\n");
    return(1);
       
} 


int Add_TB2Attr(struct SELECT_class *Select, int i)
{
    char *token = new char[MAX_TOKEN];
    char *token_peek = new char[MAX_TOKEN];
    char new_attr_tb[MAX_TOKEN*2];
    char *cur_att = Select->AttrName[i];
    char newcopy[2*MAX_TOKEN];
    int token_type=0, ind=0, j;
    
    for(j=0; j<2*MAX_TOKEN; j++)
        newcopy[j] = '\0';
    
    cur_att = lex_anal_pm( cur_att, token, &token_type ); //get next 
    while(token[0] != '\0')
    {
        if(token_type == 0)
        {
            printf("\t Error: Unknow Token \"%s\"!\n", token);
            delete [] token;
            delete [] token_peek;
            return(0);
        }
        if(token_type == IDENTIFIER)
        {
            //peak
            lex_anal_pm( cur_att, token_peek, &token_type );
            if(token_type == '.') // simple case: already explicit
            {
                j=0;
                while(token[j] != '\0')
                    newcopy[ind++] = token[j++];
                newcopy[ind++] = '.';
                cur_att = lex_anal_pm( cur_att, token, &token_type ); //skip '.'
                if(token_type != '.') printf("\t Error: '.' not '.'!\n");
                cur_att = lex_anal_pm( cur_att, token, &token_type ); //get att name
                if(token_type == IDENTIFIER)
                {
                    j=0;
                    while(token[j] != '\0')
                      newcopy[ind++] = token[j++];
                }
                else
                {
                    printf("\t Error: AttrName Error; expected Attribute Name after '.' instead of \"%s\"!\n", token);
                    delete [] token;
                    delete [] token_peek;
                    return(0);
                }
            }
            else
            {
                if(Search_TB(Select, token, new_attr_tb) == 0)
                {
                    printf("\t Error: Attrname \"%s\"!\n", token);
                    return(0);
                }
                else
                {
                    j=0;
                    while(new_attr_tb[j] != '\0')
                      newcopy[ind++] = new_attr_tb[j++];
                }
            }
        }
        else
        {
            j=0;
            while(token[j] != '\0')
               newcopy[ind++] = token[j++];
        }
        cur_att = lex_anal_pm( cur_att, token, &token_type ); //get next 
    }
    strcpy(Select->AttrName[i], newcopy);
    delete[] token;
    delete[] token_peek;
    return(1);
    
}



/********* Search AttrName in Table; Returns TB.ATTR or alias.ATTR *********/
int Search_TB(struct SELECT_class *Select, char *AttrName, char *new_attr_tb)
{
    int i, j;
    bool found_flag=0;
    for(i=0; i < Select->NRels; i++)
    {
        for(j=0; j < Select->Rel[i]->n_fields; j++)
        {
            if( strcmp( Select->Rel[i]->DataDes[j].fieldname, AttrName) == 0)
            {
                if(found_flag == 0)
                    found_flag = 1;
                else
                {
                    printf("\t Error: \"%s\" is ambiguous!\n", AttrName);
                    return(0);
                }
                
                if(Select->Rel[i]->alias[0]=='\0')
                {
                    strcpy(new_attr_tb, Select->Rel[i]->TableName);
                }
                else
                {
                    strcpy(new_attr_tb, Select->Rel[i]->alias);
                }
                strcat(new_attr_tb, ".");
                strcat(new_attr_tb, AttrName);
            }
        }
    }
    if(found_flag == 0)
    {
        printf("\t Error: Attribute \"%s\" not found!\n", AttrName);
        return(0);
    }
    return(1);
}

// analyze IN conditions
char *InSelect(char *current_pos, struct SELECT_class *Select)
{ 
    char *current = current_pos;
    int token_type, token_type_peek, j=0; 
    char token[MAX_TOKEN], token_peek[MAX_TOKEN];
    char atname[MAX_TOKEN], relname[MAX_TOKEN];
    current = lex_anal_pm( current, token, &token_type ); // get identifier
    if(token_type != SELECT)
    {
        printf("\t Error: \"SELECT\" expected after \"IN (\"; not \"%s\"!\n", token);
        return(NULL);
    }
    current = lex_anal_pm( current, token, &token_type ); // get identifier
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: Attribute name expected after \"SELECT\"!\n");
        return(NULL);
    }
    strcpy( atname, token);
    //cout << "attr:" << atname << endl;
    current = lex_anal_pm( current, token, &token_type ); // get identifier
    if(token_type != FROM)
    {
        printf("\t Error: \"FROM\" expected after attribute; only 1 attribute expected!\n");
        return(NULL);
    }
    
    current = lex_anal_pm( current, token, &token_type ); // get identifier
    if(token_type != IDENTIFIER)
    {
        printf("\t Error: Relation expected after FROM!\n");
        return(NULL);
    }
    strcpy( relname, token);
    //cout << "Relname: " << relname << endl;
    Select->Rel[Select->NRels] = new ScanTable();
    if ( Select->Rel[Select->NRels]->Open(relname) == 0 )
    {
        cout << "\t Error: Relation \"" << relname << "\" not found !" << endl;
        delete Select;
        return(0);
    }
    Select->NRels += 1;
    *(curr++) = ' ';
    *(curr++) = '=';
    *(curr++) = ' ';
    j=0;
    while(relname[j] != '\0')
    {
        *(curr++) = relname[j++];
    }
    *(curr++) = '.';
    j=0;
    while(atname[j] != '\0')
    {
        *(curr++) = atname[j++];
    }
    *(curr++) = ' ';
    *(curr++) = 'A';
    *(curr++) = 'N';
    *(curr++) = 'D';
    *(curr++) = ' ';
    
    lex_anal_pm( current, token, &token_type ); // get identifier
    if(token_type != WHERE && token_type != ')')
    {
        printf("\t Error: WHERE or ')' expected after Relation; NO alias in ATTR IN!\n");
        return(NULL);
    }
    if(token_type == ')')
    { // no condition is given 
        *(--curr) = '\0';
        *(--curr) = '\0';
        *(--curr) = '\0';
        *(--curr) = '\0';
        return(current);
    }
    if(token_type == WHERE)
    {
        current = lex_anal_pm( current, token, &token_type );//skip where
        lex_anal_pm( current, token, &token_type ); //peek condtions
    }
    *(curr++)='(';
    // if identifier, add rel.att; 
    // elif IN, recursive;
    // else copy; 
    while(token_type != ')')  // checking peeked token
    {
        if(token_type == 0)
        {
            printf("\t Error: ')' Missed; \"%s\" not expected!\n", token);
            delete Select;
            return(NULL);
        }
        current = lex_anal_pm( current, token, &token_type ); //pass condtions
        if(token_type == IDENTIFIER)
        {
            lex_anal_pm( current, token_peek, &token_type_peek ); //get next table name
            if(token_type_peek == '.')
            {
                if(strcmp(relname, token) != 0)
                {
                    printf("\t Error: \"%s.\" can only be \"%s.\"!\n", token, relname);
                    delete Select;
                    return(NULL);
                }
                j=0;
                while(token[j] != '\0')
                {
                    *curr = token[j++];
                    curr++;
                }
                *(curr++) = '.';
                current = lex_anal_pm( current, token, &token_type ); // skip '.'
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token_type == IDENTIFIER)
                {
                    j=0;
                    while(token[j] != '\0')
                    {
                        *curr = token[j++];
                        curr++;
                    }
                    *(curr++) = ' ';
                }
                else
                {
                    printf("\t Error: Attribute name expected after \".\" instead of \"%s\"!\n", token);
                    return(0);
                }
                lex_anal_pm( current, token, &token_type ); // get next
            }
            else  // IDENTIFER but no '.' after it--> attribute name
            {
                j=0;
                while(relname[j] != '\0')
                    *(curr++) = relname[j++];
                *(curr++) = '.';
                
                j=0;
                while(token[j] != '\0')
                    *(curr++) = token[j++];
                
                *(curr++) = ' ';
                lex_anal_pm( current, token, &token_type ); // get identifier
            }
        }    
        else if(token_type == IN) // token_type is IN
        {
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token_type != '(')
                {
                    printf("\t Error: \"(\" expected after IN!\n");
                    delete Select;
                    return(NULL);
                }
                current = InSelect(current, Select);
                if(current == NULL)
                {
                    printf("\t Error: in \"IN (...)\"!\n");
                    delete Select;
                    return(NULL);
                }
                current = lex_anal_pm( current, token, &token_type ); // get identifier
                if(token_type != ')')
                {
                    printf("\t Error: \")\" expected!\n");
                    delete Select;
                    return(NULL);
                }
                lex_anal_pm( current, token, &token_type ); // get identifier
        }
        else
        {  // if not identifier / not IN, just copy
                if(token_type == STRING)
                    *(curr++) = '\'';
                j=0;
                while(token[j] != '\0')
                {
                    *curr = token[j++];
                    curr++;
                }
                if(token_type == STRING)
                    *(curr++) = '\'';
                *(curr++) = ' ';
                lex_anal_pm( current, token, &token_type ); //get next table name
        }
            // do not forget to lex_anal_pm() in the end
    }
    *(curr++) = ')';
    return current;
}



// print select class contents
void Print_select(struct SELECT_class *select)
{
    int i, j;
    printf("\n****** Select object ******\n");
    printf("*** Attributes: %d\n", select->NAttrs);
    for(i=0; i<select->NAttrs; i++)
        printf(" %s\n", select->AttrName[i]);
    
    printf("*** Relations: %d\n", select->NRels);
    for(i=0; i<select->NRels; i++)
    {
        if(select->Rel[i] == NULL)
        {
            printf(" %-10s : %-10s\n","None", "None");
        }
        else
        {
            printf(" %-10s :", select->Rel[i]->TableName);
            if(select->Rel[i]->alias[0] == '\0')
                printf(" %-10s\n", "None");
            else
              printf(" %-10s\n", select->Rel[i]->alias);
        }
    }
}






/*
int DoSelect(char *relnames, char *attrnames, char *inp_conds)
{
    struct SELECT Select;
    char *current, *token = new char[MAX_TOKEN], *atptr;
    int seq_ind=0, subind=0, token_type;
    
    current = attrnames;
    current = lex_anal_pm(current, token, &token_type);  // Get next token
    if(token_type == 0)
    {
        printf("\t Error: Syntax error at the very beginning!\n");
        delete []token;
        return(0);
    }
    while (token_type != 0)
    {
        atptr = Select.AttrName[seq_ind];
        while(token[0] != '\0' && token_type != ',')
        {
            strcpy(atptr, token);
            atptr = atptr + strlen(token);
            current = lex_anal_pm(current, token, &token_type);  // Get next token
        }
        if(token_type = ',')
        {
            current = lex_anal_pm(current, token, &token_type);  // Get next token
        }
        seq_ind++;
    }    

    Select.NAttrs = seq_ind;
    
    seq_ind = 0;
    current = relnames;
    current = lex_anal_pm(current, token, &token_type);  // Get next token: TABLE
    if(token_type == 0)
    {
        printf("\t Error: Syntax error at the very beginning!\n");
        delete []token;
        return(0);
    }
    while (token_type != 0)
    {
        if(token_type == IDENTIFIER)
        {
            Select.Rel[seq_ind] = new ScanTable();
            strcpy(Select.Rel[seq_ind]->TableName, token);
            current = lex_anal_pm(current, token, &token_type);  // Get next token: TABLE
            if(token_type == IDENTIFIER)
            {
                strcpy(Select.Rel[seq_ind]->alias, token);
            }
            else if(token_type == ',')
            {
                Select.Rel[seq_ind]->alias[0] = '\0';
            }
            else if(*token == '\0')
            {
                Select.Rel[seq_ind]->alias[0] = '\0';
                seq_ind++;
                break;
            }
            else
            {
                printf("\t Error: Invalid token \"%s\"", token);
                delete [] token;
                return(0);
            }
        }
        else
        {
            printf("\t Error: Invalid relation name:\"%s\"!\n", token);
            delete[] token;
            return(0);
        }
        current = lex_anal_pm(current, token, &token_type);  // Get next token: TABLE
        seq_ind++;    // more tables await
    }
    Select.NRels = seq_ind;
    
    
    

    delete []token;
    return(1);
  
}
*/






















    /*** Test BEGIN: feel free to delete****/
    /*
    cout << " Attributes:" << endl;
    for(subind=0; subind < Select.NAttrs; subind++)
    {
        cout << Select.AttrName[subind] << endl;
    }
    
    cout << " Relations:" << endl;
    for(subind=0; subind < Select.NRels; subind++)
    {
        cout << Select.Rel[subind]->TableName << " \t:";
        if(Select.Rel[subind]->alias[0] == '\0')
        {
            printf("No alias\n");
        }
        else
        {
            printf("%s\n", Select.Rel[subind]->alias);
        }
    }
    /*** Test END: feel free to delete****/
    
