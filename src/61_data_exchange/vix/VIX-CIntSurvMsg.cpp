#include <stdio.h>
#include <string.h>

#include "VIX-CIntSurvMsg.hpp"
#include "VIX-BSC-defines.hpp"

CIntSurvMsg::CIntSurvMsg(void)
{
	type		= INT_SURV;
	//TODO(JD): set those values to 0 by default when debug done
	year		= 113;
	month		= 8;
	day		= 19;
	hour		= 12;
	min		= 10;
	sec		= 50;
	num_driver	= 4320;
	num_park	= 104;
	etat_expl	= 1;
	num_line	= 6;//3;
	num_service	= 4024;	// No service agent
	num_journey	= 21;	// course NOT USED in VIX code.
	num_stop	= 2045;//2050;
	direction	= 0;
// Donnée d'essai chez VIX
/*
	year		= 113;
	month		= 8;
	day		= 19;
	hour		= 12;
	min		= 10;
	sec		= 50;
	num_driver	= 4320;
	num_park	= 104;
	etat_expl	= 1;
	num_line	= 06;
	num_service	= 4024;	// No service agent
	num_journey	= 247;	// course
	num_stop	= 1743;
	direction	= 1;
*/
}

CIntSurvMsg::~CIntSurvMsg(void)
{
}

void CIntSurvMsg::IntLigneToString(char *p, unsigned int i)
{
	//TODO(jd): add buffer len check
	p[0]=0x20;
	p[1]=0;
	p[2]=0;
	sprintf(p, "%d", i);
	size_t len = strlen(p);
	if(len>0)
	{
		p[len]=0x20;
		p[len+1]=0x20;
		p[len+2]=0x20;
		p[len+3]=0x20;
		p[len+4]=0;
		p[len+5]=0;
	}
}

// because of BSC transparent mode, we need to double any char the has a DLE value.
// so, we must return the number of real char inserted. (So 0 if any error, 1 or 2)
int CIntSurvMsg::insertCharToBufferTransparentMode(unsigned char *buf, int bufSize, unsigned char cToInsert)
{
	//BSC (BiSync) Transparent mode. Double the any DLE found in the sequence.
	int nbInserted = 0;

	// check if the char to insert is a special one.
	if(cToInsert==BSC_CODE_ASCII_DLE)
	{
		// yes, we need to double it.
		if(bufSize>1){
			buf[0]=BSC_CODE_ASCII_DLE;
			buf[1]=BSC_CODE_ASCII_DLE;
			nbInserted=2;	// we must not cound transpatent mode extra data (Will be removed)
		}
	}else{
		// ok, just insert it.
		if(bufSize>0){
			//not a DLE special control character. we can insert it.
			buf[0]=cToInsert;
			nbInserted=1;
		}
	}

	if(nbInserted==0)
	{
		// got an error. buffer too small
		printf("CIntSurv::insertCharToBufferTransparentMode CANNOT insert char. Buffer too small\n");
	}

	return nbInserted;

}

int CIntSurvMsg::StreamToBuffer(unsigned char *buf, int bufSize)
{
	if(bufSize<INT_SURV_BUF_SIZE){
		printf("CIntSurv::StreamToBuffer INVALID buffer size. size:%d. Must be at least: %d\n", bufSize, INT_SURV_BUF_SIZE);
		return 0;
	}

	int o = 0;
	buf[o] = INT_SURV_DATA_SIZE;
	o+=1;
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, type);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, year);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, month);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, day);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, hour);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, min);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, sec);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, (num_driver>>8) & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, num_driver & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, (num_park>>8) & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, num_park & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, etat_expl);

	// this one is a "Text" field.
	// we have to guess the codding. Probably "regular" ASCII
	// but with 'low char' first. and space for null char. like "1   " for decimal 1.
	// so, "9998" for 9998. => 0x38 0x39 0x39 0x39
	// and "128 " for 128. => 0x31 0x32 0x38 0x20
	char p[10];
	IntLigneToString(p, num_line);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, p[0]);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, p[1]);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, p[2]);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, p[3]);

	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, (num_service>>8) & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, num_service & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, (num_journey>>8) & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, num_journey & 0xFF);

	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, (num_stop>>8) & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, num_stop & 0xFF);
	o+=insertCharToBufferTransparentMode(&buf[o], bufSize-o, direction);

	return o;
}