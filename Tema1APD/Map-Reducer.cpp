#include "Map-Reducer.h"

//#define _DEBUG_USER /*comment this if u don't want logs in console*/

/*static functions area*/
static inline void set_file_number(ifstream &file, unsigned int &number_of_input_files);

static bool process_arguments(int argc, char *argv[], int &number_of_mapers,
                              int &number_of_reducers, std::string &main_file_name);

static inline bool alloc_memory(pthread_t                   *&all_threads,
                                mapper_thread_type_data     *&mapper_threads_data,
                                reducer_thread_type_data    *&reducer_thread_data,
                                int number_of_mapers,       int number_of_reducers,
                                std::deque<std::string>     *list_of_input_files,
                                volatile unsigned int       *file_counter,
                                volatile unsigned int       *lists_reduced_counter,
                                pthread_mutex_t             *mapers_mutex,
                                pthread_mutex_t             *reducer_mutex,
                                pthread_barrier_t           *barrier_start_reduce);
static inline void init_reducer_list_of_data(mapper_thread_type_data *& maper_threads_data,
                                        reducer_thread_type_data *& reducer_threads_data,
                                        int number_of_mapers,
                                        int number_of_reducers);


/*end of the static functions*/

inline double simple_pow(double number, double exponent)
{
    double _POW(1);
    for (int i = 0; i < exponent; ++i)
        _POW *= number;
    return _POW;
}

inline double n_root(double number, double exponent)
{
    double x, dx;
    double A = number;
    double epsilon(10e-5);

    x = number * 0.5;
    dx = (number / simple_pow(x, exponent-1) - x) / exponent;
    while (abs(dx) >= epsilon)
    {
        x += dx;
        dx = (number / simple_pow(x, exponent-1) - x) / exponent;
    }
    return x;
}

inline bool get_possible_root(int input_number, int exponent, int &base_integer_value)
{
    try {
        double base = n_root(input_number, exponent);
        double rounded_base = round(base);
        double diff = abs(base - rounded_base);
        if (diff < ACCEPTED_ERROR) {
            base_integer_value = (int)(rounded_base);
            return true;
        }
    }
    catch (...)
    {
#ifdef _DEBUG_USER
        fprintf(stdout, "Exception here: %d\n", base_integer_value);
#endif
    }
    return false;
}
inline bool is_power_of_base(int number, int base, int &exponent)
{
    if (number == 1)
        return true;
    while (number % base == 0){
        number /= base;
    }
    return (number == 1);
}
inline void addToAllLists(int number, power_list * const&list_of_data)
{
    for(int i = 0; i < TOTAL_EXPONENTS; i++)
        list_of_data[i].push_back(number);
}
inline void addInList(int number, int exponent, power_list * const&list_of_data)
{
    switch (exponent) {
        case 2:
            list_of_data[POW2].push_back(number);
            break;
        case 3:
            list_of_data[POW3].push_back(number);
            break;
        case 4:
            list_of_data[POW4].push_back(number);
            break;
        case 5:
            list_of_data[POW5].push_back(number);
            break;
        default:
            break;
    }
}
void process_input_file(ifstream &input_file, power_list * const&crt_thread_lists)
{
    std::list<int> list_of_numbers;
    std::string line;
    int crt_number = 0;
    int exponent = 0;
    int base = 0;
    int number_of_lines = 0;
    if (input_file.is_open()){
        getline(input_file, line);
        number_of_lines = atoi(line.c_str());
        while (getline(input_file, line)){
            crt_number = atoi(line.c_str());
            if (crt_number == 1){
                /*add in every list -> save some processing time*/
                addToAllLists(crt_number, crt_thread_lists);
                continue;
            }
            for (exponent = MIN_EXPONENT_VALUE; exponent <= MAX_EXPONENT_VALUE; exponent++)
            {
                if (get_possible_root(crt_number, exponent, base) && base != 0)
                {
                    if (is_power_of_base(crt_number, base, exponent)) {
#ifdef _DEBUG_USER
                        fprintf(stdout, "MAGIC NUMBER: %d base%d exponent: %d\n", crt_number, base, exponent);
#endif
                        addInList(crt_number, exponent, crt_thread_lists);
                        base = 0;
                    }
                }
            }
        }
    }
}

void read_from_main_file(ifstream &file, unsigned int number_of_input_files,
                         std::deque<std::string>* &list_of_input_files)
{
    std::string line;
    //ifstream current_file;

#ifdef _DEBUG_USER
    fprintf(stdout,"nr de input files: %d\n", number_of_input_files);
#endif

    for (int i = 1; i <= number_of_input_files && getline(file, line); i++)
    {

#ifdef _DEBUG_USER
        fprintf(stdout,"file names: %s\n", line.c_str());
#endif
        //current_file.open(line);
        list_of_input_files->push_back(line);
        //process_input_file(current_file);
        //current_file.close();
    }
}

static inline void set_file_number(ifstream &file, unsigned int &number_of_input_files)
{
    std::string line;
    /*get the number of mapping files*/
    getline(file, line);
    number_of_input_files = atoi(line.c_str());
}
static bool process_arguments(int argc, char *argv[], int &number_of_mapers,
                              int &number_of_reducers, std::string &main_file_name)
{
    /*get arguments*/
    if (argc != 4) {
        fprintf(stdout, "Input data not available\n");
        return false;
    }
    else
    {
        number_of_mapers = atoi(argv[1]);
        number_of_reducers = atoi(argv[2]);
        main_file_name += argv[3];
#ifdef _DEBUG_USER
        fprintf(stdout, "Input data: %d %d %s\n",
                number_of_mapers, number_of_mapers, main_file_name.c_str());
#endif
    }
    return true;
}
static inline bool alloc_memory(pthread_t                   *&all_threads,
                                mapper_thread_type_data     *&mapper_threads_data,
                                reducer_thread_type_data    *&reducer_thread_data,
                                int number_of_mapers,       int number_of_reducers,
                                std::deque<std::string>     *list_of_input_files,
                                volatile unsigned int       *file_counter,
                                volatile unsigned int       *lists_reduced_counter,
                                pthread_mutex_t             *mapers_mutex,
                                pthread_mutex_t             *reducer_mutex,
                                pthread_barrier_t           *barrier_start_reduce)
{
    all_threads = new pthread_t[number_of_mapers + number_of_reducers];
    if (!all_threads) {
#ifdef _DEBUG_USER
        fprintf(stdout, "Threads alloc Failed");
#endif
        return false;
    }
    mapper_threads_data = new mapper_thread_type_data[number_of_mapers];
    if (!mapper_threads_data){
#ifdef _DEBUG_USER
        fprintf(stdout, "Maper threads data alloc Failed");
#endif
    return false;
    }
    for (int i = 0; i < number_of_mapers; i++)
    {
        mapper_threads_data[i].input_files = list_of_input_files;
        mapper_threads_data[i].file_counter = file_counter;
        mapper_threads_data[i].mapers_mutex = mapers_mutex;
        mapper_threads_data[i].barrier_start_reduce = barrier_start_reduce;
    }
    reducer_thread_data = new reducer_thread_type_data[number_of_reducers];
    if (!reducer_thread_data){
#ifdef _DEBUG_USER
        fprintf(stdout, "Reducer threads data alloc Failed");
#endif
        return false;
    }
    for (int i = 0; i < number_of_reducers; i++) {
        reducer_thread_data[i].addresses_of_lists = new power_list *[number_of_mapers];
        if (!reducer_thread_data->addresses_of_lists) {
#ifdef _DEBUG_USER
            fprintf(stdout, "Reducer threads data addr lists alloc Failed");
#endif
            return false;
        }
    }
    for (int red_idx = 0; red_idx < number_of_reducers; red_idx++) {
        reducer_thread_data[red_idx].number_of_mappers = number_of_mapers;
        reducer_thread_data[red_idx].reducer_mutex = reducer_mutex;
        reducer_thread_data[red_idx].barrier_start_reduce = barrier_start_reduce;
        reducer_thread_data[red_idx].lists_reduced_counter = lists_reduced_counter;
        for (int map_idx = 0; map_idx < number_of_mapers; map_idx++) {
            //reducer_thread_data.addresses_of_lists[red_idx][map_idx] = (mapper_threads_data[map_idx].list_pow);
            reducer_thread_data[red_idx].addresses_of_lists[map_idx] = mapper_threads_data[map_idx].list_pow;
            /*every reducer has to save the start address of the vector of lists of every mapper*/
        }
    }

    return true;
}

void* paralel_process_map(void *arg)
{
    mapper_thread_type_data *thr_data = ((mapper_thread_type_data*)arg);
    thr_data->thread_id = pthread_self();
    power_list *thread_lists = thr_data->list_pow;
    ifstream current_file;
    std::string crt_file_name;
    std::deque<std::string> list_of_input_files = *(thr_data->input_files);
    volatile unsigned int * &file_counter = thr_data->file_counter;

    while (*file_counter != list_of_input_files.size()) {
        pthread_mutex_lock(thr_data->mapers_mutex);
        /*synchronyze the acceses to the file names*/
        if (*file_counter >= list_of_input_files.size())
        {
            /*while the thread waits, the file_counter may be updated and can overflow*/
            pthread_mutex_unlock(thr_data->mapers_mutex);
            break;
        }
        crt_file_name = list_of_input_files[*file_counter];
        (*file_counter)++;
        pthread_mutex_unlock(thr_data->mapers_mutex);

        current_file.open(crt_file_name);
        process_input_file(current_file, thread_lists);
        current_file.close();
    }
    pthread_barrier_wait(thr_data->barrier_start_reduce);
    return NULL;
}

void* paralel_process_reduce(void *arg)
{
    reducer_thread_type_data *thr_data = ((reducer_thread_type_data*)arg);
    thr_data->thread_id = pthread_self();
    std::deque<int> crt_list;
    std::unordered_set<int> unique_numbers;
    ofstream out_file;
    std::string out_file_name;
    int exponent;
    volatile unsigned int * &lists_reduced_counter = thr_data->lists_reduced_counter;
    /*wait for all mapers to end*/
    pthread_barrier_wait(thr_data->barrier_start_reduce);

    while (*lists_reduced_counter < TOTAL_EXPONENTS)
    {
        unique_numbers.clear();

        pthread_mutex_lock(thr_data->reducer_mutex);

        if (*lists_reduced_counter >= TOTAL_EXPONENTS)
        {
            /*while the thread waits, the lists_reduced_counter may be updated and can overflow*/
            pthread_mutex_unlock(thr_data->reducer_mutex);
            break;
        }
        out_file_name.clear();
        out_file_name += "out" + to_string(*lists_reduced_counter + MIN_EXPONENT_VALUE) + ".txt";
        exponent = *lists_reduced_counter;
        (*lists_reduced_counter)++;
        pthread_mutex_unlock(thr_data->reducer_mutex);

        for (int map_idx = 0; map_idx < thr_data->number_of_mappers; map_idx++) {
            /*RAR is not a conflict*/

#ifdef _DEBUG_USER
            fprintf(stdout, "size of the list: %lu %d %d %d\n",
                    thr_data->addresses_of_lists[map_idx][exponent].size(), map_idx, exponent, thr_data->thread_id);
#endif
            crt_list = thr_data->addresses_of_lists[map_idx][exponent];

            for(int elem : crt_list)
                unique_numbers.insert(elem);
        }

        out_file.open(out_file_name);
        if (out_file.is_open()) {
            out_file << unique_numbers.size();
            out_file.close();
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int     number_of_mapers, number_of_reducers = 0;
    std::string                 main_file_name;
    ifstream                    main_file;
    pthread_t                   *all_threads;
    pthread_t                   *reducer_threads;
    mapper_thread_type_data     *maper_threads_data;
    reducer_thread_type_data    *reducer_threads_data;

    unsigned int                number_of_input_files = 0;
    volatile unsigned int       file_counter = 0;
    volatile unsigned int       lists_reduced_counter = 0;
    std::deque<std::string>     list_of_input_files;
    std::deque<std::string>     *address_of_input_files = &list_of_input_files; //left operand for passing it to the refences

    pthread_mutex_t mapers_mutex;
    pthread_mutex_t reducer_mutex;
    pthread_barrier_t barrier_start_reduce;

    /*init mutexes and barrier for synchronization between threads*/

    if (process_arguments(argc, argv, number_of_mapers, number_of_reducers, main_file_name) == false)
        return 0;

    pthread_mutex_init(&mapers_mutex, NULL);
    pthread_mutex_init(&reducer_mutex, NULL);
    /*In order to make sure that reduceres are starting after mappers, all processing threads must meet the barrier*/
    pthread_barrier_init(&barrier_start_reduce, NULL, number_of_mapers + number_of_reducers);

    /*alloc memory for the threads*/
    if (alloc_memory(all_threads, maper_threads_data,
                 reducer_threads_data, number_of_mapers, number_of_reducers,
                 &list_of_input_files, &file_counter, &lists_reduced_counter,
                 &mapers_mutex, &reducer_mutex, &barrier_start_reduce) == 0)
        return 0;

    /*open file*/
    main_file.open(main_file_name);

    if (main_file.is_open()) {
        set_file_number(main_file, number_of_input_files);
        read_from_main_file(main_file, number_of_input_files, address_of_input_files);
        main_file.close();
    }
    if (list_of_input_files.size() > 1)
    {
        for(int i = 0; i < number_of_mapers + number_of_reducers; i++){
            if (i < number_of_mapers)
                pthread_create(&(all_threads[i]), NULL, &paralel_process_map, (void*)&(maper_threads_data[i]));
            else
                pthread_create(&(all_threads[i]), NULL, &paralel_process_reduce, (void*)&(reducer_threads_data[i - number_of_mapers]));

        }
        for(int i = 0; i < number_of_mapers + number_of_reducers; i++){
            pthread_join(all_threads[i], NULL);
        }

#ifdef _DEBUG_USER
        for(int i = 0; i < number_of_mapers; i++)
        {
            for (int j = 0; j < TOTAL_EXPONENTS; j++) {
                fprintf(stdout, "Lista %d a threadului %d:", j, maper_threads_data[i].thread_id);
                while (!maper_threads_data[i].list_pow[j].empty()) {
                    int elem = maper_threads_data[i].list_pow[j].back();
                    maper_threads_data[i].list_pow[j].pop_back();
                    fprintf(stdout, " %d,", elem);
                }
                fprintf(stdout, "\n");
            }
        }
#endif
    }

    if (all_threads)
        delete [] all_threads;
    if (maper_threads_data)
        delete [] maper_threads_data;
    for (int i = 0; i < number_of_reducers; i++)
    {
        if (reducer_threads_data[i].addresses_of_lists)
            delete [] reducer_threads_data[i].addresses_of_lists;
    }
    if (reducer_threads_data)
        delete [] reducer_threads_data;

    pthread_mutex_destroy(&mapers_mutex);
    pthread_mutex_destroy(&reducer_mutex);
    pthread_barrier_destroy(&barrier_start_reduce);
    return 0;
}


