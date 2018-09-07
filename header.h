/* 
 *
 *	Author : Ranjan Mishra
 *	Date   : 17th March 2016
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
	|																		|
	-------------------------------------------------------------------------

	

	
	VERSION(1 BYTE)
	Versioning will be permitted as x.y where x,y are integers
	first 4 bits will represent x and last 4 bits, y.
	e.g 1.4 will be stored as 00010100

	Following datatypes will be permitted
	0-BOOL
	1-DATE
	2-TIME
	3-CHAR(N)
	4-INT
	5-FLOAT(m,n)
	
	COLUMN INFO(1 BYTE)
	Column information byte will be made up following bits
	bit 0 : primary key(0-false 1-true)
	bit 1 :	dirty bit(0-exits in table 1-deleted)
	bit 2-5	reserved for future implemetation
	bit 6-8 datatype (0-bool..4-char)

	For floating numbers FLOAT(m,n)
	The SIZE BYTE will store m,n. First 5 bits will be used for m,
	and last 3 bits for n. e.g. FLOAT(4,5): 00100101 

	time hhmmss[24 hrs]
	date yyyymmdd
 */



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
 	char coumnInfo;
 	char columnName[30];
 	char size;
 	char constraints[5];
 }columnMetadata;


/* Necessary functions which will be useful throughout 
 */


 dbMetadata *getDBMetaData(char *tableName);
 void addDBMetaData(dbMetadata *db, char *tableName);

 int getNumberOfColumns(dbMetadata *db);
 long long getNumberOfRows(dbMetadata *db);
 void printCreateTime(dbMetadata *db);
 void printVersion(dbMetadata *db);



 columnMetadata *getColumnMetaData(char *columnName, char *tableName);
 void addColumnMetaData(char *columnName, char *tableName);

 int isDeleted(columnMetadata *column);
 int isPrimaryIndex(columnMetadata *clomun);


