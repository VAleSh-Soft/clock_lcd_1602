// структура для хранения времени
struct tTime
{
    byte hour;
    byte minute;
    byte second;
};

// получение текущего времени
void getCurTime(tTime &time);

// проверка на отображение символа в режиме блинка - одну половину периода
// отображается,вторую - нет
bool checkBlinkTimer();

// проверка времени отображения настроек при отсутствии активности;
// как только вернет false, настройки закрываются
bool checkTimeDisplay();

// установка таймеров
void setTimerData(unsigned long &val);

// отработка нажатия на кнопки +/-
void checkBtnUpDown(shButton &btn, byte &val, bool incr);

// отработка нажатия на кнопку Set
void checkBtnSet(byte h, byte m, byte s);

// сохранение измененного времени при выходе из режима настроек
void saveTimeToRTC(byte Hour, byte Minute, byte Second);

// отображение режима настройки часов и минут
void setHourMode();

// отображение режима настройки секунд
void setSecondMode();

// мигание часов/минут/секунд
void blinkNumber(byte &number);

// отображение моргающего двоеточия
void dpDisplay(boolean disp, boolean dp_blink);

// отображение данных на экране
void showClockData(byte h, byte m);

// вывод числа
void digitalClockDisplay(byte number, byte mode);

// вывод цифры
void printDigits(byte digits, byte x);

// изменение значения в настройках
void checkNumData(byte &val, bool increment, bool toHour);

// создание сегментов цифр
void createSimbols();
