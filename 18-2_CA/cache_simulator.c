/*
 * Author      : 201716422 이명규 _ MyungKyu Lee
 * Date        : 2018.12.19
 * Description : Cache Simulator 
*/

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>

int tag, indx, offset;
int cacheSize, blockSize, assoc;   
void lruCtrl(int**,int,int);
int main(int argc, char* argv[])
{
     //////////////                 input error controll                       //////////
    //                                                                                 //
    // Input format : chsm (1_TraceFile) (2_CacheSize) (3_BlockSize) (4_Associativity) //
    //                                                                                 //
     ///////////////////////////////////////////////////////////////////////////////////
   
    FILE* trfp;

    //argument number error
    if(5!=argc)                              
    {
        perror("argc");
        exit(1);
    }

    trfp = fopen(argv[1],"r");
    //input file error
    if(NULL == trfp)                        
    {
        perror("Input- memory trace file");
        exit(1);
    }

    blockSize = atoi(argv[3]);
    //input value (block size) error  
    if((log2(blockSize) - floor(log2(blockSize)))!=0)   
    {
        perror("input: Block size");
        exit(1);
    }

    assoc = atoi(argv[4]);
    //input value (Associativity) error
    if(assoc!=1 && assoc!=2 && assoc!= 4 && assoc!=8)   
    {
        perror("input: Associativity");
        exit(1);
    }

    cacheSize=atoi(argv[2]);                        //input value(cacheSize) error
    if(0 != cacheSize%(blockSize*assoc))
    {
        perror("input: Cache size");
        exit(1);
    }
    ////////////////////////////////////////////End Of Input Error Controll///////////////////////////


   //////////////////////////////////////////////////////////////////////////////////////////////////
   ////                              Main Algorithm of Cache Simulator Implement                /////
   ////                                                                                          ////
   //////////////////////////////////////////////////////////////////////////////////////////////////
    
    // About T I O BreakDown( # of bits )
    offset=log2(blockSize);
    indx=log2(cacheSize)-log2(blockSize)-log2(assoc);
    tag=32-offset-indx;

    char addr[30];
    char tmp1[5], tmp2[5];
    int tagValue,idxValue,offValue;
    int total_access=0,hit=0,comiss=0,camiss=0;
    unsigned int iaddr;
    memset(addr,0,30);
    //Address Breakdown -  (tmp1 addr tmp2)//
    printf("%d %d %d\n",tag,indx,offset);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //          cache[set(indx)][slots]             = tag value
    //          cache[set][slots] ==-1              means: invalid slot 
    //          LRU[set][slot]  LRU[set][0] means : LRU value correspond to set
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    int** cache= (int**)malloc(sizeof(int*)*(int)pow(2,indx));
    int** LRU = (int**)malloc(sizeof(int*)*pow(2,indx));
    memset(cache,0,sizeof(cache));
    memset(LRU,0,sizeof(LRU));

    for(int i=0;i<(int)pow(2,indx);i++)
    {
        cache[i] = (int*)malloc(sizeof(int)*assoc);
        LRU[i] = (int*)malloc(sizeof(int)*assoc);
        memset(cache[i],-1,sizeof(int)*assoc);
        memset(LRU[i],-1,sizeof(int)*assoc); 
    }


    while(fscanf(trfp,"%s %x %s",tmp1,&iaddr,tmp2)==3)
    {
        tagValue=iaddr>>(indx+offset);

        offValue=(iaddr<<(indx+tag))>>(indx+tag);

        idxValue= (iaddr<<tag)>>(tag+offset);
      
        for(int i=0;i<assoc;i++)
        {
            if(cache[idxValue][i]==tagValue)        //hit
            {
                hit++;
                //lru control
                lruCtrl(LRU,i,idxValue);
                break;
            }else if(cache[idxValue][i]==-1)        //compulsory miss
            {
                comiss++;
                cache[idxValue][i]=tagValue;
                //lru control
                lruCtrl(LRU,i,idxValue);
                break;
            }else if(i==assoc-1)                          //capacity miss
            {
                camiss++;
                //lru control
                cache[idxValue][LRU[idxValue][0]]=tagValue;
                lruCtrl(LRU,LRU[idxValue][0],idxValue);         //LRU 에 접근하여, LRU에 해당하는 비트를 MRU로 업데이트
            }
        }    
        total_access++;
    }    
    
    for(int i=0;i<(int)pow(2,indx);i++)
    {
         free(LRU[i]);
         free(cache[i]);
    }
   
    printf("   Tag : %d\n",tag);
    printf(" Index : %d\n",indx);
    printf("Offset : %d\n",offset);
    printf("Result : total_access_%d , hit_%d, hitrate_%.2f\n",total_access,hit,(float)hit/total_access);
   
    free(LRU);
    free(cache);
    fclose(trfp); 
    
}



////////////////////////     Least Recently Used(LRU) Implement   //////////////////////////////////////////
/////// Description : 가장 나중에 사용된 블록을 LRU 배열의 가장 앞에 위치시킨다.
//////                블록에 접근할 때 마다, 해당 블록의 인덱스를 맨 뒤로 정렬시켜, LRU가 가장 앞에 위치하게 된다.
void lruCtrl(int** LRU, int value, int idxValue)            //value 는 assoc 의 인덱스
{
    int lastIdx;
    for(int i=0;i<assoc;i++)
    {
        //////////          Test Code of LRU_CONTROLL       ////////////////////    
        // if(idxValue==3)printf("%d   ",value);                
        // for(int j=0;j<assoc && idxValue==3;j++)
        // {
        //      printf("%d ",LRU[idxValue][j]);
        // }if(idxValue==3)
        // {
        //     printf("\n");
        //     sleep(1);
        // }

        if(LRU[idxValue][i] ==-1)                               // LRU에 값이 처음 들어오면 그 자리에 바로 값 삽입
        {
            LRU[idxValue][i]=value;
            break;
        }else if(LRU[idxValue][i]==value)                       //assoc 값을 찾았을 경우 맨 뒤로 이동시켜준다.  
        {
            if(LRU[idxValue][i+1]==-1 || i==assoc-1)break;                                  //맨 뒤의 assoc에 접근했을 경우 그대로 둡니다.
            for(int j=i;j<assoc-1 && LRU[idxValue][j+1]!=-1;j++)
            {
                LRU[idxValue][j]=LRU[idxValue][j+1];
                lastIdx=j+1;
            }

            LRU[idxValue][lastIdx]=value;
            
            break;
        }
           
    }
}
