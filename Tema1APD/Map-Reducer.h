#ifndef _MAP_REDUCER_H
#define _MAP_REDUCER_H

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <pthread.h>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <math.h>
#include <deque>

#define MAX_EXPONENT_VALUE 5
#define MIN_EXPONENT_VALUE 2
#define TOTAL_EXPONENTS 4
#define ACCEPTED_ERROR 0.1f

using namespace std;

typedef std::deque<int> power_list;

typedef enum {
    POW2,
    POW3,
    POW4,
    POW5
}EXPONENTS;

typedef struct
{
    int                     thread_id;
    pthread_mutex_t         *mapers_mutex;
    pthread_barrier_t       *barrier_start_reduce;
    std::deque<std::string> *input_files;
    volatile unsigned int   *file_counter;              /*used for counting between the files with data to be maped*/
    power_list              list_pow[TOTAL_EXPONENTS];
}mapper_thread_type_data;

typedef struct
{                                      
    power_list              ** addresses_of_lists;  //list which contains the start addresses of every power list
    int                     thread_id;
    int                     number_of_mappers;
    volatile unsigned int   *lists_reduced_counter;
    pthread_mutex_t         *reducer_mutex;
    pthread_barrier_t       *barrier_start_reduce;
}reducer_thread_type_data;

/*PROTOTYPES AREA*/
/*****************/
extern void read_from_main_file(ifstream &file, unsigned int number_of_input_files,
                                std::deque<std::string>* &list_of_input_files);
extern inline double simple_pow(double number, double exponent);
extern inline double n_root(double number, double exponent);
extern inline bool is_power_of_base(int number, int exponent); /* Number processing function*/
extern inline bool get_possible_root(int input_number, int exponent, int &base_integer_value);
extern void process_input_file(ifstream &input_file, power_list* const& crt_thread_lists);
extern void* paralel_process_map(void *arg);
extern void* paralel_process_reduce(void *arg);
extern inline void addInList(int number, int exponent, power_list* const& list_of_data);
extern inline void addToAllLists(int number, power_list* const& list_of_data);
/*--*/
/*****************/
#endif