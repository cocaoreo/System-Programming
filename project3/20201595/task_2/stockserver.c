/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#define NTHREADS  500
#define SBUFSIZE  1000

typedef struct item{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    struct item *left_node;
    struct item *right_node;
    sem_t mutex;
    sem_t w;
}ITEM;

typedef struct {
    int *buf;        
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

//char tempbuf[MAXLINE];

ITEM *list=NULL;
sem_t mutex;
sbuf_t sbuf;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);
static void init_echo_cnt(void);

void echo(int connfd);
void *thread(void *vargp);
void init_set();
void checkcommand(int connfd);

ITEM *makebinary (ITEM *root, int id, int m, int price);
void showlist(ITEM *root, char* tempbuf);
ITEM *search (ITEM* root, int id);

void updatetxt(ITEM *root, FILE* update);
void sigint_handler(int sig);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    FILE *stock;
    int id, m, price;

    Signal(SIGINT, sigint_handler);

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    stock = fopen("stock.txt", "r");
    
    if(stock == NULL){
        printf("fopen error\n");
    }

    while(fscanf(stock, "%d %d %d\n", &id, &m, &price)!=EOF){
        list = makebinary(list, id, m, price);
    }
    fclose(stock);

    listenfd = Open_listenfd(argv[1]);
    init_set();

    for (int i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
	    Pthread_create(&tid, NULL, thread, NULL); 

    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
	    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }
}
/* $end echoserverimain */

void init_set(){
    //strcpy(tempbuf, "\0");
    sem_init(&mutex,0,1);
    sbuf_init(&sbuf, SBUFSIZE);
}

ITEM *makebinary (ITEM *root, int id, int m, int price){
    if(root==NULL){
        root = (ITEM *)Malloc(sizeof(ITEM));
        root-> ID= id;
        root->left_stock = m;
        root->price = price;
        root->left_node = NULL;
        root->right_node =NULL;
        root->readcnt=0;
        sem_init(&(root->mutex),0,1);
        sem_init(&(root->w),0,1);

        return root;
    }

    if(root->ID> id){
        root-> left_node = makebinary(root->left_node, id, m, price);
    }

    else if(root->ID < id){
        root-> right_node = makebinary(root->right_node, id, m, price);
    }
    return root;
}

void showlist(ITEM *root, char *tempbuf){
    char temp[100];
    
    if(root!=NULL){
        P(&(root->mutex));
        root->readcnt++;
        if(root->readcnt==1)
            P(&(root->w));
        V(&(root->mutex));

        sprintf(temp, "%d %d %d\n", root->ID, root->left_stock, root->price);
        strcat(tempbuf,temp);

        P(&(root->mutex));
        root->readcnt--;
        if(root->readcnt==0)
            V(&(root->w));
        V(&(root->mutex));
    }
    if(root->left_node!=NULL){
        showlist(root->left_node, tempbuf);
    }
    if(root->right_node!=NULL){
        showlist(root->right_node, tempbuf);
    }
}

ITEM* search(ITEM* root, int id){
    if(root!=NULL){
    if(root->ID==id){
        return root;
    }

    if (root->ID > id){
        search(root->left_node, id);
    }

    else if(root->ID < id){
        search(root->right_node, id);
    }

    else{
        return NULL;
    }
    }

    else{
        return NULL;
    }
}

void updatetxt(ITEM* root, FILE* update){
    if(root != NULL){
        fprintf(update,"%d %d %d\n", root->ID, root->left_stock, root->price);
    }

    else{
        return;
    }

    if(root ->left_node !=NULL){
        updatetxt(root ->left_node, update);
    }

    if(root ->right_node !=NULL){
        updatetxt(root ->right_node, update);
    }
}

void *thread(void *vargp){  
    Pthread_detach(pthread_self());
    
    while (1) { 
	int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
    checkcommand(connfd);
	Close(connfd);
    }

    return NULL;
}

void checkcommand(int connfd){
    int n; 
    char buf[MAXLINE]; 
    rio_t rio;
    char tempbuf[MAXLINE];
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    Pthread_once(&once, init_echo_cnt);

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	    //printf("server received %d bytes\n", n);

        if(!strcmp(buf, "show\n")){ //only read
            memset(tempbuf,0,MAXLINE);
            if(list!=NULL){            
                showlist(list, tempbuf);
                Rio_writen(connfd, tempbuf, MAXLINE);
            }

            else{
                strcpy(tempbuf, "no list\n");
                Rio_writen(connfd, tempbuf, MAXLINE);
            }
        }

        else if(!strncmp(buf, "sell", 4)){ //read&write
            char garb[10];
            int id ,m;
            char temp[MAXLINE];
            sscanf(buf,"%s %d %d", garb, &id, &m);

            ITEM * tempitem = search(list,id);
            //cs start
            if(tempitem!=NULL){
                P(&(tempitem->w));
                tempitem->left_stock += m;
                V(&(tempitem->w));
                strcpy(temp,"[sell] suceess\n");
                Rio_writen(connfd,temp,MAXLINE);
            }

            else{
                strcpy(temp,"no such stock ID\n");
                Rio_writen(connfd, temp, MAXLINE);
            }
        }

        else if(!strncmp(buf,"buy", 3)){ //read&write
            char garb[10];
            int id ,m;
            char temp[MAXLINE];
            sscanf(buf,"%s %d %d", garb, &id, &m);
            ITEM * tempitem = search(list,id);

            //cs start
            if(tempitem!=NULL){
            if(tempitem->left_stock >= m){
                P(&(tempitem->w));
                tempitem->left_stock -= m;
                V(&(tempitem->w));
                strcpy(temp,"[buy] suceess\n");
                Rio_writen(connfd,temp,MAXLINE);
            }

            else{
                strcpy(temp,"Not enough left stock\n");
                Rio_writen(connfd,tempbuf,MAXLINE);
            }
            }

            else{
                strcpy(temp,"no such stock ID\n");
                Rio_writen(connfd, temp, MAXLINE);
            }


            //cs end
        }

        else{
            Rio_writen(connfd,"\0",MAXLINE);
        }
    }
}

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int)); 
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear)%(sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[(++sp->front)%(sp->n)];
    V(&sp->mutex);
    V(&sp->slots);
    return item;
}

static void init_echo_cnt(void)
{
    Sem_init(&mutex, 0, 1);
}

void sigint_handler(int sig){
    int olderr =errno;
    FILE *update = fopen("stock.txt", "w");
    updatetxt(list, update);
    fclose(update);

    errno=olderr;
    exit(0);
}