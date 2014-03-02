#if !defined (AUDIO_STREAM_ERRS)
#define AUDIO_STREAM_ERRS

    #define AudioStreamErr(s, errco) \
        {                            \
            cerr << s << "\n";       \
            error_status = errco;    \
            return;                  \
        }


    #define AUDIOSTREAM_OPENING_ERROR           0x00010000 // Object initialization failed step 1
    #define AUDIOSTREAM_OPENING_ERROR1          0x00010001 // Object initialization failed step 2
    #define AUDIOSTREAM_OPENING_ERROR2          0x00010002 // Object initialization failed step 3
    // etc ...
    #define AUDIOSTREAM_FILE_OPEN_ERROR         0x00011000 // Object initialization failed on opening file

    #define AUDIOSTREAM_WRITE_BUFFER_ERROR      0x00020000 // The WriteBuffer instance failed
    #define AUDIOSTREAM_BAD_PARAMETER_ERROR     0x00030000 // Some parameter was wrong
    #define AUDIOSTREAM_WAIT_FAILED_ERROR       0x00040000 // Wait did not complete
    #define AUDIOSTREAM_MEMORY_ERROR            0x00050000 // Some memory allocation error


#endif






