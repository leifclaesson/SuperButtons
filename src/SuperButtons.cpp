/*
 * SuperButtons.cpp
 *
 *  Created on: 14 July 2020
 *      Author: Leif
 */

#include "Arduino.h"
#include "SuperButtons.h"


static void AppendWithSpace(String & strDestination, const char * szText)
{
	if(strDestination.length()) strDestination+=" ";
	strDestination+=szText;
};

void GetSuperButtonFlagString(String & strFlags, uint8_t flags)
{
	strFlags="";
	if(flags & SuperButtons::solid) AppendWithSpace(strFlags,"SOLID");
	if(flags & SuperButtons::mediumpress) AppendWithSpace(strFlags,"MEDIUM");
	if(flags & SuperButtons::longpress) AppendWithSpace(strFlags,"LONG");
	if(flags & SuperButtons::verylongpress) AppendWithSpace(strFlags,"VERYLONG");
}

const char * GetSuperButtonEventTypeString(eSuperButtonEvent event)
{
	switch(event)
	{
	default:
		return "INVALID";
	case eSuperButtonEvent_Tally:
		return "TALLY";
	case eSuperButtonEvent_Release:
		return "RELEASE";
	case eSuperButtonEvent_Solid:
		return "SOLID";
	case eSuperButtonEvent_MediumPress:
		return "MEDIUMPRESS";
	case eSuperButtonEvent_LongPress:
		return "LONGPRESS";
	case eSuperButtonEvent_VeryLongPress:
		return "VERYLONGPRESS";
	case eSuperButtonEvent_Done:
		return "DONE";
	}
}


bool SuperButtonTracker::Feed(uint32_t code)	//true to accept, either because we're free or because we're already processing this code
{
	if(codeMain && codeMain!=code) return false; //we're busy with another code!

	timestampMain=millis();

	if(codeMain!=code)	//first time seeing this code!
	{
		codeMain=code;
		counterMain=1;

		flags=0;
		tally=1;
		bMediumPress=false;
		bLongPress=false;
		bVeryLongPress=false;

		timestampInitial=timestampContinuous=timestampMain;

		pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_Tally, tally, flags);
	}
	else
	{
		//repeat!

		if(!bContinuous) timestampContinuous=timestampMain;


		if(!bMediumPress && (int) (timestampMain-timestampContinuous)>=mediumpress_ms)
		{
			bMediumPress=true;
			flags |= SuperButtons::mediumpress;

			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_MediumPress, 0, flags);
		}


		if(!bLongPress && (int) (timestampMain-timestampContinuous)>=longpress_ms)
		{
			bLongPress=true;
			flags |= SuperButtons::longpress;

			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_LongPress, 0, flags);
		}

		if( !bVeryLongPress && (int) (timestampMain-timestampContinuous)>=verylongpress_ms)
		{
			bVeryLongPress=true;

			flags |= SuperButtons::verylongpress;

			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_VeryLongPress, 0, flags);
		}





		if(counterMain<255)
		{
			counterMain++;
		}

		//csprintf("repeats: %i\n",counterRemote);

		if(counterMain==2)	//on the second repeat ONLY
		{
			//valid command! let's see who it belongs to.
			flags |= SuperButtons::solid;
			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_Solid, 0, flags);

		}

		if(bGap)
		{
			bGap=false;
			tally++;
			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_Tally, tally, flags);
		}
	}

	bContinuous=true;

	return true;
}

void SuperButtonTracker::Loop()
{

	if(codeMain)
	{

		if(millis()-timestampMain>=gap_time_ms)	//gap since we last saw our code
		{
			if(counterMain>0 && counterMain<2)
			{
				counterMain=0; //if we only saw it once, reset the count -- don't count as solid yet!
			}

			if(!bGap)
			{
				bGap=true;
				pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_Release, tally, flags);
			}

			bContinuous=false;

			bMediumPress=false;
			bLongPress=false;
			bVeryLongPress=false;

		}


		if(millis()-timestampMain>timeout_ms)	//we haven't seen this code for a long time
		{
			pParent->TrackerCallback(this, codeMain, eSuperButtonEvent_Done, tally, flags);
			//csprintf("remote timeout\n");
			codeMain=0;
			counterMain=0;
			bGap=false;
			flags=0;
			bContinuous=false;

		}
	}

}




SuperButtons::SuperButtons()
{
	for(int i=0;i<num_trackers;i++)
	{
		tracker[i].pParent=this;
		tracker[i].tracker_idx=i;
	}


}

void SuperButtons::SetHandler(t_SuperButtonHandler fnHandler)
{
	this->fnHandler=fnHandler;
}

void SuperButtons::SetCustomTimingFunction(t_SuperButtonCustomTiming fnTiming)
{
	this->fnTiming=fnTiming;
}

bool SuperButtons::FeedCode(uint32_t code)
{

	//is any tracker already working with this code?

	for(int i=0;i<num_trackers;i++)
	{
		if(tracker[i].codeMain==code)
		{
			tracker[i].Feed(code);
			return true;
		}
	}

	//use the first free tracker for this new code

	for(int i=0;i<num_trackers;i++)
	{
		if(tracker[i].Feed(code))
		{
			if(fnTiming) fnTiming(this,code,&tracker[i]);
			return true;
		}
	}

	return false;
}

void SuperButtons::Loop()
{
	if(millis()-timestampLoop<20) return;
	timestampLoop=millis();

	for(int i=0;i<num_trackers;i++)
	{
		tracker[i].Loop();
	}

}


void SuperButtons::TrackerCallback(SuperButtonTracker * pSource, uint32_t code, eSuperButtonEvent result, uint8_t count, uint8_t flags)
{
	(void)(pSource);
	snprintf(szLastEvent,sizeof(szLastEvent),"%x*%s,%u,%u",code,GetSuperButtonEventTypeString(result),count,flags);

	if(fnHandler)
	{
		fnHandler(this,code,result,count,flags);
	}

}
