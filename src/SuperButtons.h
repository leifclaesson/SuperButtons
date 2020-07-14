/*
 * SuperButtons.h
 *
 *  Created on: 14 July 2020
 *      Author: Leif
 */

#pragma once

#include <functional>

class SuperButtons;


enum eSuperButtonEvent
{
	eSuperButtonEvent_Invalid,
	eSuperButtonEvent_Tally,
	eSuperButtonEvent_Release,
	eSuperButtonEvent_Solid,
	eSuperButtonEvent_MediumPress,
	eSuperButtonEvent_LongPress,
	eSuperButtonEvent_VeryLongPress,
	eSuperButtonEvent_Done,
};

const char * GetSuperButtonEventTypeString(eSuperButtonEvent event);

typedef std::function<void(uint32_t code, eSuperButtonEvent event, uint8_t count, uint8_t flags)> SuperButtonHandler;


class SuperButtonTracker	//tracks one code until timeout
{
public:

	bool Feed(uint32_t code);	//true to accept, either because we're free or because we're already processing this code
	void Loop();

	uint32_t timestampMain=0;
	uint32_t codeMain=0;
	uint32_t timestampInitial=0;

	uint32_t timestampContinuous=0;

	uint8_t counterMain=0;

	bool bGap=false;

	bool bContinuous=false;

	bool bMediumPress=false;
	bool bLongPress=false;
	bool bVeryLongPress=false;

	uint8_t tally=0;
	uint8_t flags=0;

private:
	friend class SuperButtons;

	SuperButtons * pParent=NULL;
	uint8_t tracker_idx;

};



class SuperButtons
{
public:
	SuperButtons();

	uint16_t gap_time_ms=100;
	uint16_t mediumpress_ms=250;
	uint16_t longpress_ms=500;
	uint16_t verylongpress_ms=1000;
	uint16_t timeout_ms=750;

	void SetHandler(SuperButtonHandler fnHandler);

	bool FeedCode(uint32_t code);

	void Loop();



	static const int solid=1;
	static const int mediumpress=2;
	static const int longpress=4;
	static const int verylongpress=8;


private:

	SuperButtonHandler fnHandler;


	static const int num_trackers=8;

	SuperButtonTracker tracker[num_trackers];

	friend class SuperButtonTracker;

	void TrackerCallback(SuperButtonTracker * pSource, uint32_t code, eSuperButtonEvent result, uint8_t count, uint8_t flags);

	uint32_t timestampLoop=0;
 
};