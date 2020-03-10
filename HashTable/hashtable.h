#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#define HASH_TABLE_INIT_SIZE 6
#define HASH_INDEX(ht, key) ((hash_str(key)) % (ht->size))  	

#if defined(DEBUG)
#define LOG_MSG printf	
#else
#define	LOG_MSG(...)	
#endif

#define SUCCESS 0
#define FAILED -1

/* 单向链表，用来解决多个 key 哈希冲突问题，使用的是链接法，当多个 key 映射到同一个 index 时，将冲突的元素链接起来 */
typedef struct _Bucket {
	char *key;
	void *value;
	struct _Bucket *next;
} Bucket;


/* 用来保存哈希表本身 */
typedef struct _HashTable {
	int size;			/* 哈希表大小 */
	int elem_num;		/* 已经保存的元素个数 */
	Bucket **buckets;	/* buckets 是一个 Bucket 结构体指针数组 */
} HashTable;


/**
 * 为了操作哈希表，定义几个操作接口函数
 */
int hash_int(HashTable *ht);
int hash_lookup(HashTable *ht, char *key, void **result);
int hash_insert(HashTable *ht, char *key, void *value);
int hash_remove(HashTable *ht, char *key);
int hash_destroy(HashTable *ht);

#endif
