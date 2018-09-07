/* 
 *
 *	Authors : Ranjan Mishra, Nishant Nikam, Aniruddha Biswas
 *	Date   	: 17th March 2016
 *	
 *	
 *	
	The file layout will be 

	DB Metadata
	-------------------------------------------------------------------------
	|2 	BYTES  	MAGIC NUMBER(AB)                                         
	|1 	BYTE 	VERSION                                                  
	|1 	BYTE  	DB Metadata Size(36)                                     
	|14 BYTES  	DATABASE CREATE TIME (DDMMYYYYHHMMSS)                    
	|1 	BYTE 	NUMBER OF COLUMNS                                        
	|1 	BYTE 	COLUMN METADATA SIZE(VALUE=38)                           
	|8 	BYTES	NUMBER OF ROWS                                          
	|8 	BYTES	RESERVED FOR FUTURE                                      
	------------------------------------------------------------------------- 
	Column Metadata
	-------------------------------------------------------------------------
	|1 BYTES 	INDEX
	|1 BYTE 	COLUMN INFORMATION
	|30 BYTES	COLUMN NAME-case insensitive
	|1 BYTES 	SIZE 				
	|5 BYTES	CONSTRAINTS(future implementation)
	-------------------------------------------------------------------------
								-
								-
								-
								-
	------------------------------------------------------------------------- 
	|Column Metadata 				
	-------------------------------------------------------------------------
	|Data 									
	|	
	|	
	|
	|
	-
	-
	-
	-
    |													
	-------------------------------------------------------------------------

	

	
	VERSION(1 BYTE)
	Versioning will be permitted as x.y where x,y are integers
	first 4 bits will represent x and last 4 bits, y.
	e.g 1.4 will be stored as 00010100

	Following datatypes will be permitted
	0-BOOL
	1-DATE
	2-TIME
	3-INT
	4-FLOAT(m,n)
	5-CHAR(N)
	
	COLUMN INFO(1 BYTE)
	Column information byte will be made up following bits
	bit 0 : primary key(0-false 1-true)
	bit 1 :	dirty bit(0-exits in table 1-deleted)
	bit 2-4	datatype (0-bool,..,4-char)
	bit 5-7 reserved for future implementation

	For floating numbers FLOAT(m,n)
	The SIZE BYTE will store m,n. First 5 bits will be used for m,
	and last 3 bits for n. e.g. FLOAT(4,5): 00100101 


 */

#ifndef _CS2015_DATABASE_HEADER_

#define _CS2015_DATABASE_HEADER_

#define BOOL 0					//00000000
#define DATE 8					//00001000
#define TIME 16					//00010000
#define INT 24					//00011000
#define FLOAT 32				//00100000
#define CHAR 40					//00101000
#define MASK 56					//00111000

#define PK 128					//10000000
#define DIRTY 64				//01000000
#define RESERVED 0 				//00000000

#define DBMSIZE 36
#define COLUMNSIZE 38
#define VERSION_UP 16


// structure def for the db metadata


 typedef struct dbMetaInfo{
 	char magicNumber[2];	
 	char version;
 	char dbMetadataSize;
 	char dbCreateTime[14];
 	char numberOfColumns;
 	char columnMetadataSize;
 	char numberOfRows[8];
 	char futureReserved[8];
 }dbMetadata;

 


 //structure def for column metadata

 typedef struct columnMetaInfo{
 	char index;
 	char columnInfo;
 	char columnName[30];
 	char size;
 	char constraints[5];
 }columnMetadata;




// Necessary functions which will be useful throughout 
 

 /* The def which will be accepted will be of the following form
	The first line will contain the create statement, with tableName at the end
	each subsequent lines will contain the name of the column followed by the datatype
	each line will end with a ','(comma), and the definition terminates with ';'(semicolon)

	example definition

		CREATE TABLE EMPLOYEE
		EMPLOYEE_ID INT,
		SALARY FLOAT(12, 5),
		NAME CHAR(20),
		DATE_OF_JOINING DATE,
		SWIPE_TIME TIME,
		CURRENT_STATUS BOOL;

	Below function expects the ddl file of the table in above format and 
	it will produce the expected table metadata and columns metadata and will 
	store them in the file with same name as table name in the DDL file.
	it will retun 1, in case of success else 0.
	*/
 int createTable(char *def);

 /*
  *
  *	Subroutine for the above createTable, it expects the name of the table and 
  *	number of columns in the table to be created, it created new file 
  *	with the same name as tableName and puts the table metadata in it.
  *
  */

 void addDBMetaData(char *tableName, int numberOfColumns);

 /*
  *
  *	Subroutine for the above createTable function, it expects the name of the table and 
  *	information about the column which is combined colum name and dataype 
  *	it will add the column metadata to the file same name as table name.
  *
  */
 
 void addColumnMetaData(char *tableName, char* columnInfo, int index);





 /* 
  This function will read the table metadata information from file and
  return the table metadata to the caller of the function
  */
 dbMetadata *getDBMetaData(char *tableName);
 


 // Given the db metadata object it returns the number of columns in the table
 int getNumberOfColumns(dbMetadata *db);

 

 // Given the db metadata object, it returns the number of rows in the table present at the moment
 long long unsigned getNumberOfRows(dbMetadata *db);

 

 // Given  the db metadata, it prints the creation time of the table
 void printdbCreateTime(dbMetadata *db);
 
 
 //Given the db metadata it prints the current version of the database
 void printVersion(dbMetadata *db);


 /* 
  This function will read the column metadata information from file(table name) and
  return the column metadata to the caller of the function, if the column exists
  otherwise it will return NULL value, which has to handeled by the caller
  */
 columnMetadata *getColumnMetaData(char *columnName, char *tableName);
 
 

 //Given column metadata it returns whether the column is currently in database or not
 int isDeleted(columnMetadata *column);

 
 //Given the column metadata, it returns whether the column is primary key column or not
 int isPrimaryIndex(columnMetadata *column);


 
#endif
