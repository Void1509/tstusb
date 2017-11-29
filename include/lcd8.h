/*
 * lcd8.h
 *
 *  Created on: 14 нояб. 2017 г.
 *      Author: valeriy
 */

#ifndef LCD8_H_
#define LCD8_H_

#define LCD0		0x80
#define LCD1		0xC0
#define LCD2		0x94
#define LCD3		0xD4
#define LCLS		0x01

void lcd_init();
void lcd_char(char ch);
void lcd_cmd(char ch);
void lcd_str(const char *str);
void lcd_loadfont(uint8_t const *chr, uint8_t n);
#endif /* LCD8_H_ */
