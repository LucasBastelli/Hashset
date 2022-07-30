#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <malloc.h>


typedef struct{
    int val;
    int next;
}node;

typedef struct thread_data {
  unsigned long nb_add;
  unsigned long nb_remove;
  unsigned long nb_contains;
  unsigned long nb_found;
  unsigned short seed[3];
  int diff;
  int range;
  int update;
  int alternate;
} thread_data_t;

static volatile int stop;
static unsigned short main_seed[3];

static inline void rand_init(unsigned short *seed)
{
  seed[0] = (unsigned short)rand();
  seed[1] = (unsigned short)rand();
  seed[2] = (unsigned short)rand();
}

static inline int rand_range(int n, unsigned short *seed)
{
  /* Return a random number in range [0;n) */
  int v = (int)(erand48(seed) * n);
  assert (v >= 0 && v < n);
  return v;
}


# define INIT_SET_PARAMETERS            /* Nothing */

# define NB_BUCKETS                     (1UL << 17)

# define HASH(a)                        (hash((uint32_t)a) & (NB_BUCKETS - 1))

typedef intptr_t val_t;

static uint32_t hash(uint32_t a)
{
  /* Knuth's multiplicative hash function */
  a *= 2654435761UL;
  return a;
}

void set_new(FILE *arqHASH){
    int i=0;
    node new;
    new.val=-1;
    new.next=-1;
    while(i<NB_BUCKETS){
        fwrite(&new,sizeof(node),1,arqHASH);
        i++;
    }
    return;
}

int delete(int val,FILE *arqHASH)
{
    int i=0,previous=0;
    node test,aux;
    i=HASH(val);
    fseek(arqHASH,i*sizeof(node),SEEK_SET);
    fread(&test, sizeof(node), 1, arqHASH);
    while(1){
        if(test.val == val){
            if(test.next==-1){
                fseek(arqHASH,-sizeof(node),SEEK_CUR);
                test.val=-1;
                test.next=-1;
                fwrite(&test,sizeof(node),1,arqHASH);
                return 1;
            }
            else{
                previous=ftell(arqHASH)-sizeof(node);
                fseek(arqHASH,test.next,SEEK_SET);
                fread(&aux, sizeof(node), 1, arqHASH);
                fseek(arqHASH,-sizeof(node),SEEK_CUR);
                test.val=-1;
                test.next=-1;
                fwrite(&test,sizeof(node),1,arqHASH);
                fseek(arqHASH,previous,SEEK_SET);
                fwrite(&aux,sizeof(node),1,arqHASH);
                return 1;
            }
        }else{
            if(test.next!=-1){
                fseek(arqHASH,test.next,SEEK_SET);
                fread(&test, sizeof(node), 1, arqHASH);
            }
            else{
				return 0;
			}
        }  
    }
}


int add(int val,FILE *arqHASH)
{
    int i=0, aux=0;
    node test;
    i=HASH(val);
    fseek(arqHASH,i*sizeof(node),SEEK_SET);
    fread(&test, sizeof(node), 1, arqHASH);
    int previous = ftell(arqHASH) - sizeof(node);
    while(1){
        if(test.val==val)
            return 0;
        if(test.val == -1){
            fseek(arqHASH,-sizeof(node),SEEK_CUR);
            test.val=val;
            test.next=-1;
            fwrite(&test,sizeof(node),1,arqHASH);
            if(previous!=(ftell(arqHASH)-sizeof(node))){
                aux=ftell(arqHASH);
                fseek(arqHASH,previous,SEEK_SET);
                fread(&test, sizeof(node),1,arqHASH);
                test.next=aux;
                fseek(arqHASH,-sizeof(node),SEEK_CUR);
                fwrite(&test,sizeof(node),1,arqHASH);
            }
            return 1;
        }else{
            previous = ftell(arqHASH)-sizeof(node);
            if(test.next!=-1){
                fseek(arqHASH,test.next,SEEK_SET);
                fread(&test, sizeof(node), 1, arqHASH);
            }
            else{
                fseek(arqHASH,0,SEEK_END);
                test.val=val;
                test.next=-1;
                aux=ftell(arqHASH);
                fwrite(&test,sizeof(node),1,arqHASH);
                fseek(arqHASH,previous,SEEK_SET);
                fread(&test, sizeof(node), 1, arqHASH);
                test.next=aux;
                fseek(arqHASH,-sizeof(node),SEEK_CUR);
                fwrite(&test,sizeof(node),1,arqHASH);
				return 1;
			}
        }  
    }
}

int search(int val,FILE *arqHASH)
{
    int i=0;
    node test,aux;
    i=HASH(val);
    fseek(arqHASH,i*sizeof(node),SEEK_SET);
    fread(&test, sizeof(node), 1, arqHASH);
    while(1){
        if(test.val == val){
            return 1;
        }else{
            if(test.next!=-1){
                fseek(arqHASH,test.next,SEEK_SET);
                fread(&test, sizeof(node), 1, arqHASH);
            }
            else{
				return 0;
			}
        }  
    }
}

int set_size(FILE *arqHASH){
  int size = 0, posi = 0;
  unsigned int i;
  node test;
  rewind(arqHASH);
  for (i = 0; i < NB_BUCKETS; i++) {
    fread(&test, sizeof(node), 1, arqHASH);
    posi = ftell(arqHASH);
    if(test.val!=-1){
      while(test.next!=-1){
        fseek(arqHASH,test.next,SEEK_SET);
        fread(&test, sizeof(node), 1, arqHASH);
        size++;
      }
      size++;
    }
    fseek(arqHASH,posi,SEEK_SET);
    i++;
  }
  return size;
}


static void *test(void *data)
{
  int op, val, last = -1;
  thread_data_t *d = (thread_data_t *)data;
  FILE *arqHASH = fopen("Hash.bin", "r+b");
  printf("Range: %u\n",d->range);
  while (stop == 0) {
    op = rand_range(100, d->seed);
    if (op < d->update) {
      if (d->alternate) {
        /* Alternate insertions and removals */
        if (last < 0) {
          /* Add random value */
          val = rand_range(d->range, d->seed) + 1;
          if (add(val, arqHASH)) {
            d->diff++;
            last = val;
          }
          d->nb_add++;
        } else {
          /* Remove last value */
          if (delete(last, arqHASH))
            d->diff--;
          d->nb_remove++;
          last = -1;
        }
      } else {
        /* Randomly perform insertions and removals */
        val = rand_range(d->range, d->seed) + 1;
        if ((op & 0x01) == 0) {
          /* Add random value */
          if (add(val, arqHASH))
            d->diff++;
          d->nb_add++;
        } else {
          /* Remove random value */
          if (delete(val, arqHASH))
            d->diff--;
          d->nb_remove++;
        }
      }
    } else {
      /* Look for random value */
      val = rand_range(d->range, d->seed) + 1;
      if (search(val, arqHASH))
        d->nb_found++;
      d->nb_contains++;
    }
  }
  fclose(arqHASH);
  return NULL;
}

int main(){

    FILE *arqHASH;
    int num=0,opc=0,i = 0;;
    //Test
    thread_data_t *data;
    pthread_t threads;
    rand_init(main_seed);
    if ((data = (thread_data_t *)malloc(1 * sizeof(thread_data_t))) == NULL) {
        perror("malloc");
        exit(1);
    }
    srand((int)time(NULL));
    data[0].range = 65536;
    data[0].update = 20;
    data[0].alternate = 1;
    rand_init(data[0].seed);
    
    //

    if((arqHASH = fopen("Hash.bin", "r+b")) == NULL){
        if((arqHASH = fopen("Hash.bin", "w+b")) == NULL){
            printf("File Error\n");
            exit(1);

        }
        set_new(arqHASH);
    }
    do{
		printf("Digite a opcao:\nAdd-->1\nRemove-->2\nSearch-->3\nTest-->4\nExit-->5\n");
		scanf("%d",&opc);
		switch(opc)
		{
			case 1:
        printf("Enter an integer: ");
        scanf("%d", &num);
        add(num,arqHASH);
        break;
			case 2:
        printf("Enter an integer: ");
        scanf("%d", &num);
        delete(num,arqHASH);
        break;
			case 3:
        printf("Enter an integer: ");
        scanf("%d", &num);
        if(search(num,arqHASH)){
          printf("Achou\n");
        }
        break;
      case 4:
        while(i<(data[0].range/2)){
            if (add(rand_range(data[0].range, main_seed), arqHASH)) {
                i++;
            }
        }
        int size = set_size(arqHASH);
        fclose(arqHASH);
        printf("STARTING...\n");
        if (pthread_create(&threads, NULL, test, &data[0]) != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
        sleep(5);
        stop = 1;
        printf("STOPPING...\n");
        pthread_join(threads, NULL);
        long unsigned int reads = data[0].nb_contains;
        long unsigned int updates = (data[0].nb_add + data[0].nb_remove);
        size+=data[0].diff;
        arqHASH = fopen("Hash.bin", "r+b");
        printf("  #add        : %lu\n", data[0].nb_add);
        printf("  #remove     : %lu\n", data[0].nb_remove);
        printf("  #contains   : %lu\n", data[0].nb_contains);
        printf("  #found      : %lu\n", data[0].nb_found);
        printf("Set size      : %d (expected: %d)\n", set_size(arqHASH), size);
        printf("#txs          : %lu (%f / s)\n", reads + updates, ((reads + updates)*1.0  / 5));
        printf("#read txs     : %lu (%f / s)\n", reads, (reads *1.0 / 5));
        printf("#update txs   : %lu (%f / s)\n", updates, (updates *1.0 / 5));
        break;
			case 5:
				fclose(arqHASH);
				exit(0);				
		}
    }while(1);

}


