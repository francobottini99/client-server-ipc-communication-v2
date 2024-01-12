#ifndef __SERVER_THREADS_HANDLE_H__
#define __SERVER_THREADS_HANDLE_H__

#include "common.h"

/**
 * @brief Create new handle thread for a new client
 * 
 */
pthread_t* handler_create(void);

/**
 * @brief Destroy a handle thread
 * 
 * @param tid Thread id
 */
void handler_destroy(pthread_t tid);

/**
 * @brief Destroy all handle threads
 * 
 */
void handler_destroy_all(void);

/**
 * @brief Get last handle thread in list
 * 
 * @return struct th_node* Last handle thread
 */
struct th_node* handler_get_last(void);

/**
 * @brief Get handle thread by thread id
 * 
 * @param tid Thread id
 * @return struct th_node* Handle thread
 */
struct th_node* handler_get_by_tid(pthread_t tid);

/**
 * @brief Get handle thread parent
 * 
 * @param th Handle thread
 * @return struct th_node* Parent handle thread
 */
struct th_node* handler_get_parent(struct th_node* th);

/**
 * @brief Wait for all handle threads to finish
 * 
 */
void handler_wait_all(void);

#endif // __SERVER_THREADS_HANDLE_H__