#ifndef WASHING_MACHINE_H
#define WASHING_MACHINE_H
//Function and protype Declaration
#if 0
#define PROPERTIY_WSWITCH			"WorkSwitch"
#define PROPERTIY_WSTATE			"WorkState"
#define PROPERTIY_CLSWITCH			"ChildLockSwitch"
#define PROPERTIY_WLEVEL			"WaterLevel"
#define PROPERTIY_LTIME 			"LeftTime"
#define PROPERTIY_SOTIME 			"SoakTime"
#define PROPERTIY_WTIME 			"WashTime"
#define PROPERTIY_RTIME 			"RinshTime"
#define PROPERTIY_SPTIME 			"SpinTime"
#define PROPERTIY_RTIMES 			"RinshTimes"
#define PROPERTIY_TSSPEED 			"TargetSpinSpeed"
#define PROPERTIY_TWTEM 			"TargetWaterTemperature"
#define PROPERTIY_DTIME 			"DryTime"
#define PROPERTIY_DSWITCH 			"DrySwitch"
#define PROPERTIY_TDETERGENT 		"TargetDetergent"
#define PROPERTIY_TSOFTENER 		"TargetSoftener"
#define PROPERTIY_TDISINFECTAN 		"TargetDisinfectant"
#define PROPERTIY_DOSTATE  		    "DoorOpeningState"
#define PROPERTIY_PASWITCH  		    "PauseSwitch"
#define PROPERTIY_DOPT  		    "DryOpt"
#define PROPERTIY_RTIMER  		    "ReservationTimer"
#define PROPERTIY_POSWITCH  		    "PowerSwitch"
#else
typedef enum{
	WORK_SW_PRO,
	WORK_ST_PRO,
	CHILD_LOCKSW_PRO,
	WATER_LEVEL_PRO,
	LEFT_TIME_PRO,
	SOAK_TIME_PRO,
	WASH_TIME_PRO,
	RINSH_TIME_PRO,
	SPINE_TIME_PRO,
	RINSH_TIMES_PRO,
	TARGET_SPINSPEED_PRO,
	TARGET_WATERTEM_PRO,
	DRY_TIME_PRO,
	DRY_SW_PRO,
	TARGET_DETERGENT_PRO,
	TARGET_SOFTENER_PRO,
	TARGET_DISINFECTAN_PRO,
	DOOR_OPENING_ST_PRO,
	PAUSE_SW_PRO,
	DRY_OPT_PRO,
	RESERVATION_TIMER_PRO,
	POWER_SW_PRO,
	WASHING_MODE_PRO,
	ALL_PRO,
}wm_propertity_e;

#endif
typedef enum{
	WM_WM_STANDARD,
	WM_WM_SOFT,
	WM_WM_STRONG,
	WM_WM_QUICK,
	WM_WM_WOOL,
	WM_WM_CHEMFIBER,
	WM_WM_COTTON,
	WM_WM_JEANS,
	WM_WM_SELF
}wm_washing_mode_e;
typedef enum{
	WS_IDLE = 0,
	WS_WORKING,
	WS_FINISHED,
	WS_RESERVATION,
	WS_PAUSE,
	WS_ERROR,
	WS_SHUTDOWN
}wm_work_state_e;

typedef enum{
	WL_LOW = 1,
	WL_MIDDLE,
	WL_HIGH

}wm_water_level_e;
typedef enum{
	DO_NONE,
	DO_WEAK,
	DO_MIDDLE,
	DO_HIGH
}wm_dry_opt_e;


typedef struct{
	bool work_switch;
	wm_work_state_e work_state;
	bool clock_switch;
	wm_water_level_e water_level;
	float left_time;
	float soak_time;
	float wash_time;
	float rinsh_time;
	float spin_time;
	int32_t rinsh_times;
	int32_t target_ss;
	float target_wtem;
	float dry_time;
	bool dry_switch;
	float target_detergent;
	float target_softener;
	float target_disinfectant;
	bool door_opening_state;
	bool pause_switch;
	wm_dry_opt_e dry_opt;
	float reserv_time;
	bool power_switch;
	wm_washing_mode_e washing_mode;
}wm_data_info_t;

typedef void (*wm_timer_cb_fun)(void *args);
typedef void (*wm_periodic_cb_fun)(int cnt_left);

typedef struct{
	struct dlist_s *prev;
	struct dlist_s *next;
	int time_set;
	int time_left;
	wm_timer_cb_fun cb_function;
	
	void *cb_args;
	wm_periodic_cb_fun pcb_function;
	char timer_event_name[32];
}wm_timer_event_t;

typedef struct{
	bool work_switch;
	wm_washing_mode_e wm;
	char timer_value[255];

}wm_local_timer_cb_args_t;



#define		WM_CBTIMER_PERIOD_MS			6000//0.5 second					
#define		WM_CONVERT_MINUTES2COUNT(minutes)		((double)(((minutes*60)*1000)/WM_CBTIMER_PERIOD_MS))
#define		WM_CONVERT_COUNT2SECONDS(countv)		((double)((countv*WM_CBTIMER_PERIOD_MS)/1000))
#define		WM_CONVERT_COUNT2MINUTES(countv)		((double)(WM_CONVERT_COUNT2SECONDS(countv)/60))

void wm_property_post(wm_propertity_e epro);
void wm_report_all_to_gui(void );

#endif