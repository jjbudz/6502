#ifndef _6502_H_
#define _6502_H_

/**
 * \brief Load an object file from disk.
 *
 * Calling this function clears the contents of the 6502 emulator memory 
 * and resets the state of registers.
 *
 * \param filename fully qualified path and file name
 * \return returns 0 on success; otherwise, returns error number
 */
int Load(const char* filename);

/**
 * \brief Saves an object file to disk.
 *
 * Save is only valid after successful Assemble or Load.
 * 
 * \param filename fully qualified path and file name
 * \return returns 0 on success; othersie, returns error number
 */
int Save(const char* filename);

/**
 * \brief Load an asm file from disk and attempt to turn into object code.
 *
 * Assemble will parse the text file, validate the syntax, and turn the 
 * contents into 6502 object code. Assembly errors are written to stderr.
 * Calling this function clears the contents of the 6502 emulator memory 
 * and resets the state of registers.
 *
 * \param filename fully qualified path and file name
 * \return returns 0 on success; otherwise, returns error number
 */
int Assemble(const char* filename);

/**
 * \brief Run the program at the specified address.
 *
 * \param address memory address to start execution at
 * \return 0 on success; otherwise, returns error number
 */
int Run(const short address=0x4000);

/**
 * \brief Start debugging at the specified address.
 */
int Debug(const short address=0x4000);

/**
 * \brief Step forward one instruction.
 */
int Step();

/**
 * \brief Continue execution from the current instruction.
 */
int Continue();

/** 
 * \brief Dump contents of memory to stderr.
 */
void Memory();

/** 
 * \brief Dump program status to stderr.
 */
void Status();

/** 
 * \brief Dump register values to stderr.
 */
void Registers(); 

/** 
 * \brief Dump stack contents to stderr.
 */
void Stack(); 

/**
 * \todo add the ability to set/clear breakpoints
 * \todo add the ability to get mem/status/reg/stack as structs
 */

#endif
