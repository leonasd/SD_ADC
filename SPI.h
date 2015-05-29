/*
 * SPI.h
 *
 *  Created on: 2014Äê11ÔÂ20ÈÕ
 *      Author: LFC
 */

#ifndef SPI_H_
#define SPI_H_

extern unsigned char SPI_Delay;
extern void SPI_CS_HIGH();
extern void SPI_CS_LOW();
extern void SPI_HighSpeed();
extern void SPI_LowSpeed();
extern void SPI_init();
extern unsigned char SPI_TxFrame(unsigned char *pBuffer, unsigned int size);
extern unsigned char SPI_RxFrame(unsigned char *pBuffer, unsigned int size);

#endif /* SPI_H_ */
