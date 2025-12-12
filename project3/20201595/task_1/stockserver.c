/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

typedef struct item{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    struct item *left_node;
    struct item *right_node;
}ITEM;

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
}stockpool;

char tempbuf[MAXLINE];
ITEM *list = NULL;

void echo(int connfd);
void init_pool(int listenfd, stockpool *p);
void add_client(int connfd, stockpool *p);
void check_clients(stockpool *p, ITEM* list);

ITEM *makebinary (ITEM *root, int id, int m, int price);
void showlist(ITEM *list);
ITEM *search (ITEM* root, int id);

void updatetxt(ITEM *list, FILE* update);
void sigint_handler(int sig);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    //char client_hostname[MAXLINE], client_port[MAXLINE];
    static stockpool pool;
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
    init_pool(listenfd, &pool);

    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1,&pool.ready_set, NULL,NULL, NULL);
        
        if(FD_ISSET(listenfd, &pool.ready_set)){
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        check_clients(&pool, list);
    }
    exit(0);
}
/* $end echoserverimain */

void init_pool(int listenfd, stockpool *p){
    p->maxi = -1;
    for(int i=0;i<FD_SETSIZE;i++){
        p->clientfd[i]= -1;
    }

    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, stockpool *p) 
{
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++)
	if (p->clientfd[i] < 0) {
	    p->clientfd[i] = connfd;
	    Rio_readinitb(&p->clientrio[i], connfd);

	    FD_SET(connfd, &p->read_set);

	    if (connfd > p->maxfd){
		    p->maxfd = connfd;
        }

	    if (i > p->maxi){
		    p->maxi = i;
        }

	    break;
	}
    if (i == FD_SETSIZE){
	    app_error("add_client error: Too many clients");
    }
}

void check_clients(stockpool *p, ITEM *list)
{
    int i, connfd, n;
    char buf[MAXLINE]; 
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
	    connfd = p->clientfd[i];
	    rio = p->clientrio[i];

	    if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) { 
	        p->nready--;
	        if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                //printf("server reveived %d bytes\n", n);

                if(!strcmp(buf, "show\n")){
                    strcpy(tempbuf,"\0");
                    if(list!=NULL){            
                    showlist(list);
                    Rio_writen(connfd, tempbuf, MAXLINE);
                    }

                    else{
                        strcpy(tempbuf, "no list\n");
                        Rio_writen(connfd, tempbuf, MAXLINE);
                    }
                }

                else if(!strncmp(buf,"sell", 4)){
                    strtok(buf, " ");
                    int id = atoi(strtok(NULL, " "));
                    int m = atoi(strtok(NULL, " "));
                    ITEM * tempitem = search(list,id);

                    if (tempitem !=NULL){
                        strcpy(tempbuf,"\0");
                        tempitem->left_stock += m;
                        strcpy(tempbuf,"[sell] suceess\n");
                        Rio_writen(connfd,tempbuf,MAXLINE);
                    }

                    else{
                        memset(tempbuf,0,MAXLINE);
                        strcpy(tempbuf,"no such stock ID\n");
                        Rio_writen(connfd,tempbuf,MAXLINE);
                    }
                }

                else if(!strncmp(buf,"buy", 3)){
                    strtok(buf, " ");
                    int id = atoi(strtok(NULL, " "));
                    int m = atoi(strtok(NULL, " "));
                    ITEM * tempitem = search(list,id);

                    if (tempitem !=NULL){
                        strcpy(tempbuf,"\0");
                        
                        if(tempitem->left_stock >= m){
                            tempitem->left_stock -= m;
                            strcpy(tempbuf,"[buy] suceess\n");
                            Rio_writen(connfd,tempbuf,MAXLINE);
                        }

                        else{
                            strcpy(tempbuf,"Not enough left stock\n");
                            Rio_writen(connfd,tempbuf,MAXLINE);
                        }
                    }

                    else{
                        memset(tempbuf,0,MAXLINE);
                        strcpy(tempbuf,"no such stock ID\n");
                        Rio_writen(connfd,tempbuf,MAXLINE);
                    }
                }

                else{
                    strcpy(tempbuf,"\0");
                    Rio_writen(connfd,tempbuf,MAXLINE);
                }
		            
	        }

	        else { 
		        Close(connfd);
		        FD_CLR(connfd, &p->read_set);
		        p->clientfd[i] = -1;
	        }
	    }
    }
}

ITEM *makebinary (ITEM *root, int id, int m, int price){
    if(root==NULL){
        root = (ITEM *)Malloc(sizeof(ITEM));
        root-> ID= id;
        root->left_stock = m;
        root->price = price;
        root->left_node = NULL;
        root->right_node =NULL;

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

void showlist(ITEM *list){
    char temp[100];
    if(list!=NULL){
        sprintf(temp, "%d %d %d\n", list->ID, list->left_stock, list->price);
        strcat(tempbuf,temp);
    }
    if(list->left_node!=NULL){
        showlist(list->left_node);
    }
    if(list->right_node!=NULL){
        showlist(list->right_node);
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

void updatetxt(ITEM* list, FILE* update){
    if(list != NULL){
        fprintf(update,"%d %d %d\n", list->ID, list->left_stock, list->price);
    }

    else{
        return;
    }

    if(list ->left_node !=NULL){
        updatetxt(list ->left_node, update);
    }

    if(list ->right_node !=NULL){
        updatetxt(list ->right_node, update);
    }
}

void sigint_handler(int sig){
    int olderr =errno;
    FILE *update = fopen("stock.txt", "w");
    updatetxt(list, update);
    fclose(update);

    errno=olderr;
    exit(0);
}