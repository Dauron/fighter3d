#ifndef __incl_InputCodes_h
#define __incl_InputCodes_h

#define IC_Undefined     256

#define IC_Accept        IC_Undefined+1
#define IC_Reject        IC_Undefined+2
#define IC_Help          IC_Undefined+3
#define IC_Console       IC_Undefined+4

#define IC_TurnUp        IC_Undefined+10
#define IC_TurnDown      IC_TurnUp+1
#define IC_TurnLeft      IC_TurnUp+2
#define IC_TurnRight     IC_TurnUp+3
#define IC_RollLeft      IC_TurnUp+4
#define IC_RollRight     IC_TurnUp+5

#define IC_OrbitUp       IC_RollRight+1
#define IC_OrbitDown     IC_OrbitUp+1
#define IC_OrbitLeft     IC_OrbitUp+2
#define IC_OrbitRight    IC_OrbitUp+3

#define IC_MoveForward   IC_OrbitRight+1
#define IC_MoveBack      IC_MoveForward+1
#define IC_MoveLeft      IC_MoveForward+2
#define IC_MoveRight     IC_MoveForward+3
#define IC_MoveUp        IC_MoveForward+4
#define IC_MoveDown      IC_MoveForward+5
#define IC_RunModifier   IC_MoveForward+6
#define IC_RunLock       IC_MoveForward+7

#define IC_Con_BackSpace IC_RunLock+1
#define IC_Con_LineUp    IC_Con_BackSpace+1
#define IC_Con_LineDown  IC_Con_BackSpace+2
#define IC_Con_PageUp    IC_Con_BackSpace+3
#define IC_Con_PageDown  IC_Con_BackSpace+4
#define IC_Con_FirstPage IC_Con_BackSpace+5
#define IC_Con_LastPage  IC_Con_BackSpace+6

#define IC_LClick        IC_Undefined+98
#define IC_RClick        IC_Undefined+99

#define IC_FullScreen    IC_Undefined+100

#define IC_CameraChange      IC_FullScreen+1
#define IC_CameraReset       IC_CameraChange+1
#define IC_CameraFront       IC_CameraChange+2
#define IC_CameraBack        IC_CameraChange+3
#define IC_CameraLeft        IC_CameraChange+4
#define IC_CameraRight       IC_CameraChange+5
#define IC_CameraTop         IC_CameraChange+6
#define IC_CameraBottom      IC_CameraChange+7
#define IC_CameraPerspective IC_CameraChange+8

#define IC_PolyModeChange    IC_CameraPerspective+1
#define IC_ViewPhysicalModel IC_PolyModeChange+1
#define IC_ShowBonesAlways   IC_PolyModeChange+2

#define IC_BE_Create          IC_PolyModeChange+10
#define IC_BE_Edit            IC_BE_Create+1
#define IC_BE_Delete          IC_BE_Create+2
#define IC_BE_Select          IC_BE_Create+3
#define IC_BE_Move            IC_BE_Create+4
#define IC_BE_ModeSkeletize   IC_BE_Create+5
#define IC_BE_ModeSkin        IC_BE_Create+6
#define IC_BE_ModeAnimate     IC_BE_Create+7
#define IC_BE_Save            IC_BE_Create+8
#define IC_BE_Play            IC_BE_Create+9
#define IC_BE_Loop            IC_BE_Create+10
#define IC_BE_Modifier        IC_BE_Create+11
#define IC_BE_CreateConstr    IC_BE_Create+12
#define IC_BE_DeleteConstr    IC_BE_Create+13
#define IC_BE_CreateConstrMax IC_BE_Create+14
#define IC_BE_CreateConstrMin IC_BE_Create+15
#define IC_BE_CreateConstrEql IC_BE_Create+16
#define IC_BE_CreateConstrAng IC_BE_Create+17

#define IC_LAST_CODE         IC_BE_CreateConstrAng

#endif
