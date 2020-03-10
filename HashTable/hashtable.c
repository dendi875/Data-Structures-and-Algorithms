#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

static int hash_str(char *key);
static void resize_hash_table_if_needed(HashTable *ht);
static int hash_resize(HashTable *ht);

/* 初始化哈希表 */
int hash_int(HashTable *ht)
{
	ht->size = HASH_TABLE_INIT_SIZE;
	ht->elem_num = 0;
	ht->buckets = (Bucket **) calloc(ht->size, sizeof(Bucket *));

	if (ht->buckets == NULL) {
		return FAILED;
	}

	LOG_MSG("[init]\tsize：%d\n", ht->size);

	return SUCCESS;
}

/**
将内容插入到哈希表中
１、以 key 计算哈希，找到元素应该存储的位置，并检查该位置是否已经有了内容，
如果发生冲突则将新元素链接到原有元素链表头部。
2、由于在插入过程中导致哈希的元素个数比较多，如果超过了哈希表的容量，则说明肯定会出现冲突，出现冲突会导致哈希表的性能下降，
为此如果出现元素容量达到哈希表容量则需要进行扩容。由于所有的 key 都进行了哈希，扩容后哈希表不能简单的扩容，而需要重新将原有
已插入的元素重新哈希后插入到新的容器中。
*/
int hash_insert(HashTable *ht, char *key, void *value)
{
	// 检查我们是否需要调整哈希表的大小
	resize_hash_table_if_needed(ht);

	int index = HASH_INDEX(ht, key);

	Bucket *org_bucket = ht->buckets[index];
	Bucket *tmp_bucket = org_bucket;

	// 检查如果 key 已经存在
	while (tmp_bucket) {
		if (strcmp(key, tmp_bucket->key) == 0) {
			LOG_MSG("[update]\tkey：%s\n", key);
			tmp_bucket->value = value;

			return SUCCESS;
		}
		tmp_bucket = tmp_bucket->next;
	}

	Bucket *bucket = (Bucket *)malloc(sizeof(Bucket));

	bucket->key = key;
	bucket->value = value;
	bucket->next = NULL;

	ht->elem_num += 1;

	if (org_bucket != NULL) {
		LOG_MSG("[collision]\tindex：%d key：%s\n", index, key);
		bucket->next = org_bucket;
	}

	ht->buckets[index] = bucket;

	LOG_MSG("[insert]\tindex：%d key：%s\tht(num：%d)\n", index, key, ht->elem_num);

	return SUCCESS;
}


/**
 * 根据 key 查找内容
 */
int hash_lookup(HashTable *ht, char *key, void **result)
{
	int index = HASH_INDEX(ht, key);
	Bucket *bucket = ht->buckets[index]; 

	if (bucket == NULL) {
		goto failed;
	}

	while (bucket) {
		if (strcmp(key, bucket->key) == 0) {
			LOG_MSG("[lookup]\t found %s \tindex：%d value：%p\n", key, index, bucket->value);
			
			*result = bucket->value;
			
			return SUCCESS;
		}
		
		bucket = bucket->next;
	}

failed:
	LOG_MSG("[lookup]\t key：%s\tfailed\t\n", key);

	return FAILED;
}

/**
 * 删除 key 所指向的内容
 */
int hash_remove(HashTable *ht, char *key)
{
	int index = HASH_INDEX(ht, key);
	Bucket *bucket = ht->buckets[index];
	Bucket *prev = NULL;

	if (bucket == NULL) {
		return FAILED;
	}

	/* 从链表中查找正确的 bucket */
	while (bucket) {
		if (strcmp(key, bucket->key) == 0) {
			LOG_MSG("[remove]\tkey：%s index：%d\n", key, index);
			
			if (prev == NULL) {
				ht->buckets[index] = bucket->next;
			} else {
				prev->next = bucket->next;
			}
			free(bucket);

			return SUCCESS;
		}
		
		prev = bucket;
		bucket = bucket->next;
	}

	LOG_MSG("[remvoe]\tkey：%s not found remove \tfailed\t\n", key);

	return FAILED;
}

/**
 * 销毁哈希表
 */
int hash_destroy(HashTable *ht)
{
	int i;
	Bucket *cur = NULL; 
	Bucket *tmp = NULL;

	for (i = 0; i < ht->size; i++) {
		cur = ht->buckets[i];
		while (cur) {
			tmp = cur;
			cur = cur->next;
			free(tmp);
		}
	}

	free(ht->buckets);

	return SUCCESS;
}


/**
 * 哈希函数实现
 *
 * 将 key 字符串的所有字符加起来，然后以结果对哈希表的大小取模，这样索引就能落在数组索引的范围之内了
 */
static int hash_str(char *key)
{
	int hash = 0;
	
	char *cur = key;

	while (*cur != '\0') {
		hash += *cur;
		++cur;
	}

	return hash;
}

/**
 * 如果elem_num几乎与哈希表的容量一样大，我们需要调整哈希表的大小以包含足够的元素
 */
static void resize_hash_table_if_needed(HashTable *ht)
{
	if (ht->elem_num >= ht->size) {
		hash_resize(ht);
	}
}

static int hash_resize(HashTable *ht)
{
	// double the size
	int org_size = ht->size;
	ht->size = ht->size * 2;
	ht->elem_num = 0;

	LOG_MSG("[resize \torg size：%d\tnew size：%d]\n", org_size, ht->size);

	Bucket **buckets= (Bucket **) calloc(ht->size, sizeof(Bucket *));
	Bucket **org_buckets = ht->buckets;

	ht->buckets = buckets;

	int i = 0;

	for (i = 0; i < org_size; i++) {
		Bucket *cur = org_buckets[i];
		Bucket *tmp;

		while (cur) {
			// rehash：insert again
			hash_insert(ht, cur->key, cur->value);

			// free the org bucket, but not the element
			tmp = cur;
			cur = cur->next;
			free(tmp);
		}
	}

	free(org_buckets);

	LOG_MSG("[resize] done\n");

	return SUCCESS;
}


