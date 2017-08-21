// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_MAIN -o linebuffer linebuffer.cpp" -*-

#ifndef LINEBUFFER_HPP
#define LINEBUFFER_HPP

#include "prandstr.hpp" // for EFFC_3

namespace linebuffer
{

/** Interface class to allow main function to be defined in .cpp file. */
class I_LBCallback
{
public:
   virtual ~I_LBCallback() { }
   virtual void operator()(const char* line) const = 0;
};

/** Template class to instantiate a I_LBCallback class. */
template <class Func>
class LBCallback : public I_LBCallback
{
protected:
   const Func &m_f;
public:
   LBCallback(const Func &f) : I_LBCallback(), m_f(f)       { }
   EFFC_3(LBCallback);

   virtual void operator()(const char* line) const { m_f(line); }
};

/**
 * @brief Allocates a user-defined-length buffer to buffer lines from a file.
 *
 * This function scans a file into a memory buffer, using the @p line_callback
 * function (designed to use a lambda function) to return lines to the calling
 * function.
 *
 * It will throw an std::runtime_error exception if a line is too long to entirely
 * fit in the buffer (including the newline).  The exception will state the line
 * number of the too-long line.
 */
   void t_BufferFile(I_LBCallback &line_callback,
                     int filehandle,
                     int buffsize=1024);

/**
 * @brief Calls t_BufferFile after instantiating the I_LBCallback class from Func &line_callback
 */
template <class Func>
inline void BufferFile(const Func &line_callback, int filehandle, int buffsize=1024)
{
   LBCallback<Func> user(line_callback);
   t_BufferFile(user, filehandle, buffsize);
}


} // end of namespace linebuffer


#endif // LINEBUFFER_HPP
