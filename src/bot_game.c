#include "bot_game.h"

void print_list(pair_t *head) {
    pair_t* current = head;

    while (current != NULL) {
        printf("pos x: %i pos y: %i\n", current->play[0], current->play[1]);
        current = current->next_pair;
    }
    return;
}


pair_t* add_list(pair_t *head, int val[2]) {
    pair_t* new_pair, *current;

    new_pair = (pair_t*) malloc(sizeof(pair_t));
    new_pair->next_pair = NULL;
    memcpy(new_pair->play, val, 2*sizeof(int));

    if(head == NULL){
        head = new_pair;
    }else{
        current = head;
        while(current->next_pair != NULL){
            current = current->next_pair;
        }
        current->next_pair = new_pair;
    }
    return head;
}

pair_t* remove_by_index(pair_t **head, int n, int retval[2], int *max_disp){
    pair_t* current = *head;
    pair_t* temp_node = NULL;
	int i;

    if (n >= *max_disp || n < 0){
        printf("Index out of range\n");
        return *head;
    }

    if (n == 0) {       
        if (*head == NULL) {
            return *head;
        }
        temp_node = (*head)->next_pair;
        memcpy(retval, (*head)->play, 2*sizeof(int));
        free((pair_t*)*head);
        *head = temp_node;
        (*max_disp)--;
        return *head;
    }
    
    for (i = 0; i < n-1; i++) {
        if (current->next_pair == NULL){
			return *head;
        }
        current = current->next_pair;
    }

    temp_node = current->next_pair;
    memcpy(retval, temp_node->play, 2*sizeof(int));
    current->next_pair = temp_node->next_pair;
    free((pair_t *)temp_node);
    (*max_disp)--;

    return *head;
}

int search_index(pair_t *head, int pos[2]){
    pair_t *current = head;
    int ind = 0;

    while (current != NULL) {
        if (current->play[0]==pos[0] && current->play[1]==pos[1]){
			break;
        }else{
            current = current->next_pair;
            ind++;
        }
    }
    return ind;
}


pair_t* remove_by_value(pair_t *head, int pos[2], int retval[2], int *max_disp){
    int index;
    index = search_index(head, pos);
    printf("index %i: max_index: %i\n", index, *max_disp);

    if (index<0 || index >= *max_disp){
        printf("Didn't find the value in the list.\n");
    }else{
        head = remove_by_index(&head, index, retval, max_disp);
    }
    return head;
}


pair_t* init_list(int dim){
	pair_t *list = NULL;
    int play[2];
    int j, k;
    for (j=0; j<dim; j++){
        for (k=0; k<dim; k++){
            play[0]=j;
            play[1]=k;
            list = add_list(list, play);
        }
    }
    return list;
}

int is_blank(Card *card){
    return(card->color[0]==255 && card->color[1]==255 && card->color[2]==255);
}

int text_is_black(Card *card){
    return(card->tcolor[0] == 0 && card->tcolor[1] == 0 && card->tcolor[2] == 0);
}

int is_2nd_card(Memory *memory, Card *card, int i){
    return(memory[i].xy1[0]!= card->pos[0] || memory[i].xy1[1]!= card->pos[1]);
}

int is_match(Memory *memory, Card *card, int i){
    return(strcmp(memory[i].str, card->text)==0);
}

int find_play_method(Memory *memory, int num_pairs){
    int count = 0;

    for(count=0; count<num_pairs; count++){
        if (memory[count].ready && !(memory[count].sent)){
            return 0;
        }
    }
    return 1;
}




Memory* fill_memory_list(Memory *memory_bot, Card *card, int number_of_pairs, int *blank){
    int i = 0;

    if (text_is_black(card)){
        (*blank)--;
    }
    // Fills memory
    for (i = 0; i<number_of_pairs; i++){
        if(memory_bot[i].str[0]!=0){
            if (is_match(memory_bot, card, i)){             // MATCH
                if(is_2nd_card(memory_bot, card, i)){       // if it is the 2nd card

                    memory_bot[i].xy2[0] = card->pos[0];
                    memory_bot[i].xy2[1] = card->pos[1];

                    if (is_blank(card)){                // when second card is blank
                        memory_bot[i].ready = true;     // the pair is ready to be sent
					}

                    if (text_is_black(card)){           // when text is black
                        memory_bot[i].sent = true;      // means that some pair was completed then remove from the list
                    }
                }
                break;
            }
        }
		else{
            if (!is_blank(card)){
                memory_bot[i].xy1[0] = card->pos[0];
                memory_bot[i].xy1[1] = card->pos[1];
                memcpy(memory_bot[i].str, card->text , SIZE_TEXT*sizeof(char));
                break;
            }
        }   
    }
    return memory_bot;
}


void *receive_bot_events(void *arg){
    int board_dim = ((Bot_smart *)arg)->dim;
    int *max_list_index = ((Bot_smart *)arg)->max_list_index;
    int sock_fd = ((Bot_smart *)arg)->receive->sock_fd;
    int ui = ((Bot_smart *)arg)->receive->ui;
    Memory *memory = ((Bot_smart *)arg)->memory;
    pair_t** list = ((Bot_smart *)arg)->list;
    int smart_bot_mode = ((Bot_smart *)arg)->on;
    pthread_rwlock_t *rwlock = ((Bot_smart *)arg)->rwlock;

    int blank = board_dim*board_dim;
    int num_pairs = blank/2;

    Message *msg = newMessage(sock_fd);
    Card *card;
    char *end;

    // Receives one message (mainly cards)
    while(recvFromServer(msg)>0){
        switch(findType(msg)){
            case CARD:
				card = message2card(msg);
				// printClientCard(card);
				if(smart_bot_mode){
                    /* updates list of available plays*/
                    pthread_rwlock_wrlock(rwlock);
                    *list  = update_list(*list, card, max_list_index);

                    /* updates bot memory */            
                    memory = fill_memory_list(memory, card, num_pairs, &blank);

                    if (!blank){
                        *list = restart_game(memory, *list, num_pairs, board_dim, ui, msg, max_list_index);
                        printf("restart\n");
                        print_memory(memory, num_pairs);
                        print_list(*list);
                        blank = board_dim*board_dim;
                    }
                   
                    pthread_rwlock_unlock(rwlock);
                }
				
				
				if(ui){
					paint_card(card->pos, card->color);
					write_card(card->pos, card->text, card->tcolor);
				}
				free((Card *)card);
				break;

            case END:
				end = message2end(msg);
				end[SIZE_END-1] = '\0';
				printf("end = %s\n", end);
				free((char *)end);
				break;
        }
    }
    
    done = 1;
    free((Message *)msg);
    free((Memory *)memory);
    
    return NULL;
}

pair_t* update_list(pair_t *list, Card *card, int *max_list_index){
    int play[2];
    int pop_value[2];  // debuging
    if (!is_blank(card)){
        play[0] = card->pos[0];
        play[1] = card->pos[1];
        list = remove_by_value(list, play, pop_value, max_list_index);  // removes that pair from the list of pairs available
    }
    return list;
}

pair_t* restart_game(Memory *memory, pair_t* list, int pairs_num, int board_dim, int ui, Message *msg, int *max_index){

    memory = bot_memory_reset(memory, pairs_num);
    list = init_list(board_dim);
    *max_index = board_dim*board_dim;

    return list;
}


Memory* bot_memory_reset(Memory *memory, int max_p){
    memset(memory, 0 , max_p*sizeof(Memory));         // cleans everyting
    int i;
    for (i=0; i<max_p; i++){
        memset(memory[i].xy1, -1, 2*sizeof(int));     // fill positions of card1 with -1
        memset(memory[i].xy2, -1, 2*sizeof(int));     // fill positions of card2 with -1
    }
    return memory;
}


Memory* bot_memory_init(int max_p){
    Memory *b_memory = (Memory*) malloc(max_p*sizeof(Memory));
    b_memory = bot_memory_reset(b_memory, max_p);
    return b_memory;
}


void print_memory(Memory *memory_bot, int max_p){
    int i;
    for(i=0; i<max_p; i++){
        printf("string: %s pair: (%i, %i) , (%i, %i) , ready: %i, sent: %i\n", memory_bot[i].str, memory_bot[i].xy1[0], memory_bot[i].xy1[1], memory_bot[i].xy2[0], memory_bot[i].xy2[1], memory_bot[i].ready, memory_bot[i].sent);
    }
    return;
}


Memory* play_from_memory(Memory *memory, int max_pairs, int sock_fd, pthread_rwlock_t *rwlock){
    int play[2];
    int i;
	pthread_rwlock_rdlock(rwlock);
    for (i=0; i<max_pairs; i++){    // looking for an available pair in memory
        if (memory[i].ready && !memory[i].sent){
                play[0]=memory[i].xy1[0];
                play[1]=memory[i].xy1[1];
                sendToServer(sock_fd, play);
                play[0]=memory[i].xy2[0];
                play[1]=memory[i].xy2[1];
                sendToServer(sock_fd, play);
				break;
        }
    }
    pthread_rwlock_unlock(rwlock);
	usleep(SLEEP_TIME);
    return memory;
}

void play_from_list(pair_t **pair_list, int sock_fd, pthread_rwlock_t *rwlock, int *max_list_index){
    int play[2];
    pair_t* current_list;
    int pos;
    int max_pos;
	bool sleep = false;

    pthread_rwlock_rdlock(rwlock);

	max_pos = *max_list_index;

    if( *max_list_index <= 0){
        printf("EMPTY LIST\n");
    }
	else{
		pos = rand()%max_pos;

		for(current_list=*pair_list; pos>0; pos--){
			if (current_list->next_pair != NULL)
				current_list = current_list->next_pair;
			else
				break;
		}

		memcpy(play, current_list->play, 2*sizeof(int));
		sendToServer(sock_fd, play);
		sleep = true;
	}
		
    pthread_rwlock_unlock(rwlock);
    
	if(sleep)
		usleep(SLEEP_TIME);

    return;
}



void play_random(int sock_fd, int board_dim){
    int play[2];
    play[0]=rand()%board_dim;
    play[1]=rand()%board_dim;
    sendToServer(sock_fd, play);
    usleep(SLEEP_TIME);   //0.1s
}


void smart_bot_mode(Bot_smart *bot, int argc, char *argv[]){
    int fail = 1;
    if (argc>1){
        if(strcmp(argv[1], "1")==0){
            bot->on = 1;
            fail = 0;
            printf("SMART BOT MODE: ON\n");
        }else if(strcmp(argv[1], "0")==0){
            bot->on = 0;
            fail = 0;
            printf("SMART BOT MODE: OFF\n");
        }
    }
    if (fail){ // Default
        bot->on = 1;
        printf("SMART BOT MODE: DEFAULT (ON)\n");
    }
}

void UI_mode(int *mode, int argc, char *argv[], int dim){
    int fail = 1;
    if (argc>2){
        if (strcmp(argv[2], "0")==0){
            printf("UI MODE: OFF\n");
            *mode = 0;
            fail = 0;
        }else if(strcmp(argv[2], "1")==0){
            printf("UI MODE: ON\n");
            initUI(dim);
            *mode = 1;
            fail = 0;
        }
    }

    if (fail){
        printf("UI MODE: DEFAULT (ON)\n");
        initUI(dim);
        *mode = 1;
    }
}


void no_inputs(int argc){
    if(argc==1){
        printf("./bot <smart> <ui> <address> <port>\n");
    }
}
