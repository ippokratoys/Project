#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "heap/heap.h"
#include "bloom_filter/bloom_filter.h"

int count_words(char * s)
{ /*count number of words so we can know the N of our N-Gram*/
  int found_letter = 0;
  int i;
  int count =0;

  for (i = 0;i<strlen(s);i++)
  {
   if (s[i] == ' ')
       found_letter = 0;
   else
   {
       if (found_letter == 0)
           count++;
       found_letter = 1;
   }
  }
  return count;
}

char * copy_string(char * s)
{
  char * new = malloc((strlen(s)+1)*sizeof(char));
  if(new == NULL)
    error_exit("Malloc Failure");
  strcpy(new, s);

  return new;
}

void error_exit(char * s)
{
  fprintf(stderr, "%s\n Program will stop! \n", s);
  exit(EXIT_FAILURE);
}


result_of_search* new_result(int size){
    result_of_search* result=malloc(sizeof(result_of_search));
    result->cur_word=malloc(sizeof(char)*size);
    result->current_size=size;
    result->current_wrote=0;
    result->num_of_results=0;

    result->cur_word[0]='\0';
    return result;
}

void delete_result(result_of_search** result){
    // free((*result)->cur_word);//do not delete the word, we need it
    free((*result)->cur_word);
    free(*result);
    *result=NULL;
}

char* go_to_next_ngram(char* cur_ptr){
    while(*cur_ptr!='\0' && *cur_ptr!='|'){
        cur_ptr++;
    }
    return cur_ptr;
}

char* eat_empty(char* cur_ptr){
    while (cur_ptr[0]==' ' || cur_ptr[0]=='\0') {
        cur_ptr++;
    }
    return cur_ptr;
}

int word_exists(result_of_search *result, char* first_word, char* last_word){
    char* temp;
    //find the first word
    // printf("\t===searching for {%s}...{%s} in {%s}\n",first_word,last_word,result->cur_word );
    first_word=eat_empty(first_word);//deletes trailing white spaces
    temp=strstr(result->cur_word, first_word);//find the first word if exists somewhere in the result
    while(temp!=NULL){//if we found it check if the rest of the string is there also
        //it checks only the word, not if after we have a space or before we have a start of n-gram
        //if it's the first word or the previous char is | (start of ngram)
        if(temp== result->cur_word || temp[-1]=='|'){
            // printf("===found strstr\n");
            int found = 0;//we start from 0 if it never gets in the loop it's false
            char* current_word=first_word;

            while(current_word<=last_word){//check if every word exists
                if(current_word[0]==' '|| current_word[0]=='\0'){
                    current_word++;
                    continue;
                }
                found=1;

                // printf("temp->|%s| \t cur_wo->|%s|\n",temp,current_word);
                int word_size = strlen(current_word);//get the size of the word
                if( strncmp(current_word,temp , word_size)==0 ){
                    //if the word is ok (we must check the next char in the result)
                    if(temp[word_size]=='\0'){
                        exit(-1);
                    }else if (temp[word_size]==' '){
                        //everything is ok
                        //we matched the word and the next char is space
                        // so we are ok to check the next word if exists
                    }else if(temp[word_size]=='|'){
                        //the word is ok but we must break cause there is no next word in the result ngram
                        temp+=word_size+1;//go to the next word in the result

                        break;
                    }else{
                        //the next character is still a normal char so we didn't find the word
                        found=0;
                        temp=go_to_next_ngram(temp);
                        break;
                    }
                    current_word+=word_size+1;//go to the next word to add_to_result
                    temp+=word_size+1;//go to the next word in the already result
                }else{
                    //the word is not ok
                    //go to the next word
                    temp=go_to_next_ngram(temp);
                    found=0;
                    break;
                }
            }
            if(found==1 && current_word>=last_word){
                //that means that all words matches ok
                //check if the next char is |
                // printf("===it's at the end{%s}\n", temp );
                if(temp[-1]=='|' || temp[-1]=='\0'){
                    return 1;
                }
            }
        }else{
            temp++;
        }
        temp=strstr(temp, first_word);//find if there is next

    }
    return 0;
}

void add_to_result(result_of_search* result,char* the_word,char* first_word, heap* my_heap, bloom_filter * bloom){
    char *init_the_word=the_word, *init_first_word=first_word;
    result->num_of_results++;
    int bytes_wrote;
    // printf("{%s}->{%s}\t{%s}[%d]\n",first_word,the_word,result->cur_word,result->current_wrote);
    int old_start = result->current_wrote;
    int total_write = 0;
    while(first_word<=the_word){//for every word until the last one
        // printf("$$$%s\n",first_word);
        int size_of_word = strlen(first_word);
        if(size_of_word==0){//if it's an empty word (a space)
            first_word++;//start from next char
            continue;
        }
        while( size_of_word + 2 + result->current_wrote > result->current_size){//if we need to realloc
            // printf("we need to realloc %d->%d(+%d)\n",result->current_size,2*result->current_size,size_of_word );
            //we need to make it bigger
            result->current_size*=2;
            result->cur_word=realloc(result->cur_word, result->current_size);
            if(result->cur_word==NULL){
                error_exit("Realloc faile in tools add_to_result");
            }
        }
        total_write+=size_of_word+1;
        strcpy(&result->cur_word[result->current_wrote],first_word);//copy the word
        result->current_wrote+=size_of_word+1;
        result->cur_word[result->current_wrote-1]=' ';//instead of '\0' add space
        first_word+=size_of_word+1;//go to the next word
    }
    int old_val = result->cur_word[total_write-1];
    result->cur_word[old_start+total_write-1]='\0';
    // printf("{%s}\n", &result->cur_word[old_start]);
    int exists=bloom_insert_and_check(&result->cur_word[old_start],bloom);
    // int exists = word_exists(result,init_first_word,init_the_word);//checks if the word exists already on the result
    if(exists==1){
        result->cur_word[old_start]='\0';
        result->current_wrote=old_start;
        return;
    }else{
        // printf("---------------------\n");
        // printf("\nADDING {%s}->{%s}\t{%s}[%d]\n",first_word,the_word,result->cur_word,result->current_wrote);
        // printf("---------------------\n");
        // printf("\t===no\n" );
    }
    heap_insert(my_heap, &result->cur_word[old_start]);
    result->cur_word[total_write-1]=old_val;
    // printf("!!!%s\n",result->cur_word);
    result->cur_word[result->current_wrote-1]='|';//add the ending chars
    result->cur_word[result->current_wrote]='\0';
    result->current_wrote+=0;//do nothing :P
}
