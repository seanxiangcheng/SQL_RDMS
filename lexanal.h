#include <stdio.h>
/* ===================================================================
   These are the SYMBOLIC CONSTANTS of the token codes 
   for the tokens return by the lexical analyzer
   =================================================================== */
#define MAX_INPUT 2023          // max number of chars of 1 input
#define MAX_COND 1023
#define MAX_TOKEN 63           // max number of chars in one token
#define MAX_LINE 512             // max number of chars in one line
#define MAX_ATTRIBUTE 1024       // max number of total chars in all attributes
#define MAX_RELS 1024
#define ILLEGAL 32767            // code of illegal token
#define CATALOG_FILE "catalog"   // catalog filename
#define MAX_FIELD 32
#define MAX_BUF 512

extern char *curr;               // current pointer to the conditions commands
extern bool sel_print_flag;
struct DataDescription     // for insert 
{
   char relname[31];
   char fieldname[31];
   char fieldtype;
   int  startpos;
   int  fieldsize;
};


enum Token
{
   IDENTIFIER = 128,
   INTEGER_NUMBER,
   FLOAT_NUMBER,
   STRING,
   // Use the ASCII code for the single char tokens:
   //    , ( ) [ ] . : ; * + - / < > = "
   LESSOREQ, GREATEROREQ, NOTEQ,   // <=   >=  !=
   ALL, AND, ANY, AS, AVG, BETWEEN, BY, CHAR,
   CHECK, CLOSE, COMMIT, COUNT, CREATE, DECIMAL, DELETE, DISTINCT,
   DOUBLE, DROP, EXISTS, FLOAT, FROM, GO, GROUP, HAVING,
   IN, INSERT, INT, INTO, IS, LIKE, MAX, MIN,
   NOT, 
   NULL0,      // Because NULL is already defined in C, we use NULL0 
   NUMERIC, OF, ON, OR, ORDER, PRIMARY,
   REAL, SCHEMA, SELECT, SET, SOME, SUM, TABLE, TO,
   UNION, UNIQUE, UPDATE, USER, VALUES, VIEW, WHERE, WITH
};

enum TYPE
{
      INT_CONST = 1, FLOAT_CONST, STRING_CONST,  // constants
      INT_ATTR, FLOAT_ATTR, CHAR_ATTR,       // attribute vlues
      AND_B, OR_B, NOT_B,                          // boolean operators
      
      EXP_IB, EXP_FB,
      EQII, EQIF, EQFI, EQFF, EQSS,          // = compare equal of 2 experssion
      NEII, NEIF, NEFI, NEFF, NESS,
      LTII, LTIF, LTFI, LTFF, LTSS,          // < LTSS is not valid
      LEII, LEIF, LEFI, LEFF, LESS,          // <=
      GTII, GTIF, GTFI, GTFF, GTSS,          // >
      GEII, GEIF, GEFI, GEFF, GESS,          // >=
      
      POSI, POSF, POSC, // POSC is not valid
      NEGI, NEGF, NEGC, // NEGC is not valid
      ADDII, ADDIF, ADDFI, ADDFF, ADDSS,     // ADDSS is not valid
      SUBII, SUBIF, SUBFI, SUBFF, SUBSS,     // SUBSS is not valid
      MULII, MULIF, MULFI, MULFF, MULSS,     // MULSS is not valid
      DIVII, DIVIF, DIVFI, DIVFF, DIVSS      // DIVSS is not valid
};

class Node
{
 public:
    int type;
    // 1:int; 2:float; 3:char;
    union
    {
        int i;
        double f;
        char c[MAX_TOKEN];
        
        int *i_attr;
        double *f_attr;
        char * c_attr;
        
        struct Node *p[2];  
        // points to operands of a binary arithmetric, relational
        // or logic operation;
        struct Node *q;
        //
    } value;
    
    double eval_f();
    int eval_i();
    char* eval_s();
    bool eval_b();
};

class ScanTable
{
   public:
      int  fd;
      char TableName[MAX_TOKEN];		// Name of the relation
      char alias[MAX_TOKEN];
      FILE *fp;

      DataDescription DataDes[MAX_FIELD];  /* Holds data descriptions
                                              for upto 10 fields */
      int  n_fields;                       /* Actual # fields */
      int  record_size;

      char buf[MAX_BUF];			// Buffer to store tuples

      int Open(char *relname);
      int GetNext();
      void Close();

      int getAttrType( char *attrName );	// Get data type of attr attrName
      int getAttrSize( char *attrName );	// Get size of attr attrName
      void *getAttrPtr( char *attrName );	// Get pointer to attrName in buf[1000]

      void PrintTuple();
      void PrintRelationInfo();
};

class DeleteCmd
{
    ScanTable *R;
    Node *BE;
};

class SELECT_class
{
   public:
       int  NAttrs;             // Number of attrs                    
       char AttrName[32][MAX_TOKEN*2];  // Attribute name; Max number of attributes 64; max attribute length 32;
       Node *Attr[32];	       // Buffer in ScanTable to get the attr value
       int  NRels;              // Number of relations
       //char RelName[16][32];    // Relation name (optional, you can use
                                 // the TableName[ ] field in the ScanTable
       // to store the Relation Name...
       ScanTable *Rel[10];      // ScanTable object for the relation
       
       Node *where;             // Where clause
       
       //void ProcessSelect(int currRel);
       
       /* =========================================
       The following 2 variables are used by 
       the optional part of this project
       ========================================= */    
       SELECT_class *parent;       // Parent and Child are used to
       SELECT_class *child;	    // implement nested queries

};


int read_command(char *input);
char *lex_anal(char *current_pos, char *token, int *token_type);
char *lex_anal_pm(char *current_pos, char *token, int *token_type);      

/* CREATE */
int ParseCreate(char *current_pos, char *table_name, char *attributes, char *data_types, int *var_lengths);
int DoCreate(char *table_name, char *attributes, char *data_types, int *var_lengths);

/* DROP */
int ParseDrop(char *current_pos, char *table_name);
int DoDrop(char *table_name);

/* INSERT */
int ParseInsert(char *current, char *table_name, struct DataDescription *data_des, char *buf, int *record_size);
int DoInsert(char *table_name, char *buf, int record_size);


/* DELETE */
int ParseDelete(char *current, char *table_name, char *inp_conds);
int DoDelete(char *table_name, char *inp_conds);


/* SELECT */
int Select_Exec(char *current, char *inp_conds);
void Init_Select(struct SELECT_class *Select);
int ProcessSelect(struct SELECT_class *Select, char *inp_conds, int NRels, int currRel);
void Print_select(struct SELECT_class *Select);
int Add_TB2Attr(struct SELECT_class *Select, int i);
int Search_TB(struct SELECT_class *Select, char *AttrName, char *new_attr_tb);
char *InSelect(char *current, struct SELECT_class *Select);
int Print_Selected_Attrs(struct SELECT_class *s);


/* Expression evaluation functions for DELETE */
struct Node *BE(struct ScanTable *f);
struct Node *BT(struct ScanTable *f);
struct Node *BF(struct ScanTable *f);
struct Node *E(struct ScanTable *f);
struct Node *T(struct ScanTable *f);
struct Node *F(struct ScanTable *f);

/* Expression evaluation functions for SELECT */
struct Node *SelBE(struct SELECT_class *s);
struct Node *SelBT(struct SELECT_class *s);
struct Node *SelBF(struct SELECT_class *s);
struct Node *SelE(struct SELECT_class *s);
struct Node *SelT(struct SELECT_class *s);
struct Node *SelF(struct SELECT_class *s);

/* Convenience functions */
int print_F(struct Node *p); // temp function
int print_table(char *relname, char *current); // print relation tuples
