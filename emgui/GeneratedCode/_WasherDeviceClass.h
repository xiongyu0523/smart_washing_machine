/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This file was generated automatically by Embedded Wizard Studio.
*
* Please do not make any modifications of this file! The modifications are lost
* when the file is generated again by Embedded Wizard Studio!
*
* The template of this heading text can be found in the file 'head.ewt' in the
* directory 'Platforms' of your Embedded Wizard installation directory. If you
* wish to adapt this text, please copy the template file 'head.ewt' into your
* project directory and edit the copy only. Please avoid any modifications of
* the original template file!
*
* Version  : 9.20
* Profile  : iMX_RT
* Platform : NXP.iMX_RT.RGB565
*
*******************************************************************************/

#ifndef _WasherDeviceClass_H
#define _WasherDeviceClass_H

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "ewrte.h"
#if EW_RTE_VERSION != 0x00090014
  #error Wrong version of Embedded Wizard Runtime Environment.
#endif

#include "ewgfx.h"
#if EW_GFX_VERSION != 0x00090014
  #error Wrong version of Embedded Wizard Graphics Engine.
#endif

#include "_CoreSystemEvent.h"

/* Forward declaration of the class Washer::DeviceClass */
#ifndef _WasherDeviceClass_
  EW_DECLARE_CLASS( WasherDeviceClass )
#define _WasherDeviceClass_
#endif


/* Controller class, containing all data set values and all current values of the 
   washing machine. */
EW_DEFINE_FIELDS( WasherDeviceClass, XObject )
  EW_OBJECT  ( ProgramUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( TempUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( SpinUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( OptionUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( HourUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( MinuteUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( RunningUpdateEvent, CoreSystemEvent )
  EW_OBJECT  ( SetProgEvent,    CoreSystemEvent )
  EW_OBJECT  ( SetTempEvent,    CoreSystemEvent )
  EW_OBJECT  ( SetSpinEvent,    CoreSystemEvent )
  EW_OBJECT  ( SetOptionEvent,  CoreSystemEvent )
  EW_OBJECT  ( SetHourEvent,    CoreSystemEvent )
  EW_OBJECT  ( SetMinuteEvent,  CoreSystemEvent )
  EW_PROPERTY( ProgramNumber,   XInt32 )
  EW_PROPERTY( TempNumber,      XInt32 )
  EW_PROPERTY( OptionNumber,    XInt32 )
  EW_PROPERTY( Hour,            XInt32 )
  EW_PROPERTY( Minute,          XInt32 )
  EW_PROPERTY( SpinNumber,      XInt32 )
  EW_ARRAY   ( WashingProgram,  XString, [9])
  EW_ARRAY   ( WashingTemp,     XString, [6])
  EW_ARRAY   ( Options,         XString, [2])
  EW_ARRAY   ( SpinTurn,        XString, [6])
  EW_PROPERTY( Running,         XBool )
EW_END_OF_FIELDS( WasherDeviceClass )

/* Virtual Method Table (VMT) for the class : 'Washer::DeviceClass' */
EW_DEFINE_METHODS( WasherDeviceClass, XObject )
EW_END_OF_METHODS( WasherDeviceClass )

/* 'C' function for method : 'Washer::DeviceClass.OnSetProgramNumber()' */
void WasherDeviceClass_OnSetProgramNumber( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.OnSetTempNumber()' */
void WasherDeviceClass_OnSetTempNumber( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.OnSetOptionNumber()' */
void WasherDeviceClass_OnSetOptionNumber( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.OnSetHour()' */
void WasherDeviceClass_OnSetHour( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.OnSetMinute()' */
void WasherDeviceClass_OnSetMinute( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.OnSetSpinNumber()' */
void WasherDeviceClass_OnSetSpinNumber( WasherDeviceClass _this, XInt32 value );

/* 'C' function for method : 'Washer::DeviceClass.UpdateProgram()' */
void WasherDeviceClass_UpdateProgram( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateProgram()' */
void WasherDeviceClass__UpdateProgram( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateProgram(). */
#define _WasherDeviceClass__UpdateProgram_

/* 'C' function for method : 'Washer::DeviceClass.OnSetRunning()' */
void WasherDeviceClass_OnSetRunning( WasherDeviceClass _this, XBool value );

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateTemp( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateTemp()' */
void WasherDeviceClass__UpdateTemp( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateTemp(). */
#define _WasherDeviceClass__UpdateTemp_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateSpin( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateSpin()' */
void WasherDeviceClass__UpdateSpin( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateSpin(). */
#define _WasherDeviceClass__UpdateSpin_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateOption( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateOption()' */
void WasherDeviceClass__UpdateOption( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateOption(). */
#define _WasherDeviceClass__UpdateOption_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateMinute( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateMinute()' */
void WasherDeviceClass__UpdateMinute( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateMinute(). */
#define _WasherDeviceClass__UpdateMinute_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateSecond( WasherDeviceClass _this, XInt32 aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateSecond()' */
void WasherDeviceClass__UpdateSecond( void* _this, XInt32 aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateSecond(). */
#define _WasherDeviceClass__UpdateSecond_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void WasherDeviceClass_UpdateRunning( WasherDeviceClass _this, XBool aArg1 );

/* Wrapper function for the non virtual method : 'Washer::DeviceClass.UpdateRunning()' */
void WasherDeviceClass__UpdateRunning( void* _this, XBool aArg1 );

/* The following define announces the presence of the method Washer::DeviceClass.UpdateRunning(). */
#define _WasherDeviceClass__UpdateRunning_

/* Default onget method for the property 'ProgramNumber' */
XInt32 WasherDeviceClass_OnGetProgramNumber( WasherDeviceClass _this );

/* Default onget method for the property 'TempNumber' */
XInt32 WasherDeviceClass_OnGetTempNumber( WasherDeviceClass _this );

/* Default onget method for the property 'OptionNumber' */
XInt32 WasherDeviceClass_OnGetOptionNumber( WasherDeviceClass _this );

/* Default onget method for the property 'Hour' */
XInt32 WasherDeviceClass_OnGetHour( WasherDeviceClass _this );

/* Default onget method for the property 'Minute' */
XInt32 WasherDeviceClass_OnGetMinute( WasherDeviceClass _this );

/* Default onget method for the property 'SpinNumber' */
XInt32 WasherDeviceClass_OnGetSpinNumber( WasherDeviceClass _this );

/* Default onget method for the property 'Running' */
XBool WasherDeviceClass_OnGetRunning( WasherDeviceClass _this );

#ifdef __cplusplus
  }
#endif

#endif /* _WasherDeviceClass_H */

/* Embedded Wizard */
