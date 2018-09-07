
/*
	Code To delete a column from the dataBase file.
*/


#include</home/suraj/dBProject/DBHeader.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/*
	@arg1:: string of the name of the column.
	@arg2:: string of the database file name in which the column need to delete.
	@arg3:: flag to tell which option of deletion is being choosen.
		flag=0 , if need to set reset the dirty bit.
		flag=1 , delete all the data related to all the dirty column.
	return -1 if any error occrs 
		1 if succesful.

*/

void *glbPtrBuff;
int numByteReadBlk[2];
int sizeOfType[8];



/***********************************************************************************************************************************************
*	Objective: Write the byte to the file such that each time a block is being written to the disk. 
*	@arg1:: filePointer to the file in which the bytes need to be write.
*	@arg2:: array of block in the memory at which we buffer the containt to write.
*	@arg3:: Address of the locatioin  of byte to write to the file.
*	@arg4:: Number of byte that need to be Written in the file.
*	Return: The index of the block at which write occers. 
*		In case of overlapping it will return the block index at which last byte is beign written. 
***********************************************************************************************************************************************/

int writeToFile(FILE *fpTempDb, void **ptrBlkToWrite, void *byteToWrite, int numByteToWrite){

	static int i;	//remember the block in which byte need to be added.
	static int numByteWrited;	//remember the number of byte readed till this time.
	
	if(!fpTempDb || !ptrBlkToWrite || !byteToWrite || !numByteToWrite)
		return -1;
	
	if( numByteWrited + numByteToWrite > BLKSIZE){
		int numByte1 = BLKSIZE-numByteWrited;
		memcpy( ptrBlkToWrite[i]+numByteWrited, byteToWrite, numByte1 );
		memcpy( (ptrBlkToWrite[i^1]), byteToWrite+numByte1, numByteToWrite-numByte1);
		fseek( fpTempDb,0,SEEK_END);
		fwrite( ptrBlkToWrite[i], BLKSIZE, 1, fpTempDb);
		i^=1;
		numByteWrited = numByteToWrite-numByte1;
	}
	else{
		if( numByteWrited==BLKSIZE){
			fseek( fpTempDb,0,SEEK_END);
			fwrite( ptrBlkToWrite[i],BLKSIZE,1,fpTempDb);
			i^=1;
			numByteWrited=0;
		}
		memcpy( ptrBlkToWrite[i]+numByteWrited, byteToWrite, numByteToWrite);
		numByteWrited += numByteToWrite;
	}
	return i;
}

void* readFromFile(FILE *fpDb, void **ptrBlkToRead, int numByteToRead){
	static int i; //remember the block in which we are currently reading.
	static int numByteReaded; //remember the number of byte readed in the current block till now.
	void *temp;
	
	if(!fpDb || !ptrBlkToRead || !numByteToRead)
		return NULL;
	
	if(numByteReaded+numByteToRead > numByteReadBlk[i]){
		/*
		* Error Has been occured since the whole block is not covered and yet the record is spanned.
		*/
		if(numByteReadBlk[i]!=BLKSIZE)
			return NULL;
		int numByte1=numByteReadBlk[i]-numByteReaded;
		glbPtrBuff=realloc(glbPtrBuff,numByteToRead);
		memcpy(glbPtrBuff, ptrBlkToRead[i]+numByteReaded, numByte1);
		memcpy(glbPtrBuff+numByte1, ptrBlkToRead[i^1], numByteToRead-numByte1);
		numByteReaded = numByteToRead-numByte1;
		numByteReadBlk[i]=fread( ptrBlkToRead[i], BLKSIZE, 1, fpDb);
		i^=1;
		return glbPtrBuff;
	}
	else{
		if( numByteReaded == numByteReadBlk[i]){
			fseek( fpDb, 0, SEEK_END);
			numByteReadBlk[i]=fread( ptrBlkToRead[i], BLKSIZE, 1,fpDb);
			i^=1;
			numByteReaded = 0;
		}
		if(numByteReadBlk[i]!=BLKSIZE)
			return NULL;
		temp=ptrBlkToRead[i]+numByteReaded;
		numByteReaded+=numByteToRead;
		return temp;
	}
	return NULL;
}




/*******************************************************************************************************************************************************
*	Objective: function to delete the column in the table stored in the given file.
		   Column can be logically deleted or physically deleted.
		flag = 1 :If the column is logically deleted.
		flag = 0 :To delete the column permanently that are logically deleted.
*
	@arg1:: column name that needs to be deleted.
	@arg2:: File name that contains the table.
	@arg3:: flag to decide in which deletion needs to be occured.
*	
	Return:: -1 if the execution is not successful.
*
*
*
*
****************************************************************************************************************************************************/


int deleteColDb(char *strColName, char *strDbFileName, int flag){
	
	char *strDbTempFileName="temp";
	FILE *fpDbFile,*fpDbTempFile;
	void *ptrReadBlk[2], *ptrWriteBlk[2];
	int readBlkOffset;

	DBMETA dbMetaData;
	void *temp;
	COLMETA *ptrColMetaData;
	int readedColumn;

	if(!strDbFileName || flag<0 || flag>1)
		return -1;
	
	if( (fpDbFile = fopen(strDbFileName,"r+")) == NULL)
		return -1;

	//create the name of the tempory file.
	if( strcat(strDbTempFileName, strDbFileName) == NULL )
		return -1;
	
	//create the tempory file.
	if( (fpDbTempFile = fopen(strDbTempFileName,"w+")) == NULL)
		return -1;
		
	//allocate block for reading and writing from disk in block.
	if((ptrReadBlk[0] = malloc(BLKSIZE)) == NULL )
		return -1;
	
	if((ptrReadBlk[0] = malloc(BLKSIZE))== NULL)
		return -1;
	
	if((ptrWriteBlk[0] = malloc(BLKSIZE)) == NULL )
		return -1;
	
	if((ptrWriteBlk[0] = malloc(BLKSIZE))== NULL)
		return -1;
		
	//read the first two block of data from the file.	
	numByteReadBlk[0]=fread(ptrReadBlk[0],sizeof(BLKSIZE),1,fpDbFile);
	numByteReadBlk[1]=fread(ptrReadBlk[1],sizeof(BLKSIZE),1,fpDbFile);
	
	if((temp=readFromFile(fpDbFile, ptrReadBlk, DBMETASIZE))==NULL)
		return -1;
	
	memcpy(&dbMetaData,readFromFile(fpDbFile, ptrReadBlk, DBMETASIZE),DBMETASIZE);
	readedColumn=0;
	if(!flag){	
		if(!strColumnName)
			return -1;
		while( readedColumn < dbMetaData.numColumn ){
			ptrColMetaData=(COLMETA *)readFromFile(fpDbFile, ptrReadBlk, dbMetaData.sizeColMeta);
			if( strcmp( strColName, ptrColMetaData->colName)==0){
				setDIRTY( ptrColMetaData);
				readBlkOffset=dbMetaData.sizeColMeta*readedColumn + dbMetaData.sizeDbMeta;
				fseek( fpDbFile, readBlkOffset, SEEK_SET);
				if( fwrite(ptrColMetaData, dbMetaData.sizeColMeta, 1, fpDbFile)<=0)
					return -1;
				return 1;
			}
			readedColumn++;
		}
	}
	else{
		int *byteToCopy=(int *)malloc(2*(dbMetaData.numColumn+1)*sizeof(int));
		int numColToDelete=0;
		int sizeOfRecord=0,numByteToWrite;
		void *ptrRecord;
		int i,index;
		byteToCopy[0]=0;
		byteToCopy[1]=0;
		writeToFile(fpDbTempFile, ptrWriteBlk, &dbMetaData, dbMetaData.sizeDbMeta);
		while( readedColumn < dbMetaData.numColumn ){
			
			ptrColMetaData=(COLMETA*)readFromFile(fpDbFile, ptrReadBlk, dbMetaData.sizeColMeta);
			if(isDIRTYCOLUMN((ptrColMetaData))){
				numColToDelete++;
				byteToCopy[numColToDelete<<1]=byteToCopy[( (numColToDelete-1)<<1)|1]+sizeOfType[DATATYPE(ptrColMetaData)];
				byteToCopy[(numColToDelete<<1)|1]=byteToCopy[numColToDelete<<1];
			}
			else{
				byteToCopy[(numColToDelete<<1)|1] +=sizeOfType[DATATYPE(ptrColMetaData)];
				writeToFile(fpDbTempFile, ptrWriteBlk, ptrColMetaData, dbMetaData.sizeColMeta);
			}
			sizeOfRecord+=sizeOfType[DATATYPE(ptrColMetaData)];
			readedColumn++;
		}
		while((ptrRecord=readFromFile(fpDbFile, ptrReadBlk, sizeOfRecord))!=NULL){
			for(i=0;i<=numColToDelete;i++){//for each record 
				
				numByteToWrite=byteToCopy[(i<<1)|1]-byteToCopy[i<<1];
				if(numByteToWrite)
					index=writeToFile(fpDbTempFile, ptrReadBlk, ptrRecord+byteToCopy[i<<1],numByteToWrite);
			}
		}		
		fwrite(ptrWriteBlk[index^1],BLKSIZE,1,fpDbTempFile);
		fwrite(ptrWriteBlk[index], BLKSIZE, 1,fpDbTempFile);
	}
	return -1;
}

int main(){
	return 0;
}
