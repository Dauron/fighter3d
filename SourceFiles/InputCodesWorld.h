#ifndef __incl_InputCodes_h
#define __incl_InputCodes_h

#define IC_Undefined     0

#define IC_Accept        IC_Undefined+1
#define IC_Reject        IC_Undefined+2
#define IC_Help          IC_Undefined+3
#define IC_Console       IC_Undefined+4

#define IC_MoveForward   IC_Console+1
#define IC_MoveBack      IC_MoveForward+1
#define IC_MoveLeft      IC_MoveForward+2
#define IC_MoveRight     IC_MoveForward+3
#define IC_MoveUp        IC_MoveForward+4
#define IC_MoveDown      IC_MoveForward+5
#define IC_RunModifier   IC_MoveForward+6

#define IC_TurnUp        IC_RunModifier+1
#define IC_TurnDown      IC_TurnUp+1
#define IC_TurnLeft      IC_TurnUp+2
#define IC_TurnRight     IC_TurnUp+3
#define IC_RollLeft      IC_TurnUp+4
#define IC_RollRight     IC_TurnUp+5
#define IC_ZoomIn        IC_TurnUp+6
#define IC_ZoomOut       IC_TurnUp+7

#define IC_ReturnToPlayer IC_ZoomOut+1

#define IC_LClick        IC_Undefined+98
#define IC_RClick        IC_Undefined+99

#define IC_FullScreen    IC_Undefined+100

#define IC_Con_BackSpace    IC_FullScreen+1
#define IC_Con_LineUp       IC_Con_BackSpace+1
#define IC_Con_LineDown     IC_Con_BackSpace+2
#define IC_Con_PageUp       IC_Con_BackSpace+3
#define IC_Con_PageDown     IC_Con_BackSpace+4
#define IC_Con_FirstPage    IC_Con_BackSpace+5
#define IC_Con_LastPage     IC_Con_BackSpace+6
#define IC_Con_StatPrevPage IC_Con_BackSpace+7
#define IC_Con_StatNextPage IC_Con_BackSpace+8

#define IC_CODE_COUNT    IC_Con_StatNextPage+1

#endif
