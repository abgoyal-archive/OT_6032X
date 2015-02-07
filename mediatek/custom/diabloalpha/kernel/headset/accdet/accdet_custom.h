 
 #define JRD_DIABLOALPHA //Used to mask the codes special for diablo alpha product.

#if 1//added by jrd.lipeng
/*Used for iphone headset compitability*/
#define GPIO_JRD_IPHONE_HDST_TRIGGER GPIO178//GPIO_IPHONE_EN
#define JRD_IPHONE_TRIGGER_ON GPIO_OUT_ONE
#define JRD_IPHONE_TRIGGER_OFF ((JRD_IPHONE_TRIGGER_ON == GPIO_OUT_ZERO) ? GPIO_OUT_ONE : GPIO_OUT_ZERO)
#define GPIO_JRD_IPHONE_HDST_EARBIAS_SWITCH GPIO174
#define JRD_EARBIAS_MOS_ON GPIO_OUT_ZERO
#define JRD_EARBIAS_MOS_OFF ((JRD_EARBIAS_MOS_ON == GPIO_OUT_ZERO) ? GPIO_OUT_ONE : GPIO_OUT_ZERO)
 #endif

 /* For diabloalpha, do headset plugging detection just through 
   * the ACCDET module without the assistance of EINT. */
// use accdet + EINT solution
//#define ACCDET_EINT
// support multi_key feature
#define ACCDET_MULTI_KEY_FEATURE
// after 5s disable accdet
#define ACCDET_LOW_POWER

//#define ACCDET_28V_MODE
//#define ACCDET_PIN_RECOGNIZATION

#define ACCDET_SHORT_PLUGOUT_DEBOUNCE
#define ACCDET_SHORT_PLUGOUT_DEBOUNCE_CN 20

struct headset_mode_settings{
    int pwm_width;	//pwm frequence
    int pwm_thresh;	//pwm duty 
    int fall_delay;	//falling stable time
    int rise_delay;	//rising stable time
    int debounce0;	//hook switch or double check debounce
    int debounce1;	//mic bias debounce
    int debounce3;	//plug out debounce
};

//key press customization: long press time
int long_press_time = 2000;

#ifdef  ACCDET_MULTI_KEY_FEATURE
struct headset_mode_settings cust_headset_settings = {
	0x900, 0x900, 1, 0x1f0, 0x800, 0x800, 0x20
};
#else
//headset mode register settings(for MT6575)
struct headset_mode_settings cust_headset_settings = {
	0x1900, 0x140, 1, 0x12c, 0x3000, 0x3000, 0x400
};
#endif
