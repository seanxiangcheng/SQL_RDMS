
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>       /* isnan, sqrt */
#include "lexanal.h"
using namespace std;

/***** Evaluation returns float number ******/
double Node::eval_f()
{
   if( type == FLOAT_CONST) // float constant
      return(value.f);
   else if (type == FLOAT_ATTR) // float attribute
      return((double)*(value.f_attr));
   else if(type == INT_CONST || type == INT_ATTR)
   {
      printf("\t Error: eval_f(): Int is not convertable to Float!\n");
      return(NAN);
   }
   else if( type == STRING_CONST || type == CHAR_ATTR) // string constant
   {
      printf("Error: eval_f(): string cannot be converted to float!\n");
      return(NAN);
   }
   
   /* ADD */
   else if(type == ADDIF)
   {
      return((double)(value.p[0]->eval_i() + value.p[1]->eval_f()));
   }
   else if(type == ADDFI)
   {
      return((double)(value.p[0]->eval_f() + value.p[1]->eval_i()));
   }
   else if(type == ADDFF)
   {
      return((double)(value.p[0]->eval_f() + value.p[1]->eval_f()));
   }
   else if(type == ADDII)
   {
      printf("\t Error: Int+Int is not convertable to Float!\n");
      return(NAN);
   }
   
   /* SUB */
   else if(type == SUBIF)
   {
      return((double)(value.p[0]->eval_i() - value.p[1]->eval_f()));
   }
   else if(type == SUBFI)
   {
      return((double)(value.p[0]->eval_f() - value.p[1]->eval_i()));
   }
   else if(type == SUBFF)
   {
      return((double)(value.p[0]->eval_f() - value.p[1]->eval_f()));
   }
   else if(type == SUBII)
   {
      printf("\t Error: Int+Int is not convertable to Float!\n");
      return(NAN);
   }
   /* MUL */
   else if(type == MULIF)
   {
      return((double)(value.p[0]->eval_i() * value.p[1]->eval_f()));
   }
   else if(type == MULFI)
   {
      return((double)(value.p[0]->eval_f() * value.p[1]->eval_i()));
   }
   else if(type == MULFF)
   {
      return((double)(value.p[0]->eval_f() * value.p[1]->eval_f()));
   }
   else if(type == MULII)
   {
      printf("\t Error: Int*Int is not convertable to Float!\n");
      return(NAN);
   }
   /* DIV */
   else if(type == DIVIF)
   {
      return((double)(value.p[0]->eval_i() / value.p[1]->eval_f()));
   }
   else if(type == DIVFI)
   {
      return((double)(value.p[0]->eval_f() / value.p[1]->eval_i()));
   }
   else if(type == DIVFF)
   {
      return((double)(value.p[0]->eval_f() / value.p[1]->eval_f()));
   }
   else if(type == DIVII)
   {
      printf("\t Error: Int/Int is not convertable to Float!\n");
      return(NAN);
   }
         /* POS */
   else if(type == POSF)
   {
      return(1.0*value.q->eval_f());
   }
   else if(type == POSI)
   {
       printf("\t Error:eval_f():int-POS in float-POS!\n");
       
   }
   else if(type == POSC)
   {
      printf("\t Error:eval_f():+string is invlid!\n");
   }
      /* NEG*/
   else if(type == NEGF)
   {
      return(-1.0*value.q->eval_f());
   }
   else if(type == NEGI)
   {
      printf("\t Error:eval_f():int-POS in float-POS!\n");
         
   }
   else if(type == NEGC)
   {
      printf("\t Error:eval_f():-string is invlid!\n");
   }
   else
   {
      printf("\t Error: Unkonwn type for eval_f(): %d!\n", type);
      return(NAN);
   }
   return(NAN);
}


/***** Evaluation returns int number ******/
int Node::eval_i()
{
      if( type == INT_CONST) // float constant
      return(value.i);
      else if (type == INT_ATTR) // float attribute
         return(*(value.i_attr));
      else if(type == FLOAT_CONST || type == FLOAT_ATTR)
      {
         printf("\t Error: eval_i(): Float is not convertable to int!\n");
      }
      else if( type == STRING_CONST || type == CHAR_ATTR) // string constant
      {
         printf("Error: eval_i(): string cannot be converted to int!\n");
      }
      
      /* ADD */
      else if(type == ADDII)
      {
         return((value.p[0]->eval_i() + value.p[1]->eval_i()));
      }
      else if(type==ADDIF || type==ADDFI || type==ADDFF)
      {
         printf("\t Error: eval_i(): float-add in int-add!\n");
      }
      
      /* SUB */
      else if(type == SUBII)
      {
         return(value.p[0]->eval_i() - value.p[1]->eval_i());
      }
      else if(type==SUBIF || type==SUBFI || type==SUBFF)
      {
         printf("\t Error: eval_i(): float-SUB in int-SUB!\n");
      }
      /* MUL */
      else if(type == MULII)
      {
         return(value.p[0]->eval_i() * value.p[1]->eval_i());
      }
      else if(type==MULIF || type==MULFI || type==MULFF)
      {
         printf("\t Error: eval_i(): float-MUL in int-MUL!\n");
      }
      /* DIV */
      else if(type == DIVII)
      {
         return(value.p[0]->eval_i() / value.p[1]->eval_i());
      }
      else if(type==DIVIF || type==DIVFI || type==DIVFF)
      {
         printf("\t Error: eval_i(): float-DIV in int-DIV!\n");
      }
      
      /* POS */
      else if(type == POSI)
      {
         return(1*value.q->eval_i());
      }
      else if(type == POSF)
      {
         printf("\t Error:eval_i():float-POS in int-POS!\n");
         
      }
      else if(type == POSC)
      {
         printf("\t Error:eval_i():+string is invlid!\n");
      }
      /* NEG*/
      else if(type == NEGI)
      {
         return(-1*value.q->eval_i());
      }
      else if(type == NEGF)
      {
         printf("\t Error:eval_i():float-POS in int-POS!\n");
         
      }
      else if(type == NEGC)
      {
         printf("\t Error:eval_i():-string is invlid!\n");
      }
      else
      {
         printf("\t Error: Unkonwn type for eval_i(): %d!\n", type);
      }
      printf("\t Warning:eval_i():NULL is returned in eval_i()!\n");
      return(0);// it should be nothing
}

/********* Evaluate string ***********/
char* Node::eval_s()
{
      if( type == STRING_CONST) // float constant
         return(value.c);
      else if (type == CHAR_ATTR) // float attribute
         return(value.c_attr);
      else if(type == FLOAT_CONST || type== FLOAT_ATTR || type==INT_CONST || type==INT_ATTR)
      {
         printf("\t Error: eval_s(): number is not convertable to string!\n");
         return(NULL);
      }
      else
      {
         printf("\t Error: eval_s(): unknown type in eval_s()!\n");
         return(NULL);
      }
      return(NULL);
}



/* Evaluate boolean */
bool Node::eval_b()
{
   bool result=0;
/* ! not */   
   if(type == NOT_B)
   {
      result = !(value.q->eval_b());
      return(result);
   }
   
/* || OR */
   else if(type == OR_B)
   {
      result = (value.p[0]->eval_b()) || (value.p[1]->eval_b());
      return(result);   
   }
   
/* && AND */
   else if(type == AND_B)
   {
      result = (value.p[0]->eval_b()) && (value.p[1]->eval_b());
      //printf("p[0]-> %d;", value.p[0]->eval_b());//deleteit
      //printf("p[1]-> %d;\t", value.p[1]->eval_b());//deleteit
      return(result);
   }
/* expression */
   else if(type == EXP_IB)
   {
      result = ( value.p[0]->eval_i() != 0);
      printf("Msg: Arithmetic expression along is FALSE for 0 and TRUE for non-0!\n");
      return(result);
   }
   else if(type == EXP_FB)
   {
      result = ( value.p[0]->eval_f() != 0.0);
      printf("Msg: Arithmetic expression along is FALSE for 0 and TRUE for non-0!\n");
      return(result);
   }
/* < less than*/
   else if(type == LTII)
   {
      result=(value.p[0]->eval_i() < value.p[1]->eval_i());
      return(result);
   }
   else if(type == LTIF)
   {
      result=(value.p[0]->eval_i() < value.p[1]->eval_f());
      return(result);
   }
   else if(type == LTFI)
   {
      result=(value.p[0]->eval_f() < value.p[1]->eval_i());
      return(result);
   }
   else if(type == LTFF)
   {
      result=(value.p[0]->eval_f() < value.p[1]->eval_f());
      return(result);
   }
   else if(type == LTSS)
   {
      result=(strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())<0);
      return(result);
   }
   
/* > greater than*/
   else if(type == GTII)
   {
      result=(value.p[0]->eval_i() > value.p[1]->eval_i());
      return(result);
   }
   else if(type == GTIF)
   {
      result=(value.p[0]->eval_i() > value.p[1]->eval_f());
      return(result);
   }
   else if(type == GTFI)
   {
      result=(value.p[0]->eval_f() > value.p[1]->eval_i());
      return(result);
   }
   else if(type == GTFF)
   {
      result=(value.p[0]->eval_f() > value.p[1]->eval_f());
      return(result);
   }
   else if(type == GTSS)
   {
      result=(strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())>0);
      return(result);
   }
   
/* <= less or equal */
   else if(type == LEII)
   {
      result=(value.p[0]->eval_i() <= value.p[1]->eval_i());
      return(result);
   }
   else if(type == LEIF)
   {
      result=(value.p[0]->eval_i() <= value.p[1]->eval_f());
      return(result);
   }
   else if(type == LEFI)
   {
      result=(value.p[0]->eval_f() <= value.p[1]->eval_i());
      return(result);
   }
   else if(type == LEFF)
   {
      result=(value.p[0]->eval_f() <= value.p[1]->eval_f());
      return(result);
   }
   else if(type == LESS)
   {
      result=(strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())<=0);
      return(result);
   }
   
/* >= greater or equal */
   else if(type == GEII)
   {
      result=(value.p[0]->eval_i() >= value.p[1]->eval_i());
      return(result);
   }
   else if(type == GEIF)
   {
      result=(value.p[0]->eval_i() >= value.p[1]->eval_f());
      return(result);
   }
   else if(type == GEFI)
   {
      result=(value.p[0]->eval_f() >= value.p[1]->eval_i());
      return(result);
   }
   else if(type == GEFF)
   {
      result=(value.p[0]->eval_f() >= value.p[1]->eval_f());
      return(result);
   }
   else if(type == GESS)
   {
      result=(strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())>=0);
      return(result);
   }
   
/* != not equal to */
   else if(type == NEII)
   {
      result=(value.p[0]->eval_i() != value.p[1]->eval_i());
      return(result);
   }
   else if(type == NEIF)
   {
      result=(value.p[0]->eval_i() != value.p[1]->eval_f());
      return(result);
   }
   else if(type == NEFI)
   {
      result=(value.p[0]->eval_f() != value.p[1]->eval_i());
      return(result);
   }
   else if(type == NEFF)
   {
      result=(value.p[0]->eval_f() != value.p[1]->eval_f());
      return(result);
   }
   else if(type == NESS)
   {
      result = (strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())!=0);
      return(result);
   }

/* =  equal to */
   else if(type == EQII)
   {
      result=(value.p[0]->eval_i() == value.p[1]->eval_i());
      return(result);
   }
   else if(type == EQIF)
   {
      result=(value.p[0]->eval_i() == value.p[1]->eval_f());
      return(result);
   }
   else if(type == EQFI)
   {
      result=(value.p[0]->eval_f() == value.p[1]->eval_i());
      return(result);
   }
   else if(type == EQFF)
   {
      result=(value.p[0]->eval_f() == value.p[1]->eval_f());
      return(result);
   }
   else if(type == EQSS)
   {
      result = (strcmp(value.p[0]->eval_s(), value.p[1]->eval_s())==0);
      return(result);
   }
   else
   {
      return(NULL);
   }
   
   return(result);
}



int ScanTable::Open(char *relname)
{
   if ( (fp = fopen("catalog", "r")) == NULL )
   {
      cout << "Error: cannot open database catalog `./catalog'" << endl;
      return 0;
   }

   n_fields = 0;
   record_size = 0;
   
   for(int i=0; i<MAX_TOKEN; i++)
      alias[i] = '\0';
   /* ==================================================
	 Read attribute information on relation "relname"
      ================================================== */
   while ( fscanf(fp, "%s %s %c %d %d",
             DataDes[n_fields].relname, DataDes[n_fields].fieldname,
             &DataDes[n_fields].fieldtype, &DataDes[n_fields].startpos,
             &DataDes[n_fields].fieldsize) > 0 )
   {
      if ( strcmp( DataDes[n_fields].relname, relname ) == 0 )
      {
         if ( DataDes[n_fields].startpos + DataDes[n_fields].fieldsize >
              record_size )
         {
               record_size = DataDes[n_fields].startpos 
                         + DataDes[n_fields].fieldsize;
         }
         n_fields++;
      }
   }

   fclose(fp);

   if ( n_fields > 0 )
   {
      if ( (fd = open(relname, O_RDONLY)) == -1 )
      {
         cout << "\t Error: canot open relation file \"" << relname 
         << "\"!"<< endl;
         return 0;
      }
      strcpy( TableName, relname);
      return 1;
   }
   else
   {
      return 0;		// Relation not found
   }
}

/* ==================================================
   GetNext(): read next tuple

     The tuple is return in this->buf

     return 1 for OK
            0 for error
   ================================================== */
int ScanTable::GetNext()
{ 
   int n;
   if(record_size+1 > MAX_BUF)
   {
      printf("\t The record/tuple size is too big for the buf!\n");
      return(0);
   }
   n = read(fd, buf, record_size+1);

   if (n == record_size+1)
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

void ScanTable::Close()
{
   close(fd);		// Close relation file
}



/* ================================================================
   getAttrType(attrName): Get data type of attr attrName
   ================================================================ */
int ScanTable::getAttrType( char *attrName )
{
   int i;

   for ( i = 0; i < n_fields; i++ )
   {
      if ( strcmp( attrName, DataDes[i].fieldname ) == 0 )
         return DataDes[i].fieldtype;
   }

   return -1;		// Attribute not found...
}



/* ================================================================
   getAttrSize(attrName): Get size of attr attrName
	(Only necessary for 'C' typed attributes)
   ================================================================ */
int ScanTable::getAttrSize( char *attrName )
{
   int i;

   for ( i = 0; i < n_fields; i++ )
   {
      if ( strcmp( attrName, DataDes[i].fieldname ) == 0 )
         return DataDes[i].fieldsize;
   }

   return -1;		// Attribute not found...
}


/* ================================================================
   getAttrPtr(attrName): return the pointer to attribute attrName
			 in buf[ ]
   ================================================================ */
void * ScanTable::getAttrPtr( char *attrName )
{
   int i;

   for ( i = 0; i < n_fields; i++ )
   {
      if ( strcmp( attrName, DataDes[i].fieldname ) == 0 )
         return &buf[ DataDes[i].startpos ];
   }

   return NULL;		// Attribute not found...
}




/* ================================================================
   PrintTuple(): print the tuple in buf[ ]
   ================================================================ */
void ScanTable::PrintTuple()
{
   int j;

   for (j = 0; j < n_fields; j++)
   {
      int *i_ptr;
      double *f_ptr;

      if ( DataDes[j].fieldtype == 'I' )
      {
         i_ptr = (int *) &(buf[ DataDes[j].startpos]) ;
         printf("%-6d ", *i_ptr);
      }
      else if ( DataDes[j].fieldtype == 'F' )
      {
         f_ptr = (double *) &(buf[ DataDes[j].startpos]) ;
         printf("%-10.4f ", *f_ptr );
      }
      else
         printf("%-12s ", &(buf[ DataDes[j].startpos])  );
   }

   printf("Valid flag: %c \n", buf[record_size]);
}

void ScanTable::PrintRelationInfo()
{
   int i;

   printf("\nThe relation contains these fields:\n");
   for (i = 0; i < n_fields; i++)
      cout << DataDes[i].relname << "." << DataDes[i].fieldname <<
		"\ttype = " << DataDes[i].fieldtype <<
		"\tstartpos = " << DataDes[i].startpos <<
		"\tsize = " << DataDes[i].fieldsize << endl;

   cout << endl ;
}


