#include <limits>
#include <stdexcept>
#include <thread>
#include <mutex>

class hierarchical_mutex{
    private:
    std::mutex internal_mutex;
    unsigned int const this_lock_hrchy_value;
    static thread_local unsigned int this_thread_current_level;
    unsigned int this_thread_previous_level{};

    public:
    hierarchical_mutex(unsigned int _hrchy_value) : this_lock_hrchy_value(_hrchy_value){}
    void check_update_this_lock_hrchy_value(unsigned int this_lock_hrchy_value){
        this_thread_previous_level=this_thread_current_level;
        this_thread_current_level=this_lock_hrchy_value;

    }
    void lock(){
        if(this_thread_current_level>this_lock_hrchy_value){
            internal_mutex.lock();
            check_update_this_lock_hrchy_value(this_lock_hrchy_value);
        }else{
            throw std::logic_error("Hierarchy Violated!");
        }
    }
    void unlock(){
        if(this_thread_current_level==this_lock_hrchy_value){
            internal_mutex.unlock();
            this_thread_current_level=this_thread_previous_level;
        }else{
            throw std::logic_error("Unlocking order should be LIFO");
        }
    }
};
thread_local unsigned int hierarchical_mutex::this_thread_current_level = 
    std::numeric_limits<unsigned int>::max();

int main(){

}
