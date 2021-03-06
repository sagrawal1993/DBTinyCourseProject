
/*
	Code To delete a column from the dataBase file.
*/


#include</home/suraj/dBProject/DBHeader.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
//#define _DEBUG_

void *glbPtrBuff;
int numByteReadBlk[2];
int numByteWrite[2];
//int sizeOfType[8];
int check=0;


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
	
	if(!fpTempDb || !ptrBlkToWrite || !byteToWrite || !numByteToWrite){
		printf("Error In Write To File initial \n");	
		return -1;
	}
	
	if( numByteWrited + numByteToWrite > BLKSIZE){
		int numByte1 = BLKSIZE-numByteWrited;
		memcpy( ptrBlkToWrite[i]+numByteWrited, byteToWrite, numByte1 );
		memcpy( (ptrBlkToWrite[i^1]), byteToWrite+numByte1, numByteToWrite-numByte1);
		fwrite( ptrBlkToWrite[i], 1,BLKSIZE, fpTempDb);
		i^=1;
		numByteWrited = numByteToWrite-numByte1;
		numByteWrite[i]=numByteWrited;
	}
	else{
		if( numByteWrited==BLKSIZE){
			fwrite( ptrBlkToWrite[i],1,BLKSIZE,fpTempDb);
			i^=1;
			numByteWrited=0;
			numByteWrite[i]=0;
		}
		memcpy( ptrBlkToWrite[i]+numByteWrited, byteToWrite, numByteToWrite);
		#ifdef _DEBUG_
			printf("In write Block--%s\n",(char *)ptrBlkToWrite[i]+numByteWrited);
		#endif
		numByteWrited += numByteToWrite;
		numByteWrite[i]=numByteWrited;
	}
	return i;
}


/****************************************************************************************************************************************
*	Objective: Read the byte from the file, such that we access the file in blocks.
		   Each write will read the  byte after the last readed byte.
	@arg1:: the file pointer to the file from which the bytes need to be read.
*	@arg2:: the array of Block(Buffer For accessing the file in Block.)
*	@arg3:: the number of bytes to read from the file.
*	Return :: return the address of the location that will contains this byte.
	 	  return NULL, when there is no byte readed or error occurs. 
*
*
*
******************************************************************************************************************************************/
void* readFromFile(FILE *fpDb, void **ptrBlkToRead, int numByteToRead){
	static int i; //remember the block in which we are currently reading.
	static int numByteReaded; //remember the number of byte readed in the current block till now.
	void *temp;
	
	if(!fpDb || !ptrBlkToRead || !numByteToRead){
		printf("Error in Read From file Initial\n");
		return NULL;
	}
	
	#ifdef _DEBUG_
		printf("byte readed %d, i=%d, byte to read %d\n",numByteReaded,i,numByteToRead);
	#endif
	if(numByteReaded+numByteToRead > numByteReadBlk[i]){
		/*
		* Error Has been occured since the whole block is not covered and yet the record is spanned.
		*/
		if(numByteReadBlk[i]!=BLKSIZE){
			printf("Error in File Writing Down Rows\n");
			return NULL;
			}
		int numByte1=numByteReadBlk[i]-numByteReaded;
		glbPtrBuff=realloc(glbPtrBuff,numByteToRead);
		memcpy(glbPtrBuff, ptrBlkToRead[i]+numByteReaded, numByte1);
		memcpy(glbPtrBuff+numByte1, ptrBlkToRead[i^1], numByteToRead-numByte1);
		numByteReaded = numByteToRead-numByte1;
		numByteReadBlk[i]=fread( ptrBlkToRead[i], 1, BLKSIZE, fpDb);
		i^=1;
		return glbPtrBuff;
	}
	else{
		if( numByteReaded == numByteReadBlk[i]){
		//	fseek( fpDb, 0, SEEK_END);
			numByteReadBlk[i]=fread( ptrBlkToRead[i], 1, BLKSIZE,fpDb);
			i^=1;
			numByteReaded = 0;
		}
		if(numByteReaded +numByteToRead == numByteReadBlk[i]){
			#ifdef _DEBUG_
				printf("File Has Been Readed Successfully\n");
			#endif
			check|=1;
			return (ptrBlkToRead[i]+numByteReaded);
			}
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
	
	char strDbTempFileName[100]="temp",backupFile[104];
	FILE *fpDbFile,*fpDbTempFile;
	void *ptrReadBlk[2], *ptrWriteBlk[2];
	int readBlkOffset;

	DBMETA stcDbMetaData;
	void *temp;
	COLMETA *ptrColMetaData;
	int readedColumn;

	if(!strDbFileName || flag<0 || flag>1){
		printf("Error in initial of delete Column Db\n");
		return -1;
		}
//	printf("%s\n",strDbFileName);
	if( (fpDbFile = fopen(strDbFileName,"r+")) == NULL){
		printf(" Error in opening the dabase File\n");
		return -1;
		}

	//allocate block for reading and writing from disk in block.
	if((ptrReadBlk[0] = malloc(BLKSIZE)) == NULL ){
		printf("Error in allocating first block for read\n");
		return -1;
		}
	if((ptrReadBlk[1] = malloc(BLKSIZE))== NULL){
		printf("Error in allocating second block for read\n");
		return -1;
		}
	
	if((glbPtrBuff=malloc(10))==NULL){
		printf("Error in allocating space to glbPtrBuff\n");
		return -1;
	}		
	//read the first two block of data from the file.	
	numByteReadBlk[0]=fread(ptrReadBlk[0],1,BLKSIZE,fpDbFile);
	numByteReadBlk[1]=fread(ptrReadBlk[1],1,BLKSIZE,fpDbFile);
	
	#ifdef _DEBUG_
	printf("number of byte readed by block 1 is %d and by second is %d\n",numByteReadBlk[0],numByteReadBlk[1]);
	#endif

	if((temp=readFromFile(fpDbFile, ptrReadBlk, DBMETASIZE))==NULL){
		printf("Error Occurs in reading Db Metadata from file\n");
		return -1;
	}
	
	memcpy(&stcDbMetaData,temp,DBMETASIZE);
	readedColumn=0;
	if(!flag){	
		if(!strColName){
			printf("User doesn't give the column name to be deleted.\n"); 
			return -1;
			}
		while( readedColumn < stcDbMetaData.numColumn ){
			ptrColMetaData=(COLMETA *)readFromFile(fpDbFile, ptrReadBlk, COLMETASIZE);
			if( strcmp( strColName, ptrColMetaData->colName)==0){
				if(isPRIMARYKEY(ptrColMetaData)){
					printf("Column That Try to delete is the primary key\n");
					return -1;
					}
				setDIRTY( ptrColMetaData);
				readBlkOffset=stcDbMetaData.sizeColMeta*readedColumn + stcDbMetaData.sizeDbMeta;
				fseek( fpDbFile, readBlkOffset, SEEK_SET);
				if( fwrite(ptrColMetaData,1,stcDbMetaData.sizeColMeta, fpDbFile)<=0){
					printf("error in wring the changed column meta data to the file\n");
					return -1;
					}
				return 1;
			}
			readedColumn++;
		}
	}
	else{
		int *byteToCopy=(int *)malloc(2*(stcDbMetaData.numColumn+1)*sizeof(int));
		int numColToDelete=0;
		int sizeOfRecord=1,numByteToWrite;
		void *ptrRecord;
		int i,index=0,sizeOfType;
		//create the name of the tempory file.
		if( strcat(strDbTempFileName, strDbFileName) == NULL ){
			printf("Error in concatinating name for new Temp file\n");
			return -1;
		}
	
		//create the tempory file.
		if( (fpDbTempFile = fopen(strDbTempFileName,"w+")) == NULL){
			printf("Error in creating the new Temp File\n");
			return -1;
		}	
		
		if((ptrWriteBlk[0] = malloc(BLKSIZE)) == NULL ){
			printf("Error in allocating first writing block\n");
			return -1;
		}
	
		if((ptrWriteBlk[1] = malloc(BLKSIZE))== NULL){
			printf("Error in allocating second writing block\n");
			return -1;
		}
		byteToCopy[0]=0;	//this will leave the first byte. that is the Row header.
		byteToCopy[1]=1;
	
		
		#ifdef _DEBUG_
			fwrite(&stcDbMetaData,1,stcDbMetaData.sizeDbMeta,fpDbTempFile);
		#else
			writeToFile(fpDbTempFile, ptrWriteBlk, &stcDbMetaData, stcDbMetaData.sizeDbMeta);
		#endif

		
		while( readedColumn < stcDbMetaData.numColumn ){
			
			ptrColMetaData=(COLMETA*)readFromFile(fpDbFile, ptrReadBlk, stcDbMetaData.sizeColMeta);
			if((DATATYPE(ptrColMetaData))==5){
				sizeOfType= ((ptrColMetaData->sizeColDataType)&7) + ((ptrColMetaData->sizeColDataType)>>3) + 2;
				#ifdef _DEBUG_
					printf("Float-%d\n",sizeOfType);
				#endif
			}
			else if((DATATYPE(ptrColMetaData))== 4){
				sizeOfType=(ptrColMetaData->sizeColDataType)+1;
				#ifdef _DEBUG_
					printf("Int-%d\n",sizeOfType);
				#endif
			}
			else{
				switch(DATATYPE(ptrColMetaData)){
					case 0:
						sizeOfType=1;
						break;
					case 1:
						sizeOfType=8;
						break;
					case 2:
						sizeOfType=6;
						break;
					case 3:
						sizeOfType=ptrColMetaData->sizeColDataType;
						break;
					default:
						printf("Error\n");
						return -1;
					}
				#ifdef _DEBUG_
					printf("Other-%d\n",sizeOfType);
				#endif
			}
			if(isDIRTYCOLUMN((ptrColMetaData))){
				numColToDelete++;
				byteToCopy[numColToDelete<<1]=byteToCopy[( (numColToDelete-1)<<1)|1]+sizeOfType;
				byteToCopy[(numColToDelete<<1)|1]=byteToCopy[numColToDelete<<1];
			}
			else{
				byteToCopy[(numColToDelete<<1)|1] +=sizeOfType;
				
				
			#ifdef _DEBUG_
				fwrite(ptrColMetaData,1,stcDbMetaData.sizeColMeta,fpDbTempFile);
			#else
				writeToFile(fpDbTempFile, ptrWriteBlk, ptrColMetaData, stcDbMetaData.sizeColMeta);
			#endif	
			
			}
			sizeOfRecord+=sizeOfType;
			readedColumn++;
		}
		
		#ifdef _DEBUG_
			printf("%d\n",sizeOfRecord);
		#endif

		if(!numColToDelete){
			printf("There is not column to delete permanently\n");
			return -1;
		}
		while(check!=1 && (ptrRecord=readFromFile(fpDbFile, ptrReadBlk, sizeOfRecord))!=NULL){
			
			#ifdef _DEBUG_
				printf("%s\n",ptrRecord);
			#endif

			for(i=0;i<=numColToDelete;i++){//for each record 
					
				numByteToWrite=byteToCopy[(i<<1)|1]-byteToCopy[i<<1];
				if(numByteToWrite){
					#ifdef _DEBUG_
						printf("byteToCopy[i<<1]-%d, %d\n",byteToCopy[i<<1],byteToCopy[(i<<1)|1]);
						printf("%s\n",ptrRecord+byteToCopy[i<<1]);		
						fwrite(ptrRecord+byteToCopy[i<<1],1,numByteToWrite,fpDbTempFile);
					#else
						index=writeToFile(fpDbTempFile, ptrWriteBlk, ptrRecord+byteToCopy[i<<1],numByteToWrite);
					#endif		
				}
			}
		}		
		
		#ifdef _DEBUG_
			printf("index-%d,numByteWrite-%d",index,numByteWrite[index]);
		#endif

		fwrite(ptrWriteBlk[index],1 ,numByteWrite[index],fpDbTempFile);
		fseek(fpDbTempFile,0,SEEK_SET);
		stcDbMetaData.numColumn -= numColToDelete;
		fwrite(&stcDbMetaData,1,stcDbMetaData.sizeDbMeta,fpDbTempFile);
		fclose(fpDbTempFile);
	}
	fclose(fpDbFile);

	//make the previous file the backup file.This will be the hidden file in the linux system.
	backupFile[0]='.';
	strcpy(backupFile+1,strDbFileName);
	strcat(backupFile,"Backup");
	rename(strDbFileName,backupFile);
	rename(strDbTempFileName,strDbFileName);
	return 1;
}

int main(){
	
	char colName[100],fileName[100];
	int flag;
	
	printf("Enter the Table Name for deletion\t");
	scanf("%s",fileName);
	printf("Menu:\n\tPress 1 permanently delete Virtually deleted columns\n\tPress 2 for Deleting a new column\n\t");
	scanf("%d",&flag);
	if(flag==1){
		if(deleteColDb(" ",fileName,1)==-1){
			printf("Not Successful\n");
		}
	}
	else if(flag ==2){
		printf("Enter the Column Name to Delete\t");
		scanf("%s",colName);
		if(deleteColDb(colName,fileName,0)==-1)
			printf("Not Successful\n");
	}
	return 0;
}
