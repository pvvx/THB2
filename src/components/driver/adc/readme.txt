调用hal_adc_start的同时，启动一个oals_timer.

example:
//triggle function
  hal_adc_start();
  osal_start_timerEx(adcDemo_TaskID, adcProctect_EVT,50);
...............
...............

//osal task event process

if(event&adcProctect_EVT)
{
 hal_adc_stop();
 hal_clk_reset(MOD_ADCC); 
return (event^adcProctect_EVT)
}