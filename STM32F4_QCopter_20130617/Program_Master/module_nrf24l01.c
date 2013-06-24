/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_spi.h"
#include "module_nrf24l01.h"
#include "module_keyBoard.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
u8 TxBuf[SendTimes][TxBufSize] = {0};
u8 RxBuf[ReadTimes][RxBufSize] = {0};

u8 TX_ADDRESS[TX_ADR_WIDTH] = { 0x34,0x43,0x10,0x10,0x01 };		// �w�q�@���R�A�o�e�a�}
u8 RX_ADDRESS[RX_ADR_WIDTH] = { 0x34,0x43,0x10,0x10,0x01 };
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF24L01_Config
**�\�� : nRF24L01 �t�m & �]�w
**��J : None
**��X : None
**�ϥ� : nRF24L01_Config();
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF24L01_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	/* CE PD6 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	/* CSN PA4 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* SCK PA5 */	/* MISO PA6 */	/* MOSI PA7 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* IRQ PD3 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* IRQ EXTI */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource3);
	EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	NRF_CSN = 1;

	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		// ���u�����u
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;												// �D�Ҧ�
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;										// �ƾڤj�p8��
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;														// �������ʡA�Ŷ��ɬ��C
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;													// ��1����u���ġA�W�ɪu�����ˮɨ�
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;														// NSS�H���ѳn�󲣥�
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		// 8���W�A9MHz
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;										// ����b�e
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(nRF_SPI, &SPI_InitStruct);

	SPI_Cmd(nRF_SPI, ENABLE);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_Recv_IRQ
**�\�� : ������Ƥ��_
**��J : None
**��X : None
**�ϥ� : nRF_Recv_IRQ();
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_Recv_IRQ( void )
{
	u8 Sta = 0;
	static u8 nRF_Read = 0;

	NRF_CE = 0;
	Sta = nRF_ReadReg(STATUS);
	if(Sta&RX_DR) {
    LED8 = ~LED8;
		nRF_WriteReg(NRF_WRITE+STATUS, Sta);
		nRF_ReadBuf(RD_RX_PLOAD, RxBuf[nRF_Read], RX_PLOAD_WIDTH);
		nRF_WriteReg(FLUSH_RX, NOP);
// 		nRF_Read++;
// 		if(nRF_Read == ReadTimes)
// 			nRF_Read = 0;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_WriteReg
**�\�� : �g�Ȧs��
**��J : WriteAddr, WriteData
**��X : None
**�ϥ� : nRF_WriteReg(NRF_WRITE+EN_AA, 0x01);
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_WriteReg( u8 WriteAddr, u8 WriteData )
{
	NRF_CE = 0;
	NRF_CSN = 0;
	SPI_WriteByte(nRF_SPI, WriteAddr);
	SPI_WriteByte(nRF_SPI, WriteData);
	NRF_CSN = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_ReadReg
**�\�� : Ū�Ȧs��
**��J : ReadAddr
**��X : ReadData
**�ϥ� : nRF_ReadReg(STATUS);
**=====================================================================================================*/
/*=====================================================================================================*/
u8 nRF_ReadReg( u8 ReadAddr )
{
 	u8 ReadData;

	NRF_CE = 0;
 	NRF_CSN = 0;
	SPI_WriteByte(nRF_SPI, ReadAddr);
	ReadData = SPI_ReadByte(nRF_SPI);
	NRF_CSN = 1;

	return ReadData;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_WriteBuf
**�\�� : �s��g�Ȧs��
**��J : WriteAddr, WriteBuf, Bytes
**��X : None
**�ϥ� : nRF_WriteBuf(NRF_WRITE+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_WriteBuf( u8 WriteAddr, u8 *WriteBuf, u8 Bytes )
{
	u8 i;

	NRF_CE = 0;
	NRF_CSN = 0;

	SPI_WriteByte(nRF_SPI, WriteAddr);

	for(i=0; i<Bytes; i++)
		SPI_WriteByte(nRF_SPI, WriteBuf[i]);

	NRF_CSN = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_ReadBuf
**�\�� : �s��Ū�Ȧs��
**��J : ReadAddr, ReadBuf, Bytes
**��X : None
**�ϥ� : nRF_ReadBuf(TX_ADDR, CheckBuf, 5);
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_ReadBuf( u8 ReadAddr, u8 *ReadBuf, u8 Bytes )
{
	u8 i = 0;

	NRF_CE = 0;
	NRF_CSN = 0;

	SPI_WriteByte(nRF_SPI, ReadAddr);

	for(i=0; i<Bytes; i++)
		ReadBuf[i] = SPI_ReadByte(nRF_SPI);

	NRF_CSN = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_RX_Mode
**�\�� : �ন�����Ҧ�
**��J : None
**��X : None
**�ϥ� : nRF_RX_Mode();
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_RX_Mode( void )
{
	NRF_CE = 0;
	nRF_WriteBuf(NRF_WRITE+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);		// �gRX�`�I�a�}
	nRF_WriteReg(NRF_WRITE+EN_AA, 0x01);				// �ϯ�q�D0���۰�����
	nRF_WriteReg(NRF_WRITE+EN_RXADDR, 0x01);		// �ϯ�q�D0�������a�}
	nRF_WriteReg(NRF_WRITE+RF_CH, CHANAL);			// �]�mRF�q�H�W�v
	nRF_WriteReg(NRF_WRITE+RX_PW_P0, RX_PLOAD_WIDTH);		// ��ܳq�D0�����ļƾڼe��
	nRF_WriteReg(NRF_WRITE+RF_SETUP, 0x0f);			// �]�mTX�o�g�Ѽ�, 0db�W�q, 2Mbps, �C���n�W�q�}��
	nRF_WriteReg(NRF_WRITE+CONFIG, 0x0f);				// �t�m�򥻤u�@�Ҧ����Ѽ�;PWR_UP,EN_CRC, 16BIT_CRC, �����Ҧ�
	NRF_CE = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_TX_Mode
**�\�� : �ন�o�g�Ҧ�
**��J : None
**��X : None
**�ϥ� : nRF_TX_Mode();
**=====================================================================================================*/
/*=====================================================================================================*/
void nRF_TX_Mode( void )
{
	NRF_CE = 0;
	nRF_WriteBuf(NRF_WRITE+TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);		// �gTX�`�I�a�}
	nRF_WriteBuf(NRF_WRITE+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);	// �]�mTX�`�I�a�}, �D�n���F�ϯ�ACK
	nRF_WriteReg(NRF_WRITE+EN_AA, 0x01);			// �ϯ�q�D0���۰�����
	nRF_WriteReg(NRF_WRITE+EN_RXADDR, 0x01);	// �ϯ�q�D0�������a�}
	nRF_WriteReg(NRF_WRITE+SETUP_RETR, 0x05);	// �]�m�۰ʭ��o���j�ɶ�:250us + 86us;�̤j�۰ʭ��o����:5��
	nRF_WriteReg(NRF_WRITE+RF_CH, CHANAL);		// �]�mRF�q�D��CHANAL
	nRF_WriteReg(NRF_WRITE+RF_SETUP, 0x0f);		// �]�mTX�o�g�Ѽ�,0db�W�q,2Mbps,�C���n�W�q�}��
	nRF_WriteReg(NRF_WRITE+CONFIG, 0x0e);			// �t�m�򥻤u�@�Ҧ����Ѽ�;PWR_UP,EN_CRC,16BIT_CRC,�o�g�Ҧ�,�}�ҩҦ����_
	NRF_CE = 1;

	Delay_1us(12); // CE�n�԰��@�q�ɶ��~�i�J�o�e�Ҧ�
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_Check
**�\�� : nRF24L01�˴�
**��J : None
**��X : Status
**�ϥ� : Sta = nRF_Check();
**=====================================================================================================*/
/*=====================================================================================================*/
u8 nRF_Check( void )
{
	u8 TestBuf[5] = {0xC2,0xC2,0xC2,0xC2,0xC2};
	u8 CheckBuf[5];
	u8 i;

	nRF_WriteBuf(NRF_WRITE+TX_ADDR, TestBuf, 5);
	nRF_ReadBuf(TX_ADDR, CheckBuf, 5);
           
	for(i=0; i<5; i++)
		if(CheckBuf[i]!=0xC2)	break;

	if(i==5)
		return SUCCESS;		// MCU �P NRF ���\�s��
	else
		return ERROR;			// MCU �P NRF �����`�s��
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_Tx_Data
**�\�� : �ǰe���
**��J : *TxBuf
**��X : Status
**�ϥ� : nRF_Tx_Data(TxBuf);
**=====================================================================================================*/
/*=====================================================================================================*/
u8 nRF_Tx_Data( u8 *TxBuf )
{
	u8 Sta;

	NRF_CE = 0;
	nRF_WriteBuf(WR_TX_PLOAD, TxBuf, TX_PLOAD_WIDTH);
	NRF_CE = 1;

	while(NRF_IRQ!=0);
	Sta = nRF_ReadReg(STATUS);
	nRF_WriteReg(NRF_WRITE+STATUS, Sta);
	nRF_WriteReg(FLUSH_TX, NOP);

	if(Sta&MAX_RT)
		return MAX_RT;
	else if(Sta&TX_DS)
		return TX_DS;
	else
		return ERROR;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**��� : nRF_Rx_Data
**�\�� : �������
**��J : *RxBuf
**��X : Status
**�ϥ� : nRF_Tx_Data(TxBuf);
**=====================================================================================================*/
/*=====================================================================================================*/
u8 nRF_Rx_Data( u8 *RxBuf )
{
	u8 Sta;

	NRF_CE = 1;
	while(NRF_IRQ!=0);
	NRF_CE = 0;

	Sta = nRF_ReadReg(STATUS);
	nRF_WriteReg(NRF_WRITE+STATUS, Sta);

	if(Sta&RX_DR) {
		nRF_ReadBuf(RD_RX_PLOAD, RxBuf, RX_PLOAD_WIDTH);
		nRF_WriteReg(FLUSH_RX, NOP);
		return RX_DR;
	}
	else
		return ERROR;
}
/*=====================================================================================================*/
/*=====================================================================================================*/