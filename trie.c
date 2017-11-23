#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"
// #include "hash_table.h"
#include "tools.h"
#include "heap.h"
#include "trie_node.h"

#define INITIAL_RESULT_SIZE 2048 //the size of the string that holds the result
#define COPY_ON_SEARCH 0 //if we will copy the string on search

trie * init_trie()
{
    trie * my_trie = malloc(sizeof(trie));
    if(my_trie == NULL)
        error_exit("Malloc Failure");

    my_trie->children = create_hash_table();

    return my_trie;
}


OK_SUCCESS insert_ngram(trie * my_trie, char * ngram)
{
    if(ngram == NULL || strcmp(ngram, "")==0)
        return 1;
    char * word = strtok(ngram, " ");
    trie_node * node = hash_insert(my_trie->children, word);
    return insert_ngram_to_node( node, NULL );
}

OK_SUCCESS trie_clean(trie** mytree){
    int i,j;
    for(i = 0  ; i < (*mytree)->children->size ; i++ )
    {
        for(j = 0 ; j < (*mytree)->children->buckets[i].current_children ; j++)
        {
            trie_node_clean(&(*mytree)->children->buckets[i].children[j]);
        }
    }
    hash_clean(&(*mytree)->children);
    free(*mytree);
    *mytree=NULL;
    return 1;
}

OK_SUCCESS delete_ngram(trie * my_trie, char * ngram)
{
    return trie_delete(my_trie->children, ngram);
}

char* get_word(char* cur_ptr, char* original_ptr, int init_size){
    //we must not pass the original_ptr!
    char* temp=cur_ptr;
    if( (cur_ptr>=&original_ptr[init_size]) || (cur_ptr[0]=='\0' && &cur_ptr[init_size]==cur_ptr)){
        //that means that we are at the end
        return NULL;
        int i;
        // for (size_t i = 0; i < init_size; i++) {
        //     if(original_ptr[i]=='\0'){
        //         original_ptr[i]=' ';
        //     }
        // }
        // printf("end of word\n");
    }
    int num_of_white_spaces=0;
    while(temp[0]==' ' || temp[0]=='\t' || temp[0]=='\n'){
        temp[0]='\0';
        temp++;
        num_of_white_spaces++;
    }
    while(temp[0]!=' ' && temp[0]!='\0'){
        temp++;
    }
    temp[0]='\0';
    return &cur_ptr[num_of_white_spaces];
}

result_of_search* search(trie* my_trie, char* the_ngram,heap* my_heap)
{
    #if COPY_ON_SEARCH==0
        char* current_sub_str=the_ngram;//just get the pointer to the initial string
    #else
        char* current_sub_str=copy_string(the_ngram);//copy the string and start from the begining
    #endif

    char* copied_string=current_sub_str;//holds the beggin of the string so we can free
    int original_size = strlen(current_sub_str);
    result_of_search* result = new_result(INITIAL_RESULT_SIZE);

    while(current_sub_str[0]!='\0'){
    //while you don't have just a space or just a \0

/***********************search for this ngram*************/
        // printf("Search for{%s}\n",current_sub_str);
        //get pointer to the begginig of the current word until the end
        char* search_for=current_sub_str;

        // trie_node* cur_node = my_trie->root;//start from the root
        trie_node* cur_node = NULL;
        char* current_word;//the current word form iteration
        current_word=get_word(search_for,copied_string,original_size);
        //while we have nodes and the word is not finished
        //do somthing different for the root(because of hash table)
        hash_table* root_hash_table = my_trie->children;
        if(current_word!=NULL){
            cur_node = hash_search(root_hash_table, current_word);//search in the hash table
            if(cur_node!=NULL && cur_node->is_final=='Y'){//if found and it's final
                add_to_result(result,current_word,current_sub_str, my_heap);//add to the result
            }
            current_word=get_word(&current_word[strlen(current_word)+1], copied_string, original_size);//go to the next word
        }
        while(cur_node!=NULL && cur_node->word!=NULL && current_word!=NULL){
            int spot_of_word,return_value;
            // printf("\t{%s} ",current_word);fflush(stdout);
            // int spot=search_kid(cur_node,);
            return_value=binary_search_kid(cur_node, current_word, &spot_of_word);
            if (return_value==1) {
                // printf("\tFOUND at %d ",spot_of_word);fflush(stdout);
                if(cur_node->children[spot_of_word].is_final=='Y'){
                    // printf("\t and it's final|SO I ADD TO RESULT|\n");

                    // printf("I add[%s]\n",current_sub_str);
                    add_to_result(result,current_word,current_sub_str, my_heap);
                }
                cur_node=&cur_node->children[spot_of_word];//go deeper
            } else {
                // printf("\tNOT FOUND at %d\n",spot_of_word);
                cur_node=NULL;
                break;
            }

            current_word=get_word(&current_word[strlen(current_word)+1], copied_string, original_size);
            //go deeper
            // printf("\n---\n" );
        }
/*****************end of search***************************/
        // free(search_for);
        // printf("End of search for {%s}\n",current_sub_str);
        if(current_word==NULL){
            //if we finished the whole word
            current_word=copied_string+original_size-1;
        }else if(&current_word[strlen(current_word)]!=&copied_string[original_size]){
            //if it's not the last word change the last '\0' to ' '
            current_word[strlen(current_word)]=' ';
        }
        while(current_sub_str<=current_word){
            if(current_word[0]=='\0'){
                current_word[0]=' ';
            }
            current_word--;
        }
/******************get the next word**********************/
        while(current_sub_str[0]!='\0' && current_sub_str[0]!=' '){
            //search until the next space or the end of the word
            current_sub_str++;
        }
        if(current_sub_str[0]==' '){
            //if we found a space
            while(current_sub_str[0]==' '){
                current_sub_str++;//eat all white spaces
            }
        }
    }
    #if COPY_ON_SEARCH!=0
        free(copied_string);//nessasery only if the string is beeing copied
    #endif
    clean_vector(bloom);
    return result;
}
