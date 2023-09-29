// Roy Amit 209205665

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "producer.h"
#include "dispatcher.h"
#include "boundedBuffer.h"
#include "unboundedQueue.h"
#include "screenManager.h"

#define MAX_LENGTH 150
#define ERROR (-1)

// ******************** STRUCTS ********************

/**
 * A global struct that holds a queue - bounded
 */
typedef struct {
    char **articles;
    int front;
    int rear;
    int count;
    int size;
    sem_t mutex;
    sem_t full;
    sem_t empty;
} BoundedBuffer;

/**
 * A node representing an article in the linked list of the unbounded buffer.
 */
typedef struct Node {
    char *article;
    struct Node *next;
} Node;

/**
 * A global struct that holds an unbounded queue implemented as a linked list.
 */
typedef struct {
    Node *head;
    Node *tail;
    int count;
    sem_t mutex;
    sem_t empty;
} UnboundedBuffer;

/**
 * A global struct that holds the producer arguments
 */
typedef struct {
    int id_producers;
    int num_products;
    int queue_size;
    BoundedBuffer *buffer_ptr;
} Producer;

/**
 * A global struct that holds the dispatcher arguments
 */
typedef struct {
    BoundedBuffer **producer_queues;
    int num_producers;
    UnboundedBuffer *sports_queue;
    UnboundedBuffer *news_queue;
    UnboundedBuffer *weather_queue;
} Dispatcher;

/**
 * A global struct that holds the co-editor arguments
 */
typedef struct {
    UnboundedBuffer *dispatchersQueue;
    BoundedBuffer *sharedQueue;
} CoEditor;

/**
 * A global struct that holds the screen manager arguments
 */
typedef struct {
    BoundedBuffer *coeditorQueue;
} Screen;


// ***************** HELPING FUNCTIONS ******************

/**
 * Open the configuration file.
 * @param filename The name of the configuration file.
 * @return The file pointer for the opened file.
 */
FILE *open_config_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file\n");
        exit(ERROR);
    }
    return file;
}

/**
 * Count the number of multiple sets of numbers in a file.
 * @param file The file to be analyzed.
 * @return The number of multiple sets of numbers in the file.
 */
int count_multiple_sets(FILE *file) {
    int line_count = 0;
    int line;
    // Run over the lines in the file
    while (fscanf(file, "%d", &line) == 1) {
        line_count++;
    }
    rewind(file);
    // Return the number of multiple sets
    return line_count / 3;
}

/**
 * Generates a random type from the categories: "SPORTS", "WEATHER", "NEWS".
 * @return A randomly generated type.
 */
char *generate_type() {
    char *categories[] = {"SPORTS", "WEATHER", "NEWS"};
    // Generate a random index
    int index = rand() % 3;
    return categories[index];
}

/**
 * Extracts the type from the given message.
 * @param message The message string to extract the type from.
 * @return The extracted type
 */
char *extract_type_from_message(char *message) {
    if (strstr(message, "SPORTS") != NULL) {
        return "SPORTS";
    }
    else if (strstr(message, "WEATHER") != NULL) {
        return "WEATHER";
    }
    return "NEWS";
}

/**
 * Close the configuration file.
 * @param file The file pointer to be closed.
 */
void close_config_file(FILE *file) {
    if (fclose(file) != 0) {
        perror("Failed to close file\n");
        exit(ERROR);
    }
}


// **************** CREATING FUNCTIONS *******************

/**
 * Create a new bounded buffer with a specified size.
 * @param size The maximum number of places to store objects in the buffer.
 * @return A pointer to the newly created BoundedBuffer object.
 */
void create_bounded_buffer(BoundedBuffer* buffer, int size) {
    buffer->front = 0;
    buffer->rear = 0;
    buffer->count = 0;
    buffer->size = size;
    buffer->articles = (char **) malloc(size * sizeof(char *));
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->full, 0, 0);
    sem_init(&buffer->empty, 0, size);
}

/**
 * Create an unbounded buffer (linked list implementation).
 * @return A pointer to the created unbounded buffer.
 */
void create_unbounded_buffer(UnboundedBuffer *buffer) {
    buffer->count = 0;
    buffer->head = NULL;
    buffer->tail = NULL;
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->empty, 0, 0);
}

/**
 * Creates a producer object with the specified parameters.
 * @param producer Pointer to the Producer object to be created and initialized.
 * @param id_producers The ID of the producer.
 * @param num_products The number of products.
 * @param queue_size The size of the queue.
 */
void create_producer (Producer* producer ,int id_producers, int num_products, int queue_size) {
    producer->id_producers = id_producers;
    producer->num_products = num_products;
    producer->queue_size = queue_size;
    BoundedBuffer *buffer = (BoundedBuffer *) malloc(sizeof(BoundedBuffer));
    if(buffer == NULL) {
        exit(ERROR);
    }
    create_bounded_buffer(buffer, queue_size);
    producer->buffer_ptr = buffer;
}

/**
 * Creates producers based on input data from a file.
 * @param file The file pointer representing the input file.
 *             The file should be opened in read mode.
 * @param producer_args An array of pointers to Producer objects, used to store the created producers.
 * @param queue_coeditor_size A pointer to an integer representing the size of the coeditor queue.
 *                            The value will be updated based on the input data.
 */
void create_producers(FILE *file, Producer** producer_args, int* queue_coeditor_size) {
    int id_producers = 0;
    int num_products = 0;
    int queue_size = 0;
    int line;
    int num_producers = 0;
    int line_count = 0;

    while (fscanf(file, "%d", &line) == 1) {
        if (line_count % 3 == 0) {
            
            if (line_count >= 3) {
                num_producers++;
            }
        }
        // Create the producers arguments
        if (line_count % 3 == 0) {
            id_producers = line;
        } else if (line_count % 3 == 1) {
            num_products = line;
        } else if (line_count % 3 == 2) {
            queue_size = line;
            producer_args[num_producers] = malloc(sizeof(Producer));
            if (producer_args[num_producers] == NULL) {
                exit(ERROR);
            }
            create_producer(producer_args[num_producers], id_producers, num_products, queue_size);
        }
        line_count++;
    }
    // Catch the last line in the file - the coeditor queue size
    if (line_count % 3 == 1) {
        *queue_coeditor_size = id_producers;
    }
}


/**
 * Creates a dispatcher object with the specified parameters.
  * @param dispatcher A pointer to the Dispatcher object to be created and initialized.
 * @param producers_queues An array of pointers to BoundedBuffer objects representing the message queues
 *                         for each producer.
 * @param num_producers The number of producers.
 * @param sports_queue A pointer to the UnboundedBuffer representing the sports message queue.
 * @param news_queue A pointer to the UnboundedBuffer representing the news message queue.
 * @param weather_queue A pointer to the UnboundedBuffer representing the weather message queue.
 */
void create_dispatcher(Dispatcher *dispatcher, BoundedBuffer **producers_queues, int num_producers,
                       UnboundedBuffer *sports_queue, UnboundedBuffer *news_queue,
                       UnboundedBuffer *weather_queue) {
    dispatcher->num_producers = num_producers;
    dispatcher->news_queue = news_queue;
    dispatcher->sports_queue = sports_queue;
    dispatcher->weather_queue = weather_queue;
    dispatcher->producer_queues = producers_queues;
}

/**
 * Initializes a CoEditor object with the provided dispatchers queue and shared queue.
 * @param coEditor The CoEditor object to initialize.
 * @param dispatchersQueue The UnboundedBuffer representing the dispatchers queue to assign to the coeditor.
 * @param sharedQueue The BoundedBuffer representing the shared queue to assign to the coeditor.
 */
void create_coeditor(CoEditor* coEditor ,BoundedBuffer *sharedQueue, UnboundedBuffer* dispatchersQueue){
    coEditor->sharedQueue = sharedQueue;
    coEditor->dispatchersQueue = dispatchersQueue;
}

/**
 * Initializes a Screen object with the provided coeditor queue.
 * @param screen The Screen object to initialize.
 * @param coeditorQueue The BoundedBuffer representing the coeditor queue to assign to the screen.
 */
void create_screen(Screen *screen, BoundedBuffer *coeditorQueue) {
    screen->coeditorQueue = coeditorQueue;
}


// ************** HANDELING INSERTION AND REMOVE TO BUFFERS ****************

/**
* Insert a new object into the bounded buffer.
* @param buffer The BoundedBuffer object to insert the object into.
* @param item The object to be inserted into the buffer.
*/
void insert_to_bounded(BoundedBuffer *buffer, char *item) {
    sem_wait(&buffer->empty);
    sem_wait(&buffer->mutex);

    buffer->articles[buffer->rear] = strdup(item);
    buffer->rear = (buffer->rear + 1) % buffer->size;
    buffer->count++;

    sem_post(&buffer->mutex);
    sem_post(&buffer->full);
}

/**
 * Remove the first object from the bounded buffer and return it.
 * @param buffer The BoundedBuffer object to remove the object from.
 * @return The removed object from the buffer.
 */
char *remove_from_bounded(BoundedBuffer *buffer) {
    char *item;
    sem_wait(&buffer->full);
    sem_wait(&buffer->mutex);

    item = buffer->articles[buffer->front];
    buffer->front = (buffer->front + 1) % buffer->size;
    buffer->count--;

    sem_post(&buffer->mutex);
    sem_post(&buffer->empty);
    return item;
}

/**
 * Insert a message at the tail of the dispatcher's unbounded buffer (acting as a queue).
 * @param buffer The unbounded buffer (queue) to insert the message into.
 * @param message The message to be inserted.
 */
void insert_to_unbounded(UnboundedBuffer* buffer, char* message) {
    Node *newNode = (Node *) malloc(sizeof(Node));
    if(newNode == NULL) {
        exit(ERROR);
    }
    newNode->article = strdup(message);
    newNode->next = NULL;

    sem_wait(&buffer->mutex);
    if (buffer->head == NULL) {
        // Buffer is empty, set the new node as both head and tail
        buffer->head = newNode;
        buffer->tail = newNode;
    } else {
        // Append the new node to the tail
        buffer->tail->next = newNode;
        buffer->tail = newNode;
    }

    buffer->count++;
    sem_post(&buffer->mutex);
}

/**
 * Removes the item from the top of the linked list in the UnboundedBuffer.
 * This function retrieves the message from the head node and updates the buffer accordingly.
 * If the buffer is empty, it returns NULL.
 * @param buffer The UnboundedBuffer from which to remove the item.
 * @return The message removed from the buffer, or NULL if the buffer is empty.
 */
char* remove_from_unbounded(UnboundedBuffer* buffer) {
    sem_wait(&buffer->mutex);

    if (buffer->head == NULL) {
        sem_post(&buffer->mutex);  // Release the mutex
        return NULL;
    }

    // Retrieve the message from the head node
    Node *nodeToRemove = buffer->head;
    char *message = nodeToRemove->article;
    
    // Update the head pointer
    buffer->head = buffer->head->next;
    buffer->count--;

    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }

    free(nodeToRemove);
    sem_post(&buffer->mutex);

    return message;
}


// ************** THREADS FUNCTIONS *******************

/**
 * The thread function for the producer.
 * This function is responsible for generating and producing messages based on the specified number of products
 * for a producer. It generates a type for each product and creates a producer string based on the generated type
 * and the producer's ID. The producer string is then inserted into the producer's bounded buffer. After producing
 * all the products, a "DONE" message is inserted into the bounded buffer to indicate completion.
 * @param arg A pointer to the Producer object containing the necessary information for the producer thread.
 * @return This function returns NULL.
 */
void *producer_thread_function(void* arg) {
    Producer* args = (Producer*)arg;
    int count_news = 0;
    int count_sports = 0;
    int count_weather = 0;
    for (int i = 0; i < args->num_products; ++i) {
        char *type = generate_type();
        char producer_string[MAX_LENGTH];

        // Perform operations based on the generated type
        if (strcmp(type, "SPORTS") == 0) {
            snprintf(producer_string, sizeof(producer_string),
                     "Producer %d %s %d", args->id_producers, type, count_sports);
            count_sports++;
        } else if (strcmp(type, "WEATHER") == 0) {
            snprintf(producer_string, sizeof(producer_string),
                     "Producer %d %s %d", args->id_producers, type, count_weather);
            count_weather++;
        } else if (strcmp(type, "NEWS") == 0) {
            snprintf(producer_string, sizeof(producer_string),
                     "Producer %d %s %d", args->id_producers, type, count_news);
            count_news++;
        }
        insert_to_bounded(args->buffer_ptr, producer_string);
    }
    insert_to_bounded(args->buffer_ptr, "DONE");
    return NULL;
}

/**
 * Dispatcher thread function.
 * This function implements the logic for the dispatcher thread, which continuously accepts messages from the
 * producer queues and inserts them into the appropriate dispatcher queues based on their type.
 * @param arg The argument containing the dispatcher configuration and data.
 * @return NULL.
 */
void *dispatcher_thread_function(void *arg) {
    Dispatcher *args = (Dispatcher *) arg;
    int *producer_done_flags = (int *) malloc(args->num_producers * sizeof(int));
    if(producer_done_flags == NULL) {
        exit(ERROR);
    }
    for (int i = 0; i < args->num_producers; ++i) {
        producer_done_flags[i] = 0;
    }
    while (1) {
        for (int i = 0; i < args->num_producers; i++) {
            if (producer_done_flags[i] == 0) {
                char *message = remove_from_bounded(args->producer_queues[i]);
                if (strcmp(message, "DONE") == 0) {
                    producer_done_flags[i] = 1;
                    free(message);
                    continue;
                }
                char *type = extract_type_from_message(message);
                if (strcmp(type, "SPORTS") == 0) {
                    insert_to_unbounded(args->sports_queue, message);
                } else if (strcmp(type, "NEWS") == 0) {
                    insert_to_unbounded(args->news_queue, message);
                } else if (strcmp(type, "WEATHER") == 0) {
                    insert_to_unbounded(args->weather_queue, message);
                } 
                free(message);
            }
        }
        // Check if all producers are done
        int all_producers_done = 1;
        for (int i = 0; i < args->num_producers; i++) {
            if (!producer_done_flags[i]) {
                all_producers_done = 0;
                break;
            }
        }
        if (all_producers_done) {
            insert_to_unbounded(args->sports_queue, "DONE");
            insert_to_unbounded(args->news_queue, "DONE");
            insert_to_unbounded(args->weather_queue, "DONE");
            break;
        }
    }
    free(producer_done_flags);
    return NULL;
}

/**
 * Thread function for a coeditor.
 * @param arg A pointer to the CoEditor object representing the coeditor's arguments.
 * @return NULL.
 */
void *coeditor_thread_function(void *arg) {
    CoEditor *args = (CoEditor *)arg;
    while (1) {
        char *message = remove_from_unbounded(args->dispatchersQueue);
        if (message == NULL) {
            continue;
        }
        if(strcmp(message, "DONE") == 0) {
            // Pass the "DONE" message without waiting
            insert_to_bounded(args->sharedQueue, message);
            free(message);
            return NULL;
        } else {
            insert_to_bounded(args->sharedQueue, message);
        }
        usleep(100000);
        free(message);
    }
}

/**
 * Thread function for a screen.
 * @param arg A pointer to the Screen object representing the screen's arguments.
 * @return NULL.
 */
void* screen_thread_function(void *arg) {
    Screen *args = (Screen *) arg;
    int done_count = 0;
    while (1) {
        // Remove message from the shared queue
        char* message = remove_from_bounded(args->coeditorQueue);
        if (strcmp(message, "DONE")==0) {
            done_count++;
            if(done_count == 3) {
                printf("%s", message);
                free(message);
                return NULL;
            }
        } else {
            // Print message to the screen
            printf("%s\n", message);
        }
        free(message);
    }
    return NULL;
}


/**
 * Retrieves the bounded buffers associated with each producer.
 * @param producers_args An array of Producer objects.
 * @param producers_queues An array of BoundedBuffer pointers to store the retrieved buffer pointers.
 * @param size The size of the arrays.
 */
void get_producers_queues(Producer** producers_args, BoundedBuffer** producers_queues, int size) {
    // Handeling options of null arguments
    if (producers_args == NULL || producers_queues == NULL) {
        return;
    }
    for (int i = 0; i < size; i++) {
        if (producers_args[i] == NULL || producers_args[i]->buffer_ptr == NULL) {
            continue;
        }
        producers_queues[i] = producers_args[i]->buffer_ptr;
    }
}


// ********** HANDELING FREE MEMORY AND DESTROY SEMAPHORES *************

void free_bounded_Buffer(BoundedBuffer* buffer) {
    sem_destroy(&buffer->mutex);
    sem_destroy(&buffer->full);
    sem_destroy(&buffer->empty);
    free(buffer->articles);
}

void free_unbounded_Buffer(UnboundedBuffer* buffer) {
    sem_destroy(&buffer->mutex);
    Node* currentNode = buffer->head;
    while (currentNode != NULL) {
        Node* next = currentNode->next;
        free(currentNode->article);
        free(currentNode);
        currentNode = next;
    }
}

/**
 * This is the main function
 */
int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "There are more are less than 1 argument - the config.txt file");
        return ERROR;
    }

    // File:
    const char *filename = argv[1];
    FILE *file = open_config_file(filename);

    // Producer:
    int num_producers = count_multiple_sets(file);
    Producer **producers_args = malloc(num_producers * sizeof(Producer*));
    if(producers_args == NULL) {
        exit(ERROR);
    }

    int *queue_coeditor_size = malloc(sizeof(int));
    if(queue_coeditor_size == NULL) {
        exit(ERROR);
    }
    create_producers(file, producers_args, queue_coeditor_size);
    close_config_file(file);

    // Dispatcher:
    Dispatcher *dispatcher_args = malloc(sizeof(Dispatcher));
    BoundedBuffer **producers_queues = malloc(num_producers * sizeof(BoundedBuffer*));
    UnboundedBuffer *sports_queue = malloc(sizeof(UnboundedBuffer));
    UnboundedBuffer *weather_queue = malloc(sizeof(UnboundedBuffer));
    UnboundedBuffer *news_queue = malloc(sizeof(UnboundedBuffer));
    if(dispatcher_args == NULL || producers_queues == NULL || sports_queue == NULL || news_queue == NULL || weather_queue == NULL) {
        exit(ERROR);
    }

    create_unbounded_buffer(sports_queue);
    create_unbounded_buffer(news_queue);
    create_unbounded_buffer(weather_queue);

    // Populate the producers_args and the dispatcher_args
    get_producers_queues(producers_args, producers_queues, num_producers);
    create_dispatcher(dispatcher_args, producers_queues, num_producers, news_queue, sports_queue, weather_queue);

    // Co-Editor:
    BoundedBuffer *sharedQueue = malloc(sizeof(BoundedBuffer));
    CoEditor *sports_coeditor = malloc(sizeof(CoEditor));
    CoEditor *news_coeditor = malloc(sizeof(CoEditor));
    CoEditor *weather_coeditor = malloc(sizeof(CoEditor));
    if(sharedQueue == NULL || sports_coeditor == NULL || news_coeditor == NULL || weather_coeditor == NULL) {
        exit(ERROR);
    }
    create_bounded_buffer(sharedQueue,*queue_coeditor_size);
    create_coeditor(sports_coeditor, sharedQueue, sports_queue);
    create_coeditor(news_coeditor, sharedQueue, news_queue);
    create_coeditor(weather_coeditor, sharedQueue, weather_queue);

    // Screen Manager:
    Screen* screen = malloc(sizeof(Screen));
    if(screen == NULL) {
        exit(ERROR);
    }
    create_screen(screen, sharedQueue);

    // Threads creation:
    pthread_t** producer_threads = malloc(num_producers * sizeof(pthread_t*));
    if(producer_threads == NULL) {
        exit(ERROR);
    }
    pthread_t sports_coeditor_thread;
    pthread_t news_coeditor_thread;
    pthread_t weather_coeditor_thread;
    pthread_t dispatcher_thread;
    pthread_t screen_thread;

    // Create producers threads:
    for (int i = 0; i < num_producers; i++) {
        producer_threads[i] = malloc(sizeof(pthread_t));
        if(producer_threads[i] == NULL) {
            exit(ERROR);
        }
    }
    for (int i = 0; i < num_producers; i++) {
        if (pthread_create(producer_threads[i], NULL, producer_thread_function, (void *)producers_args[i]) != 0) {
            perror("Failed in creating producers threads");
            exit(ERROR);
        }
    }
    // Create dispatcher thread:
    if (pthread_create(&dispatcher_thread, NULL, dispatcher_thread_function, (void *)dispatcher_args) !=0 ){
        perror("Failed in creating dispatcher thread");
        exit(ERROR);
    }
    // Create co-editors threads:
    if (pthread_create(&sports_coeditor_thread, NULL, coeditor_thread_function, (void *)sports_coeditor) !=0 ){
        perror("Failed in creating sports coeditor thread");
        exit(ERROR);
    }
    if (pthread_create(&news_coeditor_thread, NULL, coeditor_thread_function, (void *)news_coeditor) != 0){
        perror("Failed in creating news coeditor thread");
        exit(ERROR);
    }
    if (pthread_create(&weather_coeditor_thread, NULL, coeditor_thread_function, (void*)weather_coeditor) !=0 ){
        perror("Failed in creating weather coeditor thread");
        exit(ERROR);
    }
    //Create screen thread:
    if (pthread_create(&screen_thread, NULL, screen_thread_function, (void *) screen) !=0 ){
        perror("Failed in creating screen thread");
        exit(ERROR);
    }

    // Wait for producers threads to complete:
    for(int i = 0; i < num_producers; i++) {
        if(pthread_join(*producer_threads[i], NULL)!=0) {
            exit(-1);
        }
    }

    // Wait for dispatcher thread to complete:
    if(pthread_join(dispatcher_thread, NULL) != 0) {
        perror("Failed in joining dispatcher thread");
        exit(ERROR);
    }
    // Wait for co-editor threads to complete:
    if(pthread_join(sports_coeditor_thread, NULL) != 0) {
        perror("Failed in joining sports coeditor thread");
        exit(ERROR);
    }
    if(pthread_join(news_coeditor_thread, NULL) != 0) {
        perror("Failed in joining news coeditor thread");
        exit(ERROR);
    }
    if(pthread_join(weather_coeditor_thread, NULL) != 0) {
        perror("Failed in joining weather coeditor thread");
        exit(ERROR);
    }
    // Wait for screen thread to complete:
    if(pthread_join(screen_thread, NULL) !=0 ) {
        perror("Failed in joining screen thread");
        exit(ERROR);
    }

    // Free allocated memory from producers:
    for (int i = 0; i < num_producers; i++) {
        free(producer_threads[i]);
    }
    free(producer_threads);

    for (int i = 0; i < num_producers; i++) {
        free_bounded_Buffer(producers_args[i]->buffer_ptr);
        free(producers_args[i]->buffer_ptr);
        free(producers_args[i]);
    }
    free(producers_args);
    free(producers_queues);

    // Free allocated memory from dispatcher:
    free(dispatcher_args);
    free_unbounded_Buffer(sports_queue);
    free_unbounded_Buffer(news_queue);
    free_unbounded_Buffer(weather_queue);
    free(sports_queue);
    free(news_queue);
    free(weather_queue);

    // Free allocated memory from coeditor:
    free_bounded_Buffer(sharedQueue);
    free(sharedQueue);
    free(sports_coeditor);
    free(news_coeditor);
    free(weather_coeditor);
    free(queue_coeditor_size);

    // Free allocated memory from screen:
    free(screen);

    return 0;
}