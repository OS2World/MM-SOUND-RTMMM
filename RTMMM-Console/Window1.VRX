/*:VRX         Main
*/
/*  Main
*/
Main:
/*  Process the arguments.
    Get the parent window.
*/
    parse source . calledAs .
    parent = ""
    argCount = arg()
    argOff = 0
    if( calledAs \= "COMMAND" )then do
        if argCount >= 1 then do
            parent = arg(1)
            argCount = argCount - 1
            argOff = 1
        end
    end; else do
        call VROptions 'ImplicitNames'
        call VROptions 'NoEchoQuit'
    end
    InitArgs.0 = argCount
    if( argCount > 0 )then do i = 1 to argCount
        InitArgs.i = arg( i + argOff )
    end
    drop calledAs argCount argOff

/*  Load the windows
*/
    call VRInit
    parse source . . spec
    _VREPrimaryWindowPath = ,
        VRParseFileName( spec, "dpn" ) || ".VRW"
    _VREPrimaryWindow = ,
        VRLoad( parent, _VREPrimaryWindowPath )
    drop parent spec
    if( _VREPrimaryWindow == "" )then do
        call VRMessage "", "Cannot load window:" VRError(), ,
            "Error!"
        _VREReturnValue = 32000
        signal _VRELeaveMain
    end

/*  Process events
*/
    call Init
    signal on halt
    do while( \ VRGet( _VREPrimaryWindow, "Shutdown" ) )
        _VREEvent = VREvent()
        interpret _VREEvent
    end
_VREHalt:
    _VREReturnValue = Fini()
    call VRDestroy _VREPrimaryWindow
_VRELeaveMain:
    call VRFini
exit _VREReturnValue

VRLoadSecondary:
    __vrlsWait = abbrev( 'WAIT', translate(arg(2)), 1 )
    if __vrlsWait then do
        call VRFlush
    end
    __vrlsHWnd = VRLoad( VRWindow(), VRWindowPath(), arg(1) )
    if __vrlsHWnd = '' then signal __vrlsDone
    if __vrlsWait \= 1 then signal __vrlsDone
    call VRSet __vrlsHWnd, 'WindowMode', 'Modal' 
    __vrlsTmp = __vrlsWindows.0
    if( DataType(__vrlsTmp) \= 'NUM' ) then do
        __vrlsTmp = 1
    end
    else do
        __vrlsTmp = __vrlsTmp + 1
    end
    __vrlsWindows.__vrlsTmp = VRWindow( __vrlsHWnd )
    __vrlsWindows.0 = __vrlsTmp
    do while( VRIsValidObject( VRWindow() ) = 1 )
        __vrlsEvent = VREvent()
        interpret __vrlsEvent
    end
    __vrlsTmp = __vrlsWindows.0
    __vrlsWindows.0 = __vrlsTmp - 1
    call VRWindow __vrlsWindows.__vrlsTmp 
    __vrlsHWnd = ''
__vrlsDone:
return __vrlsHWnd

/*:VRX         CH1_LINK_Click
*/
CH1_LINK_Click: 
    call RTMMM_Link 1
return

/*:VRX         CH1_MUTE_Click
*/
CH1_MUTE_Click: 
  call RTMMM_Mute 1
return

/*:VRX         CH2_LINK_Click
*/
CH2_LINK_Click: 
call RTMMM_Link 2
return

/*:VRX         CH2_MUTE_Click
*/
CH2_MUTE_Click: 
call RTMMM_Mute 2
return

/*:VRX         CH3_LINK_Click
*/
CH3_LINK_Click: 
call RTMMM_Link 3
return

/*:VRX         CH3_MUTE_Click
*/
CH3_MUTE_Click: 
call RTMMM_Mute 3
return

/*:VRX         CH4_LINK_Click
*/
CH4_LINK_Click: 
call RTMMM_Link 4
return

/*:VRX         CH4_MUTE_Click
*/
CH4_MUTE_Click: 
call RTMMM_Mute 4
return

/*:VRX         CH5_LINK_Click
*/
CH5_LINK_Click: 
call RTMMM_Link 5
return

/*:VRX         CH5_MUTE_Click
*/
CH5_MUTE_Click: 
call RTMMM_Mute 5
return

/*:VRX         CH6_LINK_Click
*/
CH6_LINK_Click: 
call RTMMM_Link 6
return

/*:VRX         CH6_MUTE_Click
*/
CH6_MUTE_Click: 
call RTMMM_Mute 6
return

/*:VRX         CH7_LINK_Click
*/
CH7_LINK_Click: 
call RTMMM_Link 7
return

/*:VRX         CH7_MUTE_Click
*/
CH7_MUTE_Click: 
call RTMMM_Mute 7
return

/*:VRX         CH8_LINK_Click
*/
CH8_LINK_Click: 
call RTMMM_Link 8
return

/*:VRX         CH8_MUTE_Click
*/
CH8_MUTE_Click: 
call RTMMM_Mute 8
return

/*:VRX         Fini
*/
Fini:
    window = VRWindow()
    call VRSet window, "Visible", 0
    drop window
return 0

/*:VRX         Halt
*/
Halt:
    signal _VREHalt
return

/*:VRX         Init
*/
Init:
    event_count = -2
    number_of_sliders = 8
    Channels.0 = 0  /* MAX 8 channels */
    Channels.Master = 0 /* Master volume */
    Channels.Busy = 0 /* if the setup is being changed */
    SettingsChanged = 0 /* if something is changed */
    i=1
    do while (i<=8) /* fill remaining channels with 0 */ 
        Channels.i.Active = 0
        Channels.i.Kfilt = 0
        Channels.i.Kright = 0
        Channels.i.Kleft  = 0
        Channels.i.Mute  = 1   /* default as muted */
        Channels.i.Linked  = 1  /* default as linked */
        i = i+1
    end

    call RTMMM_GetInfo

    window = VRWindow()
    call VRMethod window, "CenterWindow"
    call VRSet window, "Visible", 1
    call VRMethod window, "Activate"
    call VRMethod "Application","StartThread","RTMMM_rf"

    Result = Stream("\PIPE\RTMMM_PIPE","C","OPEN");
    if (Result <> "READY:") then do
        return 
    end
    Player = "\PIPE\RTMMM_PIPE"

    drop window
return

/*:VRX         Input1_L_Track
*/
Input1_L_Track: 
    err = RTMMM_Change(1,"L")
return

/*:VRX         Input1_R_Track
*/
Input1_R_Track: 
    err = RTMMM_Change(1,"R")
return

/*:VRX         Input2_L_Track
*/
Input2_L_Track: 
    err = RTMMM_Change(2,"L")
return

/*:VRX         Input2_R_Track
*/
Input2_R_Track: 
    err = RTMMM_Change(2,"R")
return

/*:VRX         Input3_L_Track
*/
Input3_L_Track: 
 err = RTMMM_Change(3,"L")
return

/*:VRX         Input3_R_Track
*/
Input3_R_Track: 
 err = RTMMM_Change(3,"R")
return

/*:VRX         Input4_L_Track
*/
Input4_L_Track: 
 err = RTMMM_Change(4,"L")
return

/*:VRX         Input4_R_Track
*/
Input4_R_Track: 
 err = RTMMM_Change(4,"R")
return

/*:VRX         Input5_L_Track
*/
Input5_L_Track: 
 err = RTMMM_Change(5,"L")
return

/*:VRX         Input5_R_Track
*/
Input5_R_Track: 
 err = RTMMM_Change(5,"R")
return

/*:VRX         Input6_L_Track
*/
Input6_L_Track: 
 err = RTMMM_Change(6,"L")
return

/*:VRX         Input6_R_Track
*/
Input6_R_Track: 
 err = RTMMM_Change(6,"R")
return

/*:VRX         Input7_L_Track
*/
Input7_L_Track: 
 err = RTMMM_Change(7,"L")
return

/*:VRX         Input7_R_Track
*/
Input7_R_Track: 
 err = RTMMM_Change(7,"R")
return

/*:VRX         Input8_L_Track
*/
Input8_L_Track: 
 err = RTMMM_Change(8,"L")
return

/*:VRX         Input8_R_Track
*/
Input8_R_Track: 
 err = RTMMM_Change(8,"R")
return

/*:VRX         MAIN_VOLUME_Track
*/
MAIN_VOLUME_Track: 
    object = "MAIN_VOLUME"
    value = VRGet(object,"Percentile")
    Channels.Master = value * 327.68
    SettingsChanged = 1
    call RTMMM_SetInfo 
return

/*:VRX         Quit
*/
Quit:
    Result = Stream("\PIPE\RTMMM_PIPE","C","CLOSE");
    window = VRWindow()
    call VRSet window, "Shutdown", 1
    drop window
return

/*:VRX         RTMMM_Change
*/
RTMMM_Change: 
    parse arg c_channel, position

/*
    sum_other_channels = 2
    if Channels.channel.Linked = 1 then do 
        object = "input"channel"_"position
        value = VRGet(object,"Percentile")
        fvalue = value * 0.01 
        sum_other_channels = sum_other_channels - (Channels.channel.Kright + Channels.channel.Kleft);
        step = (2 * fvalue) - Channels.channel.Kright - Channels.channel.Kleft
        Channels.channel.Kright = value * 0.01
        Channels.channel.Kleft  = value * 0.01
    end
    else do 
        object = "input"channel"_"position
        value = VRGet(object,"Percentile")
        fvalue = value * 0.01
        sum_other_channels = sum_other_channels - (Channels.channel.Kright + Channels.channel.Kleft);
        if position = "R" then step = Channels.channel.Kright
        else                   step = Channels.channel.Kleft
        if position = "R" then Channels.channel.Kright = fvalue             
        else                   Channels.channel.Kleft  = fvalue
        step = fvalue - step
    end

    if  sum_other_channels > 0 then  ratio = 1 - (step / sum_other_channels);
    else ratio = 0

    i=1
    j= 0+channel
    do while (i<=Channels.0)
        if i=j then do
        end
        else do
            Channels.i.Kright = Channels.i.Kright * ratio
            Channels.i.Kleft  = Channels.i.Kleft * ratio
        end
        i=i+1
    end    
    i = 1
*/
    object = "input"c_channel"_"position
    value = VRGet(object,"Percentile")
    fvalue = value * 327.68 
    if position = "R" then Channels.c_channel.Kright = fvalue
    else                   Channels.c_channel.Kleft = fvalue

    if Channels.c_channel.Linked = 1 then do 
        Channels.c_channel.Kright = fvalue
        Channels.c_channel.Kleft  = fvalue
    end
    else do 
        if position = "R" then Channels.c_channel.Kright = fvalue
        else                   Channels.c_channel.Kleft = fvalue
    end


    SettingsChanged = c_channel
   
    call RTMMM_SetInfo 
    call RTMMM_Show c_channel

return 0

/*:VRX         RTMMM_CONSOLE_Close
*/
RTMMM_CONSOLE_Close:
    call Quit
return

/*:VRX         RTMMM_GetInfo
*/
RTMMM_GetInfo:    
/*
    call lineout Player,"GetEventNumber ." 
    new_event_count = linein(Player)

    if new_event_count = event_count then return  
    event_count = new_event_count 
*/
    call lineout Player,"GetClientInfo ." 

    Channels.0 = linein(Player)
    Channels.Master = 0
 
    i=1
    do while (i<=Channels.0) /* turn all chans off */ 
        Channels.i.Active = 0
        Channels.i.Kfilt  = 0
        Channels.i.Kright = 0
        Channels.i.Kleft  = 0
        i = i+1
    end

    i=1
    do while (i<=Channels.0)
        line = linein(Player)
        parse var line chan name title sampling channels k1 k2 k3 mute_stat master_vol

        chan = chan + 1        
        Channels.Master      = master_vol
        Channels.chan.Mute   = mute_stat
        Channels.chan.Kfilt  = k3
        Channels.chan.Kright = k1 
        Channels.chan.Kleft  = k2
        Channels.chan.Name   = name
        Channels.chan.Title  = title
        if name = "[OFF]" then Channels.chan.Active = 0
        else Channels.chan.Active = 1
        i=i+1
    end    


    call RTMMM_Show -1
return

/*:VRX         RTMMM_Link
*/
RTMMM_Link: 
    parse arg l_channel
    object = "CH"l_channel"_LINK"
    if Channels.l_channel.Linked = 0 then do
        Channels.l_channel.Linked = 1
        err = VRSet(object,"BackColor","Red")
        average = Channels.l_channel.Kright + Channels.l_channel.Kleft
        average = average / 2
        Channels.l_channel.Kright = average
        Channels.l_channel.Kleft  = average
        SettingsChanged = l_channel
        call RTMMM_SetInfo
        call RTMMM_Show l_channel
    end
    else do
        Channels.l_channel.Linked = 0
        err = VRSet(object,"BackColor","PaleGray")
    end
return

/*:VRX         RTMMM_Mute
*/
RTMMM_Mute: 
    parse arg m_channel
    if Channels.m_channel.Mute = 0 then Channels.m_channel.Mute = 1
    else Channels.m_channel.Mute = 0

    SettingsChanged = m_channel
    call RTMMM_SetInfo
    call RTMMM_Show m_channel

return

/*:VRX         RTMMM_Refresh
*/
RTMMM_Refresh: 
    call RTMMM_SetInfo
    call RTMMM_GetInfo
return

/*:VRX         RTMMM_SetInfo
*/
RTMMM_SetInfo:

    if SettingsChanged = -1 then do /* do all channels */
        i = 1
        do while(i<=Channels.0)
            channel = i - 1
            master_vol = Channels.Master
            muted = Channels.i.Mute 
            k1 = Channels.i.Kright 
            k2 = Channels.i.Kleft 
            call lineout Player,"SetClientMixing "channel muted master_vol k1 k2 
            j = linein(Player) /* read answer (dummy)*/
            i=i+1
        end
    end
    else do
        channel = SettingsChanged - 1
        master_vol = Channels.Master
        muted = Channels.i.Mute 
        k1 = Channels.i.Kright 
        k2 = Channels.i.Kleft 
        call lineout Player,"SetClientMixing "channel muted master_vol k1 k2 
        j = linein(Player) /* read answer (dummy)*/
    end
    SettingsChanged = 0

return

/*:VRX         RTMMM_Show
*/
RTMMM_Show:  
    parse arg channel

    Channels.Busy  = 1 /* avoid recalc of values every time we touch a slider */
    value = trunc ((Channels.Master * 100) / 32768)
    err = VRSet("MAIN_VOLUME","Percentile", value )
 
    if channel = -1 then do 
        i = 1
        do while(i<=number_of_sliders)         
            object = "CH"i"_STATUS" 
            if Channels.i.Active = 1  then err = VRSet(object,"BackColor","Red") 
            else                           err = VRSet(object,"BackColor","PaleGray") 
            object = "input"i"_R"
            fvalue = (Channels.i.Kright * 100) / 32768
            value = trunc(fvalue)
            err = VRSet(object,"Percentile", value  ) 
            object = "input"i"_L"
            fvalue = (Channels.i.Kleft * 100) / 32768
            value = trunc(fvalue)
            err = VRSet(object,"Percentile", value ) 
            object = "CH"i"_MUTE"
            if Channels.i.Mute =  0 then err = VRSet(object,"BackColor","Red")
            else                        err = VRSet(object,"BackColor","PaleGray")
            object = "CH"i"_LINK"
            if Channels.i.Linked = 1 then err = VRSet(object,"BackColor","Red")
            else                        err = VRSet(object,"BackColor","PaleGray")
            i=i+1    
        end
    end 
    else do 
            i = channel
            object = "CH"i"_STATUS" 
            if Channels.i.Active = 1  then err = VRSet(object,"BackColor","Red") 
            else                           err = VRSet(object,"BackColor","PaleGray") 
            object = "input"i"_R"
            fvalue = (Channels.i.Kright * 100) / 32768
            value = trunc(fvalue)
            err = VRSet(object,"Percentile", value  ) 
            object = "input"i"_L"
            fvalue = (Channels.i.Kleft * 100) / 32768
            value = trunc(fvalue)
            err = VRSet(object,"Percentile", value ) 
            object = "CH"i"_MUTE"
            if Channels.i.Mute =  0 then err = VRSet(object,"BackColor","Red")
            else                        err = VRSet(object,"BackColor","PaleGray")
            object = "CH"i"_LINK"
            if Channels.i.Linked = 1 then err = VRSet(object,"BackColor","Red")
            else                        err = VRSet(object,"BackColor","PaleGray")\
    end
    Channels.Busy = 0

return

