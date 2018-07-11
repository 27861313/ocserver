#ifndef TEST_HASH_MAP_H
#define TEST_HASH_MAP_H
#include "../common/iavltree.inc"
#include "../common/ihashmap.h"
#include "../common/ithread.h"
#include "../common/iconvert.h"
#include "../common/idatetime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void* itimer_callbackk(uint16_i evtid, void *arg)
{
//printf("xxxxxxxxxx\n");
        return NULL;
}


void run_avltree()
{
    /*
    avl_tree* ptree = NULL;
    uint32_i* a = (uint32_i*)malloc(sizeof(uint32_i));
    *a = 0;
    uint32_i* a1 = (uint32_i*)malloc(sizeof(uint32_i));
    *a1 = 0;
    ptree = insert_tree((void*)a, (void*)a1, ptree);
    uint32_i *b = (uint32_i*)malloc(sizeof(uint32_i));
    *b = 200;
    uint32_i *b1 = (uint32_i*)malloc(sizeof(uint32_i));
    *b1 = 200;
    ptree = insert_tree((void*)b,(void*)b1, ptree);
    uint32_i *c = (uint32_i*)malloc(sizeof(uint32_i));
    *c = 300;
    uint32_i *c1 = (uint32_i*)malloc(sizeof(uint32_i));
    *c1 = 300;
    ptree = insert_tree((void*)c,(void*)c1, ptree);
    uint32_i *d = (uint32_i*)malloc(sizeof(uint32_i));
    *d = 600;
    uint32_i *d1 = (uint32_i*)malloc(sizeof(uint32_i));
    *d1 = 600;
    ptree = insert_tree((void*)d,(void*)d1, ptree);
    uint32_i *e = (uint32_i*)malloc(sizeof(uint32_i));
    *e = 800;
    uint32_i *e1 = (uint32_i*)malloc(sizeof(uint32_i));
    *e1 = 800;
    ptree = insert_tree((void*)e,(void*)e1, ptree);
    uint32_i *f = (uint32_i*)malloc(sizeof(uint32_i));
    *f = 1000;
    uint32_i *f1 = (uint32_i*)malloc(sizeof(uint32_i));
    *f1 = 1000;
    ptree = insert_tree((void*)f,(void*)f1, ptree);
    uint32_i *g = (uint32_i*)malloc(sizeof(uint32_i));
    *g = 1;
 //   ptree = del_node((void*)g, ptree);
    uint32_i *h = (uint32_i*)malloc(sizeof(uint32_i));
    *h = 50;
//    ptree = del_node((void*)h, ptree);
    //ptree = insert_tree(44, ptree);
    //ptree = insert_tree(56, ptree);
    print_tree(ptree);
    */
}

uint32_i ihashcode(void* key)
{
  //return *((uint32_i*)key);   
  return v_touint32(key);   
}

int32_i equals(void* key1, void* key2)
{
    uint32_i k1 = v_touint32(key1);
    uint32_i k2 = v_touint32(key2);
    //uint32_i k1 = 0;
    //uint32_i k2 = 0;
    if (k1 > k2)
    {
        return 1;
    }
    else if(k1 < k2)
    {
        return -1;
    }
    return 0;
}

typedef struct VALDATA valdata;
struct VALDATA
{
	uint32_i refnum;
};

void getkey(void* key, void (*getkey)(void* key, void* rev))
{
}
void addref(void* val)
{
	//((valdata*)val)->refnum++;
}
void delref(void* val)
{
	//((valdata*)val)->refnum--;
}


void *test_hasmap_exec_1(void *arg) //
{
       uint64_i begin = getdida_msec(); 

	iochashmap *lm = (iochashmap *)arg;
	uint32_i number = 1000000;
	for(uint32_i i = 0; i < number; i++)
	{
		// uint32_i* p = (uint32_i*)malloc(sizeof(uint32_i));
		// uint32_i* p1 = (uint32_i*)malloc(sizeof(uint32_i));
		uint32_i a = random() % number;
		uint32_i p = a;
		uint32_i p1 = a;
		iochashmap_put(lm, (void*)p, (void*)p1);
	}
       uint64_i end = getdida_msec(); 
       printf("pth1  insert %d number use time : %d ms\n", number, end - begin);

	return NULL;
}

void *test_hasmap_exec_2(void *arg) //
{
       uint64_i begin = getdida_msec(); 
	uint32_i number = 1000000;
	iochashmap *lm = (iochashmap *)arg;
	for(uint32_i i = 0; i < number; i++)
	{
        int x = random() % 100000;
        //uint32_i* p = (uint32_i*)malloc(sizeof(uint32_i));
        //uint32_i* p1 = (uint32_i*)malloc(sizeof(uint32_i));
        uint32_i p = x;
        uint32_i p1 = x;
        iochashmap_put(lm, (void*)p, (void*)p1);
    }

       uint64_i end = getdida_msec(); 
	printf("pth2  insert %d number use time : %d ms\n", number, end - begin);
    return NULL;
}

void *test_hasmap_exec_3(void *arg) //
{
	uint64_i begin = getdida_msec(); 
	uint32_i number = 1000000;
	iochashmap *lm = (iochashmap *)arg;
	for(uint32_i i = 0; i < number; i++)
	{
		uint32_i a = random() % 100000;
		iochashmap_del(lm, (void*)a);
	}

	uint64_i end = getdida_msec(); 
	printf("pth3  del %d number use time : %d ms\n", number, end - begin);
	return NULL;
}

void *test_hasmap_exec_4(void *arg) //
{
	uint64_i begin = getdida_msec(); 
	uint32_i number = 1000000;
	iochashmap *lm = (iochashmap *)arg;
	for(uint32_i i = 0; i < number; i++)
	{
		uint32_i a = random() % 100000;
		iochashmap_del(lm, (void*)a);
	}

	uint64_i end = getdida_msec(); 
	printf("pth4  del %d number use time : %d ms\n", number, end - begin);
	return NULL;
}

void *test_hasmap_exec_5(void *arg) //
{
	uint64_i begin = getdida_msec(); 
	uint32_i number = 1000000;
	iochashmap *lm = (iochashmap *)arg;
	uint32_i x = 0;
	for(uint32_i i = 0; i < number; i++)
	{
		uint32_i a = random() % 100000;
		if(iochashmap_get(lm, (void*)a) != NULL)
		{
			++x;
		}
	}

	uint64_i end = getdida_msec(); 
	printf("pth5  find %d number and success %d use time : %d ms\n", number, x, end - begin);
	return NULL;
}

void *test_hasmap_exec_6(void *arg) //
{
	uint64_i begin = getdida_msec(); 
	uint32_i number = 1000000;
	iochashmap *lm = (iochashmap *)arg;
	uint32_i x = 0;
	for(uint32_i i = 0; i < number; i++)
	{
		uint32_i a = random() % 100000;
		if(iochashmap_get(lm, (void*)a) != NULL)
		{
			++x;
		}
	}

	uint64_i end = getdida_msec(); 
	printf("pth6  find %d number and success %d use time : %d ms\n", number, x, end - begin);
	return NULL;
}


void test_hashmap()
{
	iochashmap* lmt =  iochashmap_create(200, &ihashcode, &equals, /*&getkey,*/ &addref, &delref);

	iocthread *pth_1 = iocthread_create(NULL, test_hasmap_exec_1, lmt);
	iocthread *pth_2 = iocthread_create(NULL, test_hasmap_exec_2, lmt);
	iocthread *pth_3 = iocthread_create(NULL, test_hasmap_exec_3, lmt);
	iocthread *pth_4 = iocthread_create(NULL, test_hasmap_exec_4, lmt);
	iocthread *pth_5 = iocthread_create(NULL, test_hasmap_exec_5, lmt);
	iocthread *pth_6 = iocthread_create(NULL, test_hasmap_exec_6, lmt);


	for (int i = 0; i < 1000; ++i)
	{
		iocthread_run(pth_1);
		iocthread_run(pth_2);
		iocthread_run(pth_3);
		iocthread_run(pth_4);
		iocthread_run(pth_5);
		iocthread_run(pth_6);
		sleep(3);
	printf("\n\n");
	}
	printf("\n\n");
	//iocprint_tree(lmt->_tbarray->_tree);
	//printf("\nthe hashmap size : %d\n", lmt->_size);
	while (1)
	{
		ISLEEP(100);
	}
}



#endif
