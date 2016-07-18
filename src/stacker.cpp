// Local Variables:
// compile-command: "g++ -o stacker -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb stacker.cpp -DINCLUDE_MAIN"
// End:
   
#include "stacker.hpp"
#include <string.h>
#include <assert.h>

size_t Stacker::StringSet::length(void) const
{
   size_t len = 0;
   for (int i=0; i<m_count; ++i)
      len += 1 + strlen(m_pp_strings[i]);

   return len;
}

void Stacker::StringSet::copy(char *buffer) const
{
   for (int i=0; i<m_count; ++i)
   {
      const char *str = m_pp_strings[i];
      size_t len = 1+strlen(str);
      memcpy(buffer,str,len);
      buffer += len;
   }
}


void Stacker::initialize_new_line(size_t size_of_data, line_handle bottom, line_handle newline, int count)
{
   if (bottom)
      get_overlay(bottom)->p = newline;

   // Zero data
   char *data = static_cast<char*>(get_pointer_to_data(size_of_data, newline));
   memset(data,0,newline-data);
   
   Overlay *overlay = get_overlay(newline);
   overlay->c = count;
   overlay->p = nullptr;
}

Stacker::line_handle Stacker::find_line(const char *name) const
{
   line_handle h = m_head_line;
   while (h)
   {
      if (strcmp(h,name)==0)
         return h;

      h = get_next_line(h);
   }

   return nullptr;
}

#ifdef INCLUDE_MAIN

#include <stdio.h>

const char *namelist[4] = {"Zero", "One", "Two", "Three" };
const char *vallist[4] = {"Daddy", "Mommy", "Girl", "Boy" };

bool stringset_getter(int index, Stacker::StringSet_User su)
{
   if (index<4)
   {
      const char *list[2] = { namelist[index], vallist[index] };
      Stacker::StringSet ss(2,list);
      su(ss);
      return true;
   }
   else
      return false;
}

void LongInt_Stacker_User(StackerT<long int> &s)
{
   printf("Got to LongInt_Stacker_User!\n");
   long int *val;
   if ((val=s.find("One")))
      printf("Found %ld value for \"One\".\n", *val);

   if ((val=s.find("Three")))
      printf("Found %ld value for \"Three\".\n", *val);
}


class I_Nextor
{
public:
   virtual bool operator()(void) = 0;
};

class Nextor : public I_Nextor
{
private:
   long int &m_index;
   long int m_times; 
public:
   Nextor(long int &index, long int times)
      : m_index(index), m_times(times) { }
   
   virtual bool operator()(void) { return m_index++<m_times; }
};

void iterate_nextor(I_Nextor &n)
{
   int count = 0;
   while (n())
      printf("iterate_nextor %02d\n", ++count);
}


int main(int argc, char **argv)
{
   long int index=0;

   Nextor n(index, 5);
   iterate_nextor(n);

   

// //   Stacker::StringSet_Getter:
//    auto getter =
//       [&index] (Stacker::StringSet_User su)->void { stringset_getter(index, su); };

// //   Stacker::StringSet_Nexter
//    auto nexter =
//       [&index] (void)->bool { return (++index<4); };
   
// //   StackerT<long int>::Stacker_Data_Setter
//    auto setter =
//       [&index] (long int *pint)->void { *pint = index; };
   
//    StackerT<long int>::build(getter, nexter, setter, LongInt_Stacker_User);
   return 0;
}



#endif // INCLUDE_MAIN
