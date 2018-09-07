#include<stdio.h>
struct  test{
	int x;
	int y;
};
main(){
	FILE *fp;
	struct test str1;
	str1.x=1;
	str1.y=2;
	int k;
	fp=fopen("temp.txt","w+");
	if(fwrite(&str1,sizeof(struct test),1,fp)<0){
		printf("error\n");
	}
	if(fseek(fp,4,SEEK_SET)<0)
		printf("error\n");
	if(fread(&k,sizeof(int),1,fp)<0){
		printf("error\n");
	}	
	printf("%d\n",k);
//	printf("%lld\n", sizeof(unsigned long long int));
	return 0;
}
