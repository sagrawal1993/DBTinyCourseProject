/*
	Header file to store the structure and Macros.
	
	DataType-
*/

#ifndef __DBMACRO__
 #define __DBMACRO__

 #define VERSION 16
 #define DBMETASIZE 36
 #define COLMETASIZE 38
 #define BLKSIZE 512
 #define isPRIMARYKEY( ColMeta ) ( ColMeta->flagNdataType & 128 )
 #define isDIRTYCOLUMN( ColMeta ) ( ColMeta->flagNdataType & 64 )
 #define DATATYPE( ColMeta ) ( ColMeta->flagNdataType & 7 )
 #define setDIRTY( ColMeta ) (ColMeta->flagNdataType = ColMeta->flagNdataType | 64)

typedef struct dbMetaData{
	
	char magicNumber[2];
	char version;
	char sizeDbMeta;
	char timeOfCreation[14];
	char numColumn;
	char sizeColMeta;
	char numRows[8];
	char reserved[8];
} DBMETA;

typedef struct colMetaData{
	char index;
	char flagNdataType;	//primarykeyflag(1bit)+dirtybitFlag(1bit)+3-bit free+3-bit(DataType)
	char colName[30];
	char sizeColDataType;	//applicable for only string.
	char constraint[5];
	} COLMETA;

#endif

