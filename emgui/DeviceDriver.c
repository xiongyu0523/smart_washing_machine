/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software is delivered "as is" and shows the usage of other software
* components. It is provided as an example software which is intended to be
* modified and extended according to particular requirements.
*
* TARA Systems hereby disclaims all warranties and conditions with regard to the
* software, including all implied warranties and conditions of merchantability
* and non-infringement of any third party IPR or other rights which may result
* from the use or the inability to use the software.
*
********************************************************************************
*
* DESCRIPTION:
*   This file implements an interface between an Embedded Wizard generated UI
*   application and a certain device. Please consider this file only as template
*   that is intended to show the binding between an UI application and the
*   underlying system (e.g. middleware, BSP, hardware driver, protocol, ...).
*
*   This device driver is the counterpart to a device class implemented within
*   your Embedded Wizard project.
*
*   Feel free to adapt this file according your needs!
*
*   Within this sample, we demonstrate the access to the board LED and the
*   hardware button. Furthemore, the serial interface is used to print a string.
*
*   This file assumes to be the counterpart of the device class 'DeviceClass'
*   within the unit 'Application'.
*   In order to ensure that this file can be compiled for all projects with or
*   without the device class, the generated define '_ApplicationDeviceClass_'
*   is used.
*
*******************************************************************************/

/*
   Include all necessary files to access the real device and/or to get access
   to the required operating system calls.
*/
#include "ewrte.h"
#include "ew_bsp_inout.h"
#include "ew_bsp_clock.h"

/*
   Include the generated header file to access the device class, for example to
   access the class 'DeviceClass' from the unit 'Application' include the
   generated file 'Application.h'.
*/
#include "Application.h"

#include "Washer.h"

#ifdef _WasherDeviceClass_
  static WasherDeviceClass DeviceObject = 0;
  XInt32 isUpdateProgram = 0;
#endif

#ifdef _ApplicationDeviceClass_

/*
   Create a static variable to keep the global instance (autoobject) of the
   device class. The type of the variable has to correspond to the device class
   and the unit name, for example the static variable of the class 'DeviceClass'
   from the unit 'Application' has the type 'ApplicationDeviceClass'.
*/
  static ApplicationDeviceClass DeviceObject = 0;

  /* variable to detect that hardware button is pressed */
  static int IsHardButtonPressed = 0;

  /* variable to store the current hardware button state */
  static int IsHardButtonDown = 0;

  /* variable to count while Button is pressed */
  static int ButtonCounter = 0;

#endif


/*******************************************************************************
* FUNCTION:
*   HardButtonIsrCallback
*
* DESCRIPTION:
*   Callback function for the hardware button. This function is called every time
*   the hardware button is pressed or released. Please note, that this function is
*   called from the interrupt service routine.
*
* ARGUMENTS:
*   aButtonPresssed - 1, if hardware button is pressed and 0, if it is released.
*
*******************************************************************************/
void HardButtonIsrCallback( int aButtonPresssed )
{

#ifdef _ApplicationDeviceClass_

  if ( aButtonPresssed )
  {
    IsHardButtonPressed = 1;
    IsHardButtonDown = 1;
	 
	
  }
  else
    IsHardButtonDown = 0;

  /*
     Important note: This function is called from an interrupt handler and not
     in the context of the main GUI task/thread/process. NEVER make a direct
     function call to a method of the driver class or any other generated code
     from an interrupt handler or another task/thread/process.
  */

#endif
	if(aButtonPresssed){

		wm_button_pressed();

	}
}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Initialize
*
* DESCRIPTION:
*   The function DeviceDriver_Initialize() initializes the module and prepares all
*   necessary things to access or communicate with the real device.
*   The function has to be called from your main module, after the initialization
*   of your GUI application.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Initialize( void )
{
  /*
     You can implement here the necessary code to initialize your particular
     hardware, to open needed devices, to open communication channels, etc.
  */

  /* configure LED */
  EwBspConfigLed();

  /* Configure interrupt for hardware button */
  EwBspConfigButton( HardButtonIsrCallback );

#ifdef _WasherDeviceClass_

  /*
     Get access to the counterpart of this device driver: get access to the
     device class that is created as autoobject within your Embedded Wizard
     project. For this purpose you can call the function EwGetAutoObject().
     This function contains two paramters: The pointer to the autoobject and
     the class of the autoobject.
     Assuming you have implemented the class 'DeviceClass' within the unit
     'Application' and you have an autoobject with the name 'Device', make
     the following call:
     EwGetAutoObject( &ApplicationDevice, ApplicationDeviceClass );
  */

  DeviceObject = EwGetAutoObject( &WasherDevice, WasherDeviceClass );

  /*
     Once you have the access to the autoobject, lock it as long as you need
     the access (typically, until your device driver is closed). By locking
     the autoobject, you can ensure that the object of the device class will
     be kept within the memory and not freed by the Garbage Collector.
     Once the device class is locked, you can easily store it within a static
     variable to access the driver class during the runtime.
  */

  EwLockObject( DeviceObject );

  WasherDeviceClass__UpdateProgram( DeviceObject, 0 );
  WasherDeviceClass__UpdateTemp( DeviceObject, 1 );
  WasherDeviceClass__UpdateSpin( DeviceObject, 3 );
  WasherDeviceClass__UpdateOption( DeviceObject, 0 );
  WasherDeviceClass__UpdateMinute( DeviceObject, 8 );
  WasherDeviceClass__UpdateSecond( DeviceObject, 0 );
  WasherDeviceClass__UpdateRunning( DeviceObject, 0 );
  
#endif

}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Deinitialize
*
* DESCRIPTION:
*   The function DeviceDriver_Deinitialize() deinitializes the module and
*   finalizes the access or communication with the real device.
*   The function has to be called from your main module, before the GUI
*   application will be deinitialized.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Deinitialize( void )
{
  /*
     You can implement here the necessary code to deinitialize your particular
     hardware, to close used devices, to close communication channels, etc.
  */

#ifdef _WasherDeviceClass_

  /*
     If you have access to the device class autoobject, don't forget to unlock
     it. Clear the static variable to avoid later usage.
  */

  if ( DeviceObject )
    EwUnlockObject( DeviceObject );

  DeviceObject = 0;

#endif

}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_ProcessData
*
* DESCRIPTION:
*   The function DeviceDriver_ProcessData() is called from the main UI loop, in
*   order to process data and events from your particular device.
*   This function is responisble to update properties within the device class
*   if the corresponding state or value of the real device has changed.
*   This function is also responsible to trigger system events if necessary.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   The function returns a non-zero value if a property has changed or if a
*   system event was triggered. If nothing happens, the function returns 0.
*
*******************************************************************************/
int DeviceDriver_ProcessData( void )
{
  int needUpdate = 0;

  /*
     Get the data you want to provide to the GUI application.
     In case your are working with an operating system and your device is
     controlled from a separate task/thread/process, take all information
     from your device driver out of the message queue.
     Please note, that this function is called within the context of the main
     GUI thread.
     If you control your system by direct register access or some BSP functions,
     get all necessary data you want to provide to the GUI application.
  */

#ifdef _WasherDeviceClass_

  /* here we just evaluate the current hardware button state */
  /*if ( IsHardButtonDown )
    ButtonCounter++;
  else
    ButtonCounter = 0;*/

  /* check for a valid access to the autoobject of the device class */
  if ( DeviceObject == 0 )
    return 0;

  /*
     For each device paramter, that is represented by a property within the
     Embedded Wizard device class and that you want to update, you have to call
     the appropriate UpdateProperty() method.

     The following examples assumes, that you have a device class with the
     name 'DeviceClass' within the unit 'Application'.
  */

  /* Update the property HardButtonCounter within the class Application::DeviceClass
     by calling the method 'UpdateHardButtonCounter' - the generated define is
     evaluated to ensures that the method is available within the generated code. */
  #ifdef _ApplicationDeviceClass__UpdateHardButtonCounter_

    ApplicationDeviceClass__UpdateHardButtonCounter( DeviceObject, (XInt32)ButtonCounter );
    
  #endif

  #ifdef 0//_WasherDeviceClass__UpdateProgram_
    
    if ( isUpdateProgram == 1000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 1 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 1 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 1 );
      WasherDeviceClass__UpdateOption( DeviceObject, 1 );
      WasherDeviceClass__UpdateHour( DeviceObject, 1 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 5 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 1 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 2000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 2 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 2 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 2 );
      WasherDeviceClass__UpdateOption( DeviceObject, 0 );
      WasherDeviceClass__UpdateHour( DeviceObject, 2 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 10 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 0 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 3000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 3 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 3 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 3 );
      WasherDeviceClass__UpdateOption( DeviceObject, 1 );
      WasherDeviceClass__UpdateHour( DeviceObject, 3 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 15 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 1 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 4000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 4 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 4 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 4 );
      WasherDeviceClass__UpdateOption( DeviceObject, 0 );
      WasherDeviceClass__UpdateHour( DeviceObject, 4 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 20 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 0 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 5000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 5 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 5 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 5 );
      WasherDeviceClass__UpdateOption( DeviceObject, 1 );
      WasherDeviceClass__UpdateHour( DeviceObject, 5 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 25 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 1 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 6000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 6 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 0 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 0 );
      WasherDeviceClass__UpdateOption( DeviceObject, 0 );
      WasherDeviceClass__UpdateHour( DeviceObject, 6 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 30 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 0 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 7000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 7 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 1 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 1 );
      WasherDeviceClass__UpdateOption( DeviceObject, 1 );
      WasherDeviceClass__UpdateHour( DeviceObject, 7 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 35 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 1 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 8000 )
    {
      WasherDeviceClass__UpdateProgram( DeviceObject, 8 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 2 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 2 );
      WasherDeviceClass__UpdateOption( DeviceObject, 0 );
      WasherDeviceClass__UpdateHour( DeviceObject, 8 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 40 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 0 );
      isUpdateProgram++;
    }
    else if ( isUpdateProgram == 9000 )
    {
      isUpdateProgram = 0;
      WasherDeviceClass__UpdateProgram( DeviceObject, 0 );
      WasherDeviceClass__UpdateTemp( DeviceObject, 0 );
      WasherDeviceClass__UpdateSpin( DeviceObject, 0 );
      WasherDeviceClass__UpdateOption( DeviceObject, 0 );
      WasherDeviceClass__UpdateHour( DeviceObject, 1 );
      WasherDeviceClass__UpdateMinute( DeviceObject, 30 );
      WasherDeviceClass__UpdateRunning( DeviceObject, 1 );
    }
    else
      isUpdateProgram++;
  
  #endif
    
  /*
     Trigger system events if necessary, e.g. if a certain situation happens,
     if an error occurs or just if a certain value has changed...
  */

  /* When the hardware button is pressed, call the method 'TriggerHardButtonEvent()' of the
     device class 'DeviceClass' within the unit 'Application' - the generated define is
     evaluated to ensure that the method is available within the generated code. */
  /*if ( IsHardButtonPressed )
  {
    #ifdef _ApplicationDeviceClass__TriggerHardButtonEvent_

      ApplicationDeviceClass__TriggerHardButtonEvent( DeviceObject );

    #endif

    IsHardButtonPressed = 0;
    needUpdate = 1;
  }*/

#endif

  /*
     Return a value != 0 if there is at least on property changed or if a
     system event was triggered. The return value is used by the main loop, to
     decide whether the GUI application has changed or not.
  */

  return needUpdate;
}

void DeviceDriver_updateWaterTemp(XInt32 aValue){
	WasherDeviceClass__UpdateTemp( DeviceObject, aValue);
}

void DeviceDriver_updateSpinSpeed(XInt32 aValue){
	WasherDeviceClass__UpdateSpin( DeviceObject, aValue);
}

void DeviceDriver_updateLeftTime(XInt32 aValue){
	XInt32 minutes = aValue/60;
	XInt32 seconds = aValue%60;
	WasherDeviceClass__UpdateSecond( DeviceObject, seconds);
	WasherDeviceClass__UpdateMinute( DeviceObject, minutes);
}

void DeviceDriver_updateWashMode(XInt32 aValue){
	WasherDeviceClass__UpdateProgram( DeviceObject, aValue);

}

void DeviceDriver_updateRunning(XInt32 aValue){
	WasherDeviceClass__UpdateRunning( DeviceObject, aValue );

}
void DeviceDriver_updateDrySwitch( XInt32 aValue )
{
  WasherDeviceClass__UpdateOption( DeviceObject, aValue );
}

/*******************************************************************************
* FUNCTION:
*   DeviceDriver_SetLedStatus
*
* DESCRIPTION:
*   This is a sample for a function called from the device class, when a
*   property has changed. As a result, the corresponding value of the real
*   device should be changed.
*   In this implementation simply the LED is switched on or off.
*
*******************************************************************************/
void DeviceDriver_SetLedStatus( XInt32 aValue )
{
  /*
     In case you are using an operating system to communicate with your
     device driver that is running within its own thread/task/process,
     send a message to the device driver and transmit the new value.
     Please note, that this function is called within the context of the main
     GUI thread.
  */

  /*
     Here we are accessing directly the device driver by calling a certain
     BSP / driver function.
  */

  if ( aValue )
    EwBspLedOn();
  else
    EwBspLedOff();
}

void Device_SetProgram( XInt32 aValue )
{
  /*
     In case you are using an operating system to communicate with your
     device driver that is running within its own thread/task/process,
     send a message to the device driver and transmit the new value.
     Please note, that this function is called within the context of the main
     GUI thread.
  */ 

  /*
     Here we are accessing directly the device driver by calling a certain
     BSP / driver function.
  */

  EwPrint( "Change the Washer program value to %u \n", aValue );
  wm_workmode_changed_by_ui(aValue);
}

void Device_SetTemp( XInt32 aValue)
{
  EwPrint( "Change the Washer temp value to %u \n", aValue );
  wm_watertemp_changed_by_ui(aValue);
}

void Device_SetSpin( XInt32 aValue)
{
  EwPrint( "Change the Washer spin value to %u \n", aValue );
  wm_targetspin_changed_by_ui(aValue);
}

void Device_SetOption( XInt32 aValue)
{
  EwPrint( "Change the Washer option value to %u \n", aValue );
  wm_dryswitch_changed_by_gui(aValue);
}

void Device_SetSecond( XInt32 aValue )
{
  EwPrint( "Change the Washer second value to %u \n", aValue );
  wm_second_changed_by_gui(aValue);
}

void Device_SetMinute( XInt32 aValue )
{
  EwPrint( "Change the Washer minute value to %u \n", aValue );
  wm_minute_changed_by_gui(aValue);
}

void Device_SetRunning( XInt32 aValue )
{
  EwPrint( "Change the Washer running value to %u \n", aValue );
  wm_workswitch_changed_by_ui(aValue);
}
/*******************************************************************************
* FUNCTION:
*   DeviceDriver_PrintMessage
*
* DESCRIPTION:
*   This is a sample for a function that is called directly from a 'Command'
*   method of the device class. As a result, the corresponding action should
*   happen.
*   In this implementation the given message is printed via the serial interface.
*
*******************************************************************************/
void DeviceDriver_PrintMessage( XString aText )
{
  char    buf[ 256 ];
  char*   ptr = buf;
  int     buflen = sizeof( buf ) - 1;

  /* check for an empty string */
  if ( aText == 0 )
    return;

  /* read each 16-bit character and convert it to 8-bit character */
  while ( aText && *aText && buflen-- )
    *ptr++ = (char)(*aText++);

  /* terminate the converted string */
  *ptr = 0x00;

  /* do something with the converted string */
  EwPrint( "The string is: %s\n", buf );
}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_SetTime
*
* DESCRIPTION:
*   This is a sample for a function called from the device class, when the
*   system time (RTC time) should be changed.
*
*******************************************************************************/
void DeviceDriver_SetTime( XUInt32 aTime )
{
  /*
     In case you are using an operating system to communicate with your
     device driver that is running within its own thread/task/process,
     send a message to the device driver and transmit the new value.
     Please note, that this function is called within the context of the main
     GUI thread.
  */

  /*
     Here we are accessing directly the device driver by calling a certain
     BSP / driver function.
  */

  EwBspSetTime( aTime );
}


/* msy */
