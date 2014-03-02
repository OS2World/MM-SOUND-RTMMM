/* Rexx */



If rxfuncquery('RxCreateRexxThread') Then
  Do
  Call rxfuncadd 'rxydbautilinit','ydbautil','rxydbautilinit'
  Call rxydbautilinit
  End

ret = RxGetNamedSharedMem(rtmmm_memory,'\\SHAREMEM\\RTMMM_SHARED',R)
ret2 = RxOpenEventSem(rtmmm_semaphore,'\\SEM32\\RTMMM_NEWS')
say 'ret2 = 'ret2 

i=1
do  while i=1 
    rc = RxWaitEventSem(rtmmm_semaphore)
    posts = RxResetEventSem(rtmmm_semaphore)
    Say hello! rc posts
end