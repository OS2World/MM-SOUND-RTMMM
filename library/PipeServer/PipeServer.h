#if !defined (_PIPESERVER_H)
#define _PIPESERVER_H

#define  INCL_DOS

#include <os2.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


typedef void (PipeServer_Function_type)( char *msg, char *answer,int answersize) ;


typedef struct {
    void                      *next;
    PipeServer_Function_type  *action;
    unsigned char             *function_name;
} PipeServer_FunctionEntry_type;


class PipeServer
{
private:
    PipeServer_FunctionEntry_type *function_list;
    unsigned char                 *server_name;
    int                           status;          // 0 = suspended
    int                           error_status;    // 0 = no error
    HPIPE                         command_pipe;
    unsigned char                 message[2048];   // a buffer
    unsigned char                 answer[2048];    // a buffer
    TID                           thread;

    void                    Destroy(PipeServer_FunctionEntry_type *p); // destroy one element;
    void                    Execute(void); 
    // THREADS
    void                    PipeServerThread(void *args);

public:
                            PipeServer (const char *name);
                            ~PipeServer (void);

    void                    AddFunction(PipeServer_Function_type *function, const char *name);
    void                    RemoveFunction(PipeServer_Function_type *function);
    void                    Suspend(void);
    void                    Activate(void);

};
    
#endif