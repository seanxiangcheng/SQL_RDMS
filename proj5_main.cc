
#include <iostream>
#include <string.h>
#include "lexanal.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
char *curr;
bool sel_print_flag;

int main(int argc, char ** argv)
{
   char inp[MAX_INPUT];  // to store all the entered characters
   char token[MAX_TOKEN];  // to store single token
   int  token_type;  // token_type or token_code defined in lexanal.cc
   int  n;           // number of char entered
   char *current;    // current pointer position of the input string 
   int command;      // store what command was parsed
   int err = 0;      // 0 means error; 1 means no error
   char table_name[MAX_TOKEN];
   
   char attributes[MAX_ATTRIBUTE];  // for create
   char data_types[MAX_TOKEN];      // for create
   int  var_lengths[MAX_TOKEN];     // for create
   
   char inp_conds[MAX_COND];    // for delete and select
                                // store the condition infomation
   
   //char attrnames[MAX_ATTRIBUTE]; // for select
   //char relnames[MAX_RELS];  // for select
   
   
   DataDescription data_des[MAX_FIELD]; // data descriptions
   char buf[1023];   // to store the data of insert
   int record_size = 0;  // tuple size in the unit the byte 
   while (1)
   {
      cout << "cs544SQL >> ";
      n = read_command( inp );                     // Read a command
      current = inp;				                     // Start parsing at p[0]

      while ((current != inp + n) && isspace(*current))
         current++;			                           // Skip space
            
      /* =========================================================
         Parse command
         ========================================================= */
      current = lex_anal( current, token, &token_type );    // Get next token
      if (token_type == CREATE)
      {
         command = CREATE;
         //cout << "\t Creating Table..." << endl;
         err = ParseCreate( current, table_name, attributes, data_types, var_lengths);
         //if(err != 1) cout << "\tTable not Created." << endl;
         // check ParseCreate
         //cout << "\t table name: " << table_name << endl;
         //cout << "\t attributes: " << attributes << endl;
         //cout << "\t data types: " << data_types << endl;
         //
      }
      else if (token_type == DROP)
      {
         command = DROP;
         //cout << "\t Dropping Table..." << endl;
         err = ParseDrop( current, table_name );
         //if(err != 1) cout << "\tTable not Dropped." << endl;
         //cout << "\t table name: " << table_name << endl;
      }
      else if(token_type == INSERT)
      {
         command = INSERT;
         err = ParseInsert(current, table_name, data_des, buf, &record_size);
         //if(err != 1) cout << "\tError: Cannot Parse INSERT command." << endl;
         //cout << "\t table name: " << table_name << endl;
      }
      else if(token_type == DELETE)
      {
         command = DELETE;
         err = ParseDelete(current, table_name, inp_conds);
         curr = inp_conds;
         //cout << "In main, curr:" << (void *)&(*curr) << endl;
         //printf("\t Parsed:%s:End!\n", inp_conds);
         //printf("\t Table name: %s.\n", table_name);
         if(err != 1) cout << "\t Error: Cannot Parse DELETE command." << endl;
      }
      else if(token_type == SELECT)
      {
         command = SELECT;
         sel_print_flag = 0;
         err = Select_Exec(current, inp_conds);
         //cout << "Relnames:\n" << relnames << endl;
         //cout << "Attrnames:\n" << attrnames << endl;
         //cout << "Input conditions:\n" << inp_conds << endl;
         //cout << "\t Message: Command \"SELECT\" under develpment!" << endl;
      }
      else if((token_type == IDENTIFIER)  && (strcmp("print", token)==0 || strcmp("PRINT", token)==0))
      {
         current = lex_anal( current, token, &token_type );    // Get next token
         print_table(token, current);
         err =0;
      }
      /* we will add more commands later*/
      else
      {
         command = ILLEGAL;
         cout << "\t Illegal command: \"" << token << "\""<< endl;
      } 
      
      
      /* =========================================================
         Execute the command using the parse tree
         ========================================================= */
         
      if(err == 1)
      {
         if (command == CREATE)
         {  // update catalog, and create empty file
               err = DoCreate(table_name, attributes, data_types, var_lengths);
               if (err == 1) cout << "\t Table Created Successfully." << endl;
               else cout << "\t Table not Created." << endl;
         }
      
         else if (command == DROP)
         {  // update catalog, and remove file;
               err = DoDrop(table_name);
               if (err == 1) cout << "\t Table Dropped Successfully." << endl;
               else cout << "\t Table not Dropped." << endl;
         }
         else if (command == INSERT)
         {
               err = DoInsert(table_name, buf, record_size);
               if (err == 1) cout << "\t Insertion is Done!" << endl;
               else cout << "\t Insertion is not successful!" << endl;
         }
         else if(command == DELETE)
         {
               //cout << "\t Message: Command \"DELETE\" Execution under development!" << endl;
               err = DoDelete(table_name, inp_conds);
               
               if (err == 1) cout << "\t Delete is Done!" << endl;
               else cout << "\t Delete is not successful!" << endl;
               //return(0);
         }
         else if(command == SELECT)
         {
               //cout << "\t Message: Command \"SELECT\" under development!" << endl;
               //err = DoSelect(relnames, attrnames, inp_conds);
               if (err == 1) cout << "SELECT is Done!\n" << endl;
               else cout << "\t SELECT is not successful!\n" << endl;
               sel_print_flag=0;
               //return(0);
         }
         else if (command == ILLEGAL)
         {  // print error information
            cout << "\t Illegal command: \"" << token << "\""<< endl;
         }
      }
      else if(command == SELECT)
      {
            cout << "\t SELECT is not successful!\n" << endl;
            sel_print_flag=0;
      }
        
   }

   /* END OF THE PROGRAM*/
}

