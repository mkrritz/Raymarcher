#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/*
    This only supports up to 64 shaders. (enum struct Shaders)
    But if and when there are more than 64 shaders
    then maybe just add another variable and be done with it.
*/
extern volatile uint64_t shader_slots_to_reload;

//extern pthread_mutex_t mutex_1;

#endif