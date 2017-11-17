/*
 * lcd8.h
 *
 *  Created on: 14 нояб. 2017 г.
 *      Author: valeriy
 */

#ifndef LCD8_H_
#define LCD8_H_

void lcd_init();
void lcd_char(char ch);
void lcd_cmd(char ch);
void lcd_str(const char *str);
#endif /* LCD8_H_ */
