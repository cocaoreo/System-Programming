#include "main.h"



int main(){
    char str[50];

    while(1){
        fgets(str, sizeof(str), stdin);
        str[strlen(str) - 1] = '\0';
        commandcheck(str);
    }

    return 0;
}

void commandcheck(char * str){
    char * ptr = strtok(str, " ");
    if(strcmp(ptr, commandlist[0])==0){  //create 명령어
        ptr = strtok(NULL, " ");
        if(strcmp(ptr,category[0])==0){//create list
            ptr = strtok(NULL, " ");
            int i = *(ptr+4) - 0x30;
            //struct list *lnew= (struct list *)malloc(sizeof(struct list));
            struct list *lnew = (struct list*)malloc(sizeof(struct list));
            list_init(lnew);
            lcon[i] = lnew; 
        }

        else if(strcmp(ptr,category[1])==0){//create hashtable
            ptr = strtok(NULL, " ");
            int i = *(ptr+4) - 0x30;
            struct hash *hnew = (struct hash *)malloc(sizeof(struct hash));
            hash_init(hnew, hash_hashf, less, NULL);
            hcon[i] = hnew; 
        }

        else if(strcmp(ptr,category[2])==0){ // create bitmap
            ptr = strtok(NULL, " ");
            int i = *(ptr+2) - 0x30;
            ptr = strtok(NULL, " ");
            size_t bc = atoi(ptr);
            struct bitmap * newmap= bitmap_create(bc); 
            bcon[i]= newmap;
        }
        
    }
    
    else if(strcmp(ptr,commandlist[1])==0){// delete 명령어
        ptr = strtok(NULL, " ");
        del(ptr);
    }

    else if(strcmp(ptr,commandlist[2])==0){// print data
        ptr = strtok(NULL, " ");
        dumpdata(ptr);
    }

    else if(strcmp(ptr,commandlist[3])==0){ // list_push_front
        ptr = strtok(NULL," ");
        int i = *(ptr+4)-0x30;
        struct list_elem *e= (struct list_elem *)malloc(sizeof(struct list_elem));
        struct list_item *temp = list_entry(e,struct list_item, elem);
        ptr=strtok(NULL," ");
        temp->data= atoi(ptr);
        list_push_front(lcon[i],e);
    }

    else if(strcmp(ptr,commandlist[4])==0){ // list_push_back
        ptr = strtok(NULL," ");
        int i = *(ptr+4)-0x30;
        struct list_elem *e= (struct list_elem *)malloc(sizeof(struct list_elem));
        struct list_item *temp = list_entry(e,struct list_item, elem);
        ptr=strtok(NULL," ");
        temp->data= atoi(ptr);
        list_push_back(lcon[i],e);
    }

    else if(strcmp(ptr,commandlist[5])==0){ // list_pop_front
        ptr = strtok(NULL," ");
        int i = *(ptr+4)-0x30;
        list_pop_front(lcon[i]);
    }

    else if(strcmp(ptr,commandlist[6])==0){ // list_pop_back
        ptr = strtok(NULL," ");
        int i = *(ptr+4)-0x30;
        list_pop_back(lcon[i]);
    }

    else if(strcmp(ptr,commandlist[7])==0){ // list_remove
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        struct list_elem *e;
        ptr = strtok(NULL, " ");
        int key = atoi(ptr);
        int k=0;
        for(e= list_begin(lcon[i]), k=0; k<key&& e !=list_end (lcon[i]) ;e = list_next(e), k++){
        }
        list_remove(e);
    }

    else if(strcmp(ptr,commandlist[8])==0){ // list_insert
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        struct list_elem *e;
        ptr = strtok(NULL, " ");
        int spot = atoi(ptr);// insert place
        ptr = strtok(NULL, " ");
        int key = atoi(ptr);// insert data

        int k =0;
        for(e= list_begin(lcon[i]), k=0; k<spot;e = list_next(e), k++){
        }
        struct list_elem *b = (struct list_elem *)malloc(sizeof(struct list_elem));
        struct list_item * li = list_entry(b, struct list_item, elem);
        li->data = key; 
        list_insert(e,b);
        
    }
    
    else if(strcmp(ptr,commandlist[9])==0){ // list_splice
        ptr = strtok(NULL," ");
        int i= atoi(ptr+4);
        struct list_elem *e;
        ptr = strtok(NULL, " ");
        int spot = atoi(ptr);// insert place

        int k =0;
        for(e= list_begin(lcon[i]), k=0; k<spot;e = list_next(e), k++){
        }

        ptr = strtok(NULL," ");
        int j= atoi(ptr+4);// list2
        ptr = strtok(NULL," ");
        int start = atoi(ptr);//start point
        ptr = strtok(NULL," ");
        int end = atoi(ptr);//end numb

        struct list_elem *first;
        struct list_elem *last;

        for(first= list_begin(lcon[j]), k=0; k<start;first = list_next(first), k++){
        }
        last = first;

        for(k=start; k<end;last = list_next(last), k++){
        }
        list_splice(e,first,last);
        
    }

    else if(strcmp(ptr,commandlist[10])==0){ // list front
        ptr = strtok(NULL," ");
        int i= atoi(ptr+4);
        struct list_elem * e = list_begin(lcon[i]);
        struct list_item * li = list_entry(e, struct list_item, elem);
        printf("%d\n", li->data);

    }

    else if(strcmp(ptr,commandlist[11])==0){ // list back
        ptr = strtok(NULL," ");
        int i= atoi(ptr+4);
        struct list_elem * e = list_rbegin(lcon[i]);
        struct list_item * li = list_entry(e, struct list_item, elem);
        printf("%d\n", li->data);

    }

    else if(strcmp(ptr,commandlist[12])==0){ // list sort
        ptr = strtok(NULL," ");
        int i= atoi(ptr+4);
        list_sort(lcon[i], compare, NULL);

    }

    else if(strcmp(ptr,commandlist[13])==0){ // list insert ordered
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        int key = atoi(ptr);// insert data

        struct list_elem *b = (struct list_elem *)malloc(sizeof(struct list_elem));
        struct list_item * li = list_entry(b, struct list_item, elem);
        li->data = key;
        list_insert_ordered(lcon[i], b, compare, NULL);
    }

    else if(strcmp(ptr,commandlist[14])==0){ // list unique
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        char* ptr2 = strtok(NULL, " ");
        if(ptr2==NULL){
            list_unique(lcon[i], NULL, compare, NULL);
        }

        else{
            int k = *(ptr2+4) - 0x30;
            list_unique(lcon[i], lcon[k], compare, NULL);
        }
        
    }

    else if(strcmp(ptr,commandlist[15])==0){ // list reverse
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        list_reverse(lcon[i]);
    }

    else if(strcmp(ptr,commandlist[16])==0){ // list empty
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        if(list_empty(lcon[i])){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[17])==0){ // list size
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        size_t t = list_size(lcon[i]);

        printf("%zu\n", t);
    }

    else if(strcmp(ptr,commandlist[18])==0){ // list max
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        
        struct list_elem *e = list_max(lcon[i], compare, NULL);
        struct list_item * li = list_entry(e, struct list_item, elem);
        printf("%d\n", li->data);
    }

    else if(strcmp(ptr,commandlist[19])==0){ // list min
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        
        struct list_elem *e = list_min(lcon[i], compare, NULL);
        struct list_item * li = list_entry(e, struct list_item, elem);
        printf("%d\n", li->data);
    }

    else if(strcmp(ptr,commandlist[20])==0){ // list swap
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL," ");
        int a = atoi(ptr);
        ptr = strtok(NULL," ");
        int b = atoi(ptr);
        
        struct list_elem *front= NULL, *back=NULL;
        int k=0;

        for(front= list_begin(lcon[i]), k=0; k<a ;front = list_next(front), k++){
        }

        for(back= list_begin(lcon[i]), k=0; k<b ;back = list_next(back), k++){
        }
        list_swap(front, back);
    }

    else if(strcmp(ptr,commandlist[21])==0){ // list shuffle
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;

        list_shuffle(lcon[i]);
    }
    //hash
    else if(strcmp(ptr,commandlist[22])==0){ // hash_insert
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        int data = atoi(ptr);
        struct hash_elem * n = (struct hash_elem *)malloc(sizeof(struct hash_elem));
        n->value = data;
        hash_insert(hcon[i], n);
    }

    else if(strcmp(ptr,commandlist[23])==0){ // hash_apply
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        
        if(strcmp(ptr,"square")==0){
            hash_apply(hcon[i], square);
        }

        else if(strcmp(ptr,"triple")==0){
            hash_apply(hcon[i], triple);
        }
    }

    else if(strcmp(ptr,commandlist[24])==0){ // hash_delete
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        int data = atoi(ptr);

        struct hash_elem e;
        e.value = data;
        struct hash_elem *t =hash_delete(hcon[i], &e);
        free(t);
    }
    
    else if(strcmp(ptr,commandlist[25])==0){ // hash_find
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        int data = atoi(ptr);

        struct hash_elem e;
        e.value = data;
        struct hash_elem *t = hash_find(hcon[i], &e);

        if(t!=NULL){
            printf("%d\n", t->value);
        }
    }

    else if(strcmp(ptr,commandlist[26])==0){ // hash_empty
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        
        if(hash_empty(hcon[i])){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[27])==0){ // hash_size
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        
        size_t size= hash_size(hcon[i]);
        printf("%zu\n", size);
    }

    else if(strcmp(ptr,commandlist[28])==0){ // hash_clear
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        
        hash_clear(hcon[i],destruct);

    }

    else if(strcmp(ptr,commandlist[29])==0){ // hash_replace
        ptr = strtok(NULL," ");
        int i= *(ptr+4) - 0x30;
        ptr = strtok(NULL, " ");
        int data= atoi(ptr);
        struct hash_elem *t= (struct hash_elem *)malloc(sizeof(struct hash_elem));
        t->value = data;
        struct hash_elem *temp = hash_replace(hcon[i], t);
        free(temp);
    }

    else if(strcmp(ptr,commandlist[30])==0){ // bitmap_mark
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t point= atoi(ptr);

        bitmap_mark(bcon[i], point);
    }

    else if(strcmp(ptr,commandlist[31])==0){ // bitmap_all
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start= atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t size= atoi(ptr);

        if(bitmap_all(bcon[i], start, size)){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[32])==0){ // bitmap_any
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start= atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t size= atoi(ptr);

        if(bitmap_any(bcon[i], start, size)){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[33])==0){ // bitmap_contains
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start= atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t size= atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            if(bitmap_contains(bcon[i], start, size, true)){
                printf("true\n");
            }

            else{
                printf("false\n");
            }
        }

        else{
            if(bitmap_contains(bcon[i], start, size, false)){
                printf("true\n");
            }

            else{
                printf("false\n");
            }
        }
    }

    else if(strcmp(ptr,commandlist[34])==0){ // bitmap_count
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start= atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t size= atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            printf("%zu\n", bitmap_count(bcon[i], start, size, true));
        }

        else{
            printf("%zu\n", bitmap_count(bcon[i], start, size, false));
        }
    }

    else if(strcmp(ptr,commandlist[35])==0){ // bitmap_flip
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t idx = atoi(ptr);

        bitmap_flip(bcon[i], idx);
    }

    else if(strcmp(ptr,commandlist[36])==0){ // bitmap_none
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t size = atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t cnt = atoi(ptr);

        if(bitmap_none(bcon[i], size, cnt)){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[37])==0){ // bitmap_reset
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t idx = atoi(ptr);

        bitmap_reset(bcon[i], idx);
    }

    else if(strcmp(ptr,commandlist[38])==0){ // bitmap_scan
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start = atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t cnt = atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            printf("%zu\n", bitmap_scan(bcon[i], start, cnt, true));

        }

        else{
            printf("%zu\n", bitmap_scan(bcon[i], start, cnt, false));
        }
    }

    else if(strcmp(ptr,commandlist[39])==0){ // bitmap_scan_and_flip
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start = atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t cnt = atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            printf("%zu\n", bitmap_scan_and_flip(bcon[i], start, cnt, true));

        }

        else{
            printf("%zu\n", bitmap_scan_and_flip(bcon[i], start, cnt, false));
        }
    }

    else if(strcmp(ptr,commandlist[40])==0){ // bitmap_set_all
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");


        if(strcmp(ptr, "true")==0){
            bitmap_set_all(bcon[i], true);

        }

        else{
            bitmap_set_all(bcon[i], false);
        }
    }

    else if(strcmp(ptr,commandlist[41])==0){ // bitmap_set_multiple
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t start = atoi(ptr);
        ptr = strtok(NULL, " ");
        size_t cnt = atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            bitmap_set_multiple(bcon[i], start, cnt, true);

        }

        else{
            bitmap_set_multiple(bcon[i], start, cnt, false);
        }
    }

    else if(strcmp(ptr,commandlist[42])==0){ // bitmap_set
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t idx = atoi(ptr);
        ptr = strtok(NULL, " ");

        if(strcmp(ptr, "true")==0){
            bitmap_set(bcon[i],idx, true);

        }

        else{
            bitmap_set(bcon[i],idx, false);
        }
    }

    else if(strcmp(ptr,commandlist[43])==0){ // bitmap_size
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;

        printf("%zu\n", bitmap_size(bcon[i]));
    }

    else if(strcmp(ptr,commandlist[44])==0){ // bitmap_test
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL, " ");
        size_t idx = atoi(ptr);

        if(bitmap_test(bcon[i], idx)){
            printf("true\n");
        }

        else{
            printf("false\n");
        }
    }

    else if(strcmp(ptr,commandlist[45])==0){ // bitmap_dump
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;

        bitmap_dump(bcon[i]);
    }

    else if(strcmp(ptr,commandlist[46])==0){ // bitmap_expand
        ptr = strtok(NULL," ");
        int i= *(ptr+2) - 0x30;
        ptr = strtok(NULL," ");
        int size = atoi(ptr);
        bcon[i]=bitmap_expand(bcon[i], size);
    }

    else if(strcmp(ptr,"quit")==0){
        exit(0);
    }
}

void del(char* str){
    
    if(strncmp(str,category[0],4)==0){// delete list
        int i = *(str+4) - 0x30;
        free(lcon[i]);
        lcon[i]=NULL;
    }

    else if(strncmp(str,category[1],4)==0){ //delete hash
        int i = *(str+4) - 0x30;
        free(hcon[i]);
        hcon[i]=NULL;
    }

    else if(strncmp(str,"bm",2)==0){
       int i = *(str+2) - 0x30;
        bitmap_destroy(bcon[i]);
    }
        
}

void dumpdata(char * str){
    if(strncmp(str,category[0],4)==0){// list
        int i= *(str+4) - 0x30;
        
        if(list_empty(lcon[i])){
            return ;
        }

        struct list_elem *e;
        for(e= list_begin(lcon[i]);e !=list_end (lcon[i]);e = list_next(e)){
            struct list_item * li = list_entry(e, struct list_item, elem);
            printf("%d ", li->data);  
        }
        printf("\n");
    }

    else if(strncmp(str,category[1],4)==0){// hash
        int i= *(str+4) - 0x30;
        if(hash_empty(hcon[i])){
            return ;
        }

        int k = hcon[i]->bucket_cnt;

        struct list_elem *e;

        for(int a=0;a<k;a++){
            for(e= list_begin(&hcon[i]->buckets[a]);e !=list_end (&hcon[i]->buckets[a]);e = list_next(e)){
                struct list_item * li = list_entry(e, struct list_item, elem);
                printf("%d ", li->data);  
            }
        }
        printf("\n");
        
    }

    else if(strncmp(str,"bm",2)==0){ //bitmap
        int i = *(str+2)-0x30;
        size_t cnt = bcon[i]->bit_cnt;

        for(size_t k=0; k<cnt;k++){
            printf("%d", bitmap_test(bcon[i], k));
        }
        printf("\n");
    }
}
