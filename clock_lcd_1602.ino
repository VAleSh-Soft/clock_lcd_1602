//  создание пользовательских символов для LCD-дисплея:
//  http://mikeyancey.com/hamcalc/lcd_characters.php

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <virtuabotixRTC.h>
#include <shButton.h>
#include "clock_lcd_1602.h"

#define B_SET 2  // кнопка переключения режима настроек
#define B_UP 3   // кнопка увеличения значения
#define B_DOWN 4 // кнопка уменьшения значения

#define DISPLAY_SETTING_TIMER 10000 // время отображения настроек при отсутствии активности, мс
#define BLINK_TIMER 1000            // период мигания цифр при настройке значений и двоеточия в основном режиме, мс

#define SHOW_TIME 0   // режим отображения времени, основной режим
#define SET_HOUR 1    // режим настройки часов
#define SET_MINUTE 2  // режим настройки минут
#define SHOW_SECOND 3 // режим отображения секунд
#define SET_SECOND 4  // режим настройки секунд

#define SHOW_HOUR 0   // показать часы
#define SHOW_MINUTE 1 // показать минуты
#define HIDE_HOUR 2   // скрыть часы
#define HIDE_MINUTE 3 // скрыть минуты

virtuabotixRTC myRTC(6, 7, 8);      // подключение часового модуля - CLK, DAT, RST
LiquidCrystal_I2C lcd(0x27, 16, 2); // установка LCD с адресом 0x27 для отображения 16 символов в двух строках
shButton btnSet(B_SET);             // кнопка выбора режима настроек
shButton btnUp(B_UP);               // кнопка увеличения значений
shButton btnDown(B_DOWN);           // кнопка уменьшения значений

unsigned long blinkTimer = 0;          // таймер для работы с мигающими символами
unsigned long displaySettingTimer = 0; // таймер выхода из настроек при отсутствии активности

byte displayMode = 0;      // текущий режим отображения
bool time_checked = false; // флаг изменения времени в настройках

// массивы для отрисовки сегментов цифр
byte LT[8] =
    {
        B00111,
        B01111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111};

byte UB[8] =
    {
        B11111,
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000};

byte RT[8] =
    {
        B11100,
        B11110,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111};

byte LL[8] =
    {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B01111,
        B00111};

byte LB[8] =
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111};

byte LR[8] =
    {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11110,
        B11100};

byte MB[8] =
    {
        B11111,
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111};

byte BM[8] =
    {
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111};

// массивы сегментов для отрисовки цифр
byte nums[][6]{{0, 1, 2, 3, 4, 5},
               {1, 2, 32, 4, 255, 4},
               {1, 1, 2, 0, 7, 7},
               {1, 6, 2, 4, 4, 5},
               {3, 4, 255, 32, 32, 255},
               {3, 6, 6, 4, 4, 5},
               {0, 6, 6, 3, 4, 5},
               {1, 1, 5, 32, 0, 32},
               {0, 6, 2, 3, 4, 5},
               {0, 1, 2, 7, 7, 5},
               {32, 32, 32, 32, 32, 32}};

void getCurTime(tTime &time)
{
  myRTC.updateTime();
  time.hour = myRTC.hours;
  time.minute = myRTC.minutes;
  time.second = myRTC.seconds;
}

bool checkBlinkTimer()
{
  bool result = false;
  int cur = millis() - blinkTimer;
  if (cur > BLINK_TIMER / 2)
  {
    if (cur <= BLINK_TIMER)
    {
      result = true;
    }
    else
    {
      setTimerData(blinkTimer);
    }
  }
  return (result);
}

void setTimerData(unsigned long &val)
{
  val = millis();
}

bool checkTimeDisplay()
{
  return (millis() - displaySettingTimer < DISPLAY_SETTING_TIMER);
}

void checkBtnUpDown(shButton &btn, byte &val, bool incr)
{
  // значение увеличивать только если кнопка была только что нажата или
  // удерживается свыше интервала задержки
  switch (btn.getButtonState())
  {
  case BTN_DOWN:
    time_checked = true;
    setTimerData(displaySettingTimer);
    if (displayMode == SHOW_SECOND)
    {
      displayMode = SET_SECOND;
    }
    checkNumData(val, incr, displayMode == SET_HOUR);
    break;
  case BTN_LONGCLICK:
    setTimerData(displaySettingTimer);
    checkNumData(val, incr, displayMode == SET_HOUR);
    break;
  }
}

void checkBtnSet(byte h, byte m, byte s)
{
  switch (btnSet.getButtonState())
  {
  case BTN_DOWN:
    if (displayMode == SHOW_SECOND)
    {
      displayMode = SHOW_TIME;
    }
    else
    {
      setTimerData(displaySettingTimer);
      if (time_checked)
      {
        tTime tm;
        getCurTime(tm);
        switch (displayMode)
        {
        case SET_SECOND:
          if (tm.second != s)
          {
            saveTimeToRTC(tm.hour, tm.minute, s);
          }
          displayMode = SHOW_TIME;
          break;
        case SET_HOUR:
        case SET_MINUTE:
          if ((tm.hour != h) || (tm.minute != m))
          {
            saveTimeToRTC(h, m, tm.second);
          }
          break;
        }
      }
      if (displayMode > SHOW_TIME && displayMode <= SET_MINUTE)
      {
        displayMode++;
      }
    }
    break;
  }
}

// Здесь сохранение измененного времени при выходе из режима настройки
void saveTimeToRTC(byte Hour, byte Minute, byte Second)
{
  myRTC.updateTime();
  // секунды, минуты, часы, день недели, число месяца, месяц, год
  myRTC.setDS1302Time(Second, Minute, Hour, myRTC.dayofweek, myRTC.dayofmonth, myRTC.month, myRTC.year);
  time_checked = false;
}

void setHourMode()
{
  byte curHour = 0;
  byte curMinute = 0;
  time_checked = false;
  setTimerData(blinkTimer);
  setTimerData(displaySettingTimer);
  dpDisplay(false, false);

  for (;;)
  {
    if (!time_checked)
    {
      tTime tm;
      getCurTime(tm);
      curHour = tm.hour;
      curMinute = tm.minute;
    }
    // здесь отработка нажатия кнопок
    checkBtnSet(curHour, curMinute, 100);
    if (displayMode > SET_MINUTE)
    {
      break;
    }

    switch (displayMode)
    {
    case SET_HOUR:
      checkBtnUpDown(btnUp, curHour, true);
      checkBtnUpDown(btnDown, curHour, false);
      break;
    case SET_MINUTE:
      checkBtnUpDown(btnUp, curMinute, true);
      checkBtnUpDown(btnDown, curMinute, false);
      break;
    }

    // отображение данных на экране
    showClockData(curHour, curMinute);

    // автовыход из настроек по таймауту
    if (!checkTimeDisplay())
    {
      if (time_checked)
      {
        tTime tm;
        getCurTime(tm);
        // сохранение значений, но только при условии, что они были изменены
        if ((tm.hour != curHour) || (tm.minute != curMinute))
        {
          saveTimeToRTC(curHour, curMinute, tm.second);
        }
      }
      displayMode = SHOW_TIME;
      break;
    }
  }
}

void setSecondMode()
{
  byte curSecond = 0;
  time_checked = false;
  setTimerData(blinkTimer);
  setTimerData(displaySettingTimer);
  digitalClockDisplay(0, HIDE_HOUR);
  dpDisplay(false, false);

  for (;;)
  {
    if (displayMode == SHOW_SECOND)
    {
      tTime tm;
      getCurTime(tm);
      curSecond = tm.second;
    }
    // здесь отработка нажатия кнопок
    checkBtnSet(100, 100, curSecond); // 100 - значения от балды, т.к. в данном случае они не обрабатываются
    if (displayMode == SHOW_TIME)
    {
      break;
    }
    checkBtnUpDown(btnUp, curSecond, true);
    checkBtnUpDown(btnDown, curSecond, false);

    // отображение данных на экране
    showClockData(100, curSecond);

    // автовыход из настроек по таймауту
    if (!checkTimeDisplay())
    {
      displayMode = SHOW_TIME;
      time_checked = false;
      break; // при автовыходе ничего не менять
    }
  }
}

void showClockData(byte h, byte m)
{
  switch (displayMode)
  {
  case SHOW_TIME:
    digitalClockDisplay(h, SHOW_HOUR);
    digitalClockDisplay(m, SHOW_MINUTE);
    break;
  case SET_HOUR:
    blinkNumber(h);
    digitalClockDisplay(m, SHOW_MINUTE);
    break;
  case SET_MINUTE:
    digitalClockDisplay(h, SHOW_HOUR);
    blinkNumber(m);
    break;
  case SHOW_SECOND:
    // пока секунды не начали настраивать, они не мигают, и отображается их нормальный ход
    digitalClockDisplay(m, SHOW_MINUTE);
    break;
  case SET_SECOND:
    // если начали настраивать, начинают мигать и изменяются только по кнопкам
    blinkNumber(m);
    break;
  }
}

void blinkNumber(byte &number)
{
  byte flag = SHOW_MINUTE;
  switch (displayMode)
  {
  case SET_HOUR:
    flag = SHOW_HOUR;
    break;
  }
  // мигание только при условии не нажатых кнопок "+" или "-"
  if (btnUp.isButtonClosed() || btnDown.isButtonClosed() || checkBlinkTimer())
  {
    digitalClockDisplay(number, flag);
  }
  else
  {
    digitalClockDisplay(number, flag + 2);
  }
}

void digitalClockDisplay(byte number, byte mode)
{
  switch (mode)
  {
  case SHOW_HOUR: // отображение часов
    printDigits(number / 10, 0);
    printDigits(number % 10, 4);
    break;
  case SHOW_MINUTE: // отображение минут
    printDigits(number / 10, 9);
    printDigits(number % 10, 13);
    break;
  case HIDE_HOUR: // убрать часы
    printDigits(10, 0);
    printDigits(10, 4);
    break;
  case HIDE_MINUTE: // убрать минуты
    printDigits(10, 9);
    printDigits(10, 13);
    break;
  }
}

void dpDisplay(boolean disp, boolean dp_blink)
{
  if (disp)
  {
    if (dp_blink)
    {
      lcd.setCursor(7, 0);
      lcd.print(" +");
      lcd.setCursor(7, 1);
      lcd.print("+ ");
    }
    else
    {
      lcd.setCursor(7, 0);
      lcd.print("+ ");
      lcd.setCursor(7, 1);
      lcd.print(" +");
    }
  }
  else
  {
    lcd.setCursor(7, 0);
    lcd.print("  ");
    lcd.setCursor(7, 1);
    lcd.print("  ");
  }
}

void printDigits(byte digits, byte x)
{
  if (digits > 9)
  {
    digits = 10;
  }
  lcd.setCursor(x, 0);
  lcd.write(nums[digits][0]);
  lcd.write(nums[digits][1]);
  lcd.write(nums[digits][2]);
  lcd.setCursor(x, 1);
  lcd.write(nums[digits][3]);
  lcd.write(nums[digits][4]);
  lcd.write(nums[digits][5]);
}

void checkNumData(byte &val, bool increment, bool toHour)
{
  if (increment)
  {
    val++;
    if (toHour)
    {
      if (val > 23)
      {
        val = 0;
      }
    }
    else
    {
      if (val > 59)
      {
        val = 0;
      }
    }
  }
  else
  {
    val--;
    if (toHour)
    {
      if (val > 23)
      {
        val = 23;
      }
    }
    else
    {
      if (val > 59)
      {
        val = 59;
      }
    }
  }
}

void createSimbols()
{
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, MB);
  lcd.createChar(7, BM);
}

void setup()
{
  lcd.init();
  lcd.backlight();

  btnUp.setLongClickMode(LCM_CLICKSERIES);
  btnDown.setLongClickMode(LCM_CLICKSERIES);

  createSimbols();

  // Задать данные в часовой модуль в формате:
  // секунды, минуты, часы, день недели, число месяца, месяц, год
  //  myRTC.setDS1302Time(30, 37, 15, 05, 07, 05, 2020);
}

void loop()
{
  // здесь обработка нажатия кнопок, реакция только на кнопку Set, без реакции на удержание кнопки
  switch (btnSet.getButtonState())
  {
  case BTN_DOWN:
    displayMode++;
    break;
  }

  // отображение режимов настройки часов, минут, секунд в зависимости от текущего режима отображения
  switch (displayMode)
  {
  case SET_HOUR:
  case SET_MINUTE:
    setHourMode();
    break;
  case SHOW_SECOND:
  case SET_SECOND:
    setSecondMode();
    break;
  }

  // отображение времени в обычном режиме
  tTime tm;
  getCurTime(tm);
  showClockData(tm.hour, tm.minute);
  dpDisplay(true, checkBlinkTimer());
}
