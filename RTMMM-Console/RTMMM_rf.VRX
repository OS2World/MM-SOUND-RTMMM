/*:VRX         Main
*/
Main:
    signal on halt
    call RxFuncAdd 'SysLoadFuncs','Rexxutil','SysLoadFuncs'
    call SysLoadfuncs
    If rxfuncquery('RxCreateRexxThread') Then
    Do
        Call rxfuncadd 'rxydbautilinit','ydbautil','rxydbautilinit'
        Call rxydbautilinit
    End
    call RefreshAction
exit

RefreshAction: procedure
    ret = RxOpenEventSem(rtmmm_semaphore,'\\SEM32\\RTMMM_NEWS')
    posts = RxResetEventSem(rtmmm_semaphore)
    call VRMethod "Application","PostQueue",0,0,"call RTMMM_Refresh","Null",0

    do forever
        ret = RxWaitEventSem(rtmmm_semaphore)
        posts = RxResetEventSem(rtmmm_semaphore)
/*      call SysSleep 1*/
        call VRMethod "Application","PostQueue",0,0,"call RTMMM_Refresh","Null",0
    end

/*:VRX         Halt
*/
Halt:

exit

