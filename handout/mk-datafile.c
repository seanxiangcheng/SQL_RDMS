#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

/* ======================================================================
   Variable for the information about the structure of the data file
   *** These are called "Meta information" - information about 
   the information that we want to store....
   ====================================================================== */
struct DataDescription
{
   char relname[30];
   char fieldname[30];
   char fieldtype;
   int  startpos;
   int  fieldsize;
};

int main( int argc, char *argv[])
{
   FILE *fp;	/* Used to access the catalog */
   int   fd;    /* Used to access the relation */

   /* ----------------------------------------------------------------
      Variables to hold the description of the data  - max 10 fields
      ---------------------------------------------------------------- */
   struct DataDescription DataDes[10];	/* Holds data descriptions
					   for upto 10 fields */
   int  n_fields;			/* Actual # fields */
   int  record_size = 0;

   /* ------------------------------------------------------
      Variables to hold the data  - max 10 fields....
      ------------------------------------------------------ */
   char buf[1000];      /* For input data */

   int  i, j;		/* Auxiliary variables */
   char ans;		/* Another auxiliary variable */

   if ( argc < 2 )
   {
      printf("Usage: %s relation-name\n\n", argv[0]);
      exit(1);
   }


   printf("\nProgram for inserting data into a mini database:\n");

   /* ---------------------------
      Read in meta information
      --------------------------- */
   fp = fopen("catalog", "r");

   n_fields = 0;
   while ( fscanf(fp, "%s %s %c %d %d", 
		DataDes[n_fields].relname,
		DataDes[n_fields].fieldname,
		&DataDes[n_fields].fieldtype, 
		&DataDes[n_fields].startpos,
		&DataDes[n_fields].fieldsize) > 0 )
   {
      if ( strcmp( DataDes[n_fields].relname, argv[1] ) == 0 )
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

   if ( n_fields == 0 )
   {
      printf("Relation `%s' not found !\n", argv[1] );
      exit(1);
   }

   /* ------------------------
      Prints meta information
      ------------------------ */
   printf("The relation contains these attributes:\n\n");
   for (i = 0; i < n_fields; i++)
   {
      printf("Fieldname `%s.%s': type = %c, startpos= %d, size = %d\n", 
		DataDes[i].relname, DataDes[i].fieldname, 
		DataDes[i].fieldtype, DataDes[i].startpos, 
		DataDes[i].fieldsize);
   }
   printf("\n----------------------------------------\n\n");

   /* ---------------------------------------------------------------
      Create file to hold data
      --------------------------------------------------------------- */
   fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0666);

   /* ---------------------------------------------------------------
      Enter the data....
      --------------------------------------------------------------- */
   printf("Creating the relation....\n");
   i = 1;
   while (1)
   {  /* ------------------------------------
	 Prompt user to enter each field....
	 ------------------------------------ */
      for (j = 0; j < n_fields; j++)
      {  
//	 printf("Record #%d, field `%s': ", i, DataDes[j].fieldname);

	 if ( DataDes[j].fieldtype == 'I' )
	 {
            i = scanf("%d", &buf[ DataDes[j].startpos] ); 
						/* Use integer encoding */
//	    getchar();		// read away \n
	 }
	 else if ( DataDes[j].fieldtype == 'F' )
	 {
            i = scanf("%lf", &buf[ DataDes[j].startpos] ); 
						/* Use double encoding */
//	    getchar();		// read away \n
	 }
	 else
	 {
            i = scanf("%s", &buf[ DataDes[j].startpos] ); 
						/* Use ASCII encoding */
//	    getchar();		// read away \n
	 }
      }


      if ( i <= 0 )
      {
         printf("DONE....\n");
	 close(fd);
	 exit(0);
      }

      char F[100];

      scanf("%s", F);
      buf[record_size] = F[0];	// Valid

      /* -------------------------------------
	 Write the data to output file 
	 ------------------------------------- */
      write(fd, buf, record_size+1);

      while ( getchar() != '\n' );

   }

}

