#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../trie/trie_node.h"
#include "../hash_table/hash_table.h"
#include "../tools.h"

/*   sdbm hash function from:
    http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash_word(char * str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

int hash_round(unsigned long code, int mod_value)
{
    return (int) (code%mod_value);
}

hash_table * create_hash_table()
{
    hash_table * table = malloc(sizeof(hash_table));

    table->max_breaking = HASH_START_SIZE;
    table->current_breaking = 0;
    table->round = 0;
    table->size = HASH_START_SIZE;
    table->mod_value =HASH_START_SIZE;
    table->buckets = malloc(HASH_START_SIZE *sizeof(hash_bucket));
    int i;
    for(i=0; i<HASH_START_SIZE; i++){
        create_hash_node(&table->buckets[i]);
    }
    return table;
}

void create_hash_node(hash_bucket * node)
{
    node->current_children =0;
    node->max_children = BUCKET_START_SIZE;

    node->children = NULL;
}

trie_node * hash_insert(hash_table * table, char * word, int version)
{
    unsigned long code = hash_word(word);
    int position = hash_round(code, table->mod_value);
    if (position < table->current_breaking)
    {
        position = hash_round(code, table->mod_value *2);
    }
    return hash_bucket_insert(table, position, word, version);
}

trie_node * hash_bucket_insert(hash_table * table, int pos, char * word, int version)
{
    hash_bucket * current_bucket = &table->buckets[pos];
    if(current_bucket->children == NULL)
    {
        current_bucket->current_children++;
        current_bucket->children = malloc(BUCKET_START_SIZE*sizeof(trie_node));
        init_trie_node(&current_bucket->children[0]);
        table->buckets[pos].children[0].word = copy_string(word);
        // table->buckets[pos].children[0].a_version = version;
        return &current_bucket->children[0];
    }
    else if(current_bucket->current_children == current_bucket->max_children)
    {/*SFAGI*/
        expand_hash_table(table);
        unsigned long code = hash_word(word);
        pos = hash_round(code, table->mod_value);
        if (pos < table->current_breaking)
        {
            pos = hash_round(code, table->mod_value *2);
        }
        if(table->buckets[pos].current_children == table->buckets[pos].max_children)
        {
            table->buckets[pos].max_children += BUCKET_START_SIZE;
            table->buckets[pos].children = realloc(table->buckets[pos].children,  table->buckets[pos].max_children*sizeof(trie_node));
        }
        current_bucket = & table->buckets[pos];
    }

    int spot_on_bucket;
    int found = binary_search_array(current_bucket->children, current_bucket->current_children, word, &spot_on_bucket);
    if(found == -1)
    {
        memmove(&current_bucket->children[spot_on_bucket+1], &current_bucket->children[spot_on_bucket], (current_bucket->current_children-spot_on_bucket)*sizeof(trie_node));
        init_trie_node(&current_bucket->children[spot_on_bucket]);
        current_bucket->children[spot_on_bucket].word = copy_string(word);
        // table->buckets[pos].children[spot_on_bucket].a_version = version;
        current_bucket->current_children ++;
    }
    else if(current_bucket->children[spot_on_bucket].d_version > current_bucket->children[spot_on_bucket].a_version )
    {
        // current_bucket->children[spot_on_bucket].a_version = version;
    }
    return &current_bucket->children[spot_on_bucket];
}

int expand_hash_table(hash_table * table)
{
    table->size++;
    table->buckets = realloc(table->buckets, table->size * sizeof(hash_bucket));
    create_hash_node(&table->buckets[table->size-1]);

    hash_bucket * breaking_bucket = &table->buckets[table->current_breaking];
    hash_bucket * new_bucket = &table->buckets[table->size-1];
    int move_left = 0;
    for (int i = 0; i < breaking_bucket->current_children; i++)
    {
        int new_pos = hash_round(hash_word(breaking_bucket->children[i].word), table->mod_value*2);
        if(new_pos == table->current_breaking)
        {
            if(move_left!=0){
                //must move it
                //also at the end
                memmove(&breaking_bucket->children[i-move_left], &breaking_bucket->children[i], (breaking_bucket->current_children-i)*sizeof(trie_node));
                i-=move_left;
                breaking_bucket->current_children-=move_left;
                move_left=0;
            }
        }
        else
        {
            if(new_bucket->children==NULL)
            {   /* first trie node in the bucket */
                new_bucket->children=malloc(sizeof(trie_node)*BUCKET_START_SIZE);
            }

            if(new_bucket->current_children == new_bucket->max_children)
            {   /* we need to resize our new_bucket cause we have more elements */
                new_bucket->max_children+=BUCKET_START_SIZE;
                new_bucket->children=realloc(new_bucket->children, sizeof(trie_node)*new_bucket->max_children);
            }
            memcpy(&new_bucket->children[new_bucket->current_children],&breaking_bucket->children[i],sizeof(trie_node));

            new_bucket->current_children++;
            move_left++;
        }
    }
    if(move_left!=0){
        /* the final move left children are not there anymore */
        breaking_bucket->current_children-=move_left;
        move_left=0;
    }
    /* increase the pointer to the next bucket */
    table->current_breaking++;
    if(table->current_breaking==table->max_breaking)
    {   /* reset the round */
        table->current_breaking=0;
        table->max_breaking*=2;
        table->round++;
        table->mod_value*=2;
    }
}

trie_node * hash_search(hash_table * table, char * word)
{
    unsigned long word_hash = hash_word(word);
    int bucket_pos = hash_round(word_hash,table->mod_value);
    if(bucket_pos < table->current_breaking){
        //if it belongs to one of the buckets that has been split
        bucket_pos = hash_round(word_hash, table->mod_value*2);
    }
    //the bucket that it supose to be
    hash_bucket* cur_bucket = &table->buckets[bucket_pos];
    int pos,found;

    //do binary search in the array
    found = binary_search_array(cur_bucket->children, cur_bucket->current_children, word, &pos);
    if(found==1){
        //if found return the trie_node
        return &cur_bucket->children[pos];
    }else{
        return NULL;
    }

}

int hash_delete(hash_table * table, char * word,int current_version)
{
    unsigned long code = hash_word(word);
    int position = hash_round(code, table->mod_value);
    if (position < table->current_breaking)
    {
        position = hash_round(code, table->mod_value *2);
    }

    return hash_bucket_delete(&table->buckets[position], word,current_version);
}

int hash_bucket_delete(hash_bucket * bucket, char * word,int current_version)
{
    int i;
    int spot, found;
    found = binary_search_array(bucket->children, bucket->current_children, word, &spot);
    if(found==1)
    {
        bucket->children[spot].d_version=current_version;
        //the original delete
        // free(bucket->children[spot].word);
        // free(bucket->children[spot].children);
        // if( (bucket->current_children-1)!=spot && (bucket->current_children-1)>0 ){
        //     memmove(&bucket->children[spot],&bucket->children[spot+1],(bucket->current_children-spot-1)*sizeof(trie_node));
        // }
        // bucket->current_children--;
        return 1;
    }
    return -1;
}

void hash_clean(hash_table ** table)
{
    hash_table* my_table = *table;
    int i;
    for (i = 0; i < my_table->size; i++) {
        //for each bucket in the table
        hash_bucket* cur_bucket = &my_table->buckets[i];
        int j;
        // for ( j = 0; j < cur_bucket->current_children; j++) {
        //     free(cur_bucket->children[j].word);
        //     free(cur_bucket->children[j].children);
        // }
        free(cur_bucket->children);
    }

    free(my_table->buckets);
    free(*table);
}

struct trie_node  * static_hash_search(hash_table * table, char * word){
    unsigned long word_hash = hash_word(word);
    int bucket_pos = hash_round(word_hash,table->mod_value);
    if(bucket_pos < table->current_breaking){
        //if it belongs to one of the buckets that has been split
        bucket_pos = hash_round(word_hash, table->mod_value*2);
    }
    //the bucket that it supose to be
    hash_bucket* cur_bucket = &table->buckets[bucket_pos];
    int pos,found;

    //do binary search in the array
    found = static_binary_search_array(cur_bucket->children, cur_bucket->current_children, word, &pos);
    if(found==1){
        //if found return the trie_node
        return &cur_bucket->children[pos];
    }else{
        return NULL;
    }
}
