#include <SuperButtons.h>
#include <RCSwitch.h>

// SuperButtons 433 MHz remote example.
// By Leif Claesson 2020-07-14.
//
// Enjoy!



//for our volume control example code below
int iVolume=50;
bool bVolumeDown=false;
bool bVolumeUp=false;


SuperButtons superbuttons;	//a superbuttons object
RCSwitch rcSwitch = RCSwitch();	//instantiate an RCSwitch object


//called by SuperButtons. superbuttons.SetHandler(SuperButtonHandlerFunction);
void SuperButtonHandlerFunction(SuperButtons * pSource, uint32_t code, eSuperButtonEvent event, uint8_t count, uint8_t flags)
{

	//example code to show how to use the available messages for different button behaviour,
	//without having to reconfigure the library!

	switch(code)
	{
	case 0xe6d798:	//simplest possible function -- activate once per press. for example, activate garage door
		if(event==eSuperButtonEvent_Solid)
			Serial.printf("GARAGE DOOR\n");
		break;
	case 0xe6d799:	//one-second continuous press to start the pool pump
		if(event==eSuperButtonEvent_Tally)
			Serial.printf("Hold to turn on pool pump\n");
		if(event==eSuperButtonEvent_VeryLongPress)
			Serial.printf("POOL PUMP\n");
		break;
	case 0xe6d79c:	//separate short from long press, in this case for a ceiling fan. push to cycle speed 0-1-2-3, hold to turn off.
	{
		static int iFanSpeed=0;
		const char * pszFanSpeed[]={"OFF","LOW","MID","HIGH"};

		switch(event)
		{
		case eSuperButtonEvent_Tally:	//happens for every push! we're ignoring the count.
			iFanSpeed++;
			iFanSpeed%=4;
			Serial.printf("FAN: %s\n",pszFanSpeed[iFanSpeed]);
			break;
		case eSuperButtonEvent_LongPress:
			iFanSpeed=0;
			Serial.printf("FAN: %s\n",pszFanSpeed[iFanSpeed]);
			break;
		default:
			break;
		}
		break;
	}
	case 0xe6d794:	// single-button internet radio preset selector! push once for preset 1, twice for preset 2 etc, hold to stop. I use this at my house, it's fabulous!
	{
		switch(event)
		{
		default:
			break;
		case eSuperButtonEvent_Tally:	//happens for every push!
			Serial.printf("Ding! (counting %i)\n",count);
			break;
		case eSuperButtonEvent_LongPress:
			Serial.printf("STOP playing\n");
			break;
		case eSuperButtonEvent_Done:

			if(flags & SuperButtons::longpress)
			{
				//ignore this event because we already did what we were supposed to on the longpress message
			}
			else
			{
				//do our thing here!
				switch(count)
				{
				case 1: Serial.printf("Tune in NPR\n"); break;
				case 2: Serial.printf("Tune in BBC\n"); break;
				case 3: Serial.printf("Tune in Deep House Radio\n"); break;
				case 4: Serial.printf("Tune in Echoes\n"); break;
				case 5: Serial.printf("Tune in SummerXL\n"); break;
				case 6: Serial.printf("Play the latest 99%% invisible episode\n"); break;
				case 7: Serial.printf("Play the latest Throughline episode\n"); break;
				case 8: Serial.printf("Play the latest Planet Money episode\n"); break;
				default:
					Serial.printf("Total tally: %i\n",count); break;
					break;
				}
			}
			break;
		}

		break;
	}
	case 0xe6d791:	//volume DOWN (repeat)

		switch(event)
		{
		default:
			break;
		case eSuperButtonEvent_Tally:	//happens for every push!
			iVolume--;
			if(iVolume<0) iVolume=0;
			Serial.printf("VOLUME: %i\n",iVolume);
			bVolumeUp=bVolumeDown=false;
			break;
		case eSuperButtonEvent_MediumPress:
			bVolumeDown=true;
			break;
		case eSuperButtonEvent_Release:
			bVolumeDown=false;
			break;
		}
		break;
	case 0xe6d793:	//volume UP (repeat)
		switch(event)
		{
		default:
			break;
		case eSuperButtonEvent_Tally:	//happens for every push!
			iVolume++;
			if(iVolume>100) iVolume=100;
			Serial.printf("VOLUME: %i\n",iVolume);
			bVolumeUp=bVolumeDown=false;
			break;
		case eSuperButtonEvent_MediumPress:
			bVolumeUp=true;
			break;
		case eSuperButtonEvent_Release:
			bVolumeUp=false;
			break;
		}
		break;
	default:	//print out any unhandled code
	{
		String strFlags;

		if(event==eSuperButtonEvent_Done)
		{
			if(flags & SuperButtons::solid) strFlags+=" SOLID";
			if(flags & SuperButtons::mediumpress) strFlags+=" MEDIUM";
			if(flags & SuperButtons::longpress) strFlags+=" LONG";
			if(flags & SuperButtons::verylongpress) strFlags+=" VERYLONG";
		}

		Serial.printf("event: %x %s count %i   %s\n", code, GetSuperButtonEventTypeString(event), count, strFlags.c_str());

		break;
	}
	}


}


void setup()
{
	Serial.begin(115200);

	Serial.printf("\n\nWelcome to this SuperButtons example sketch!\n\n");
	Serial.printf("This sketch requires a 433.92 MHz receiver module and some 433.92 MHz remotes,\n");
	Serial.printf(" and depends on the rc-switch library. It will of course also work with other RF\n");
	Serial.printf(" frequencies (such as 330 MHz) as long as your receiver matches your remotes.\n");
	Serial.printf("\n");
	Serial.printf("It also works with regular buttons if you assign them a number and keep feeding\n");
	Serial.printf(" their value as long as they're held down.\n");
	Serial.printf("\n");
	Serial.printf("Any unknown codes will be printed to this console so you can change the codes\n");
	Serial.printf(" in the switch statement inside the SuperButtonHandlerFunction above.\n");

	rcSwitch.enableReceive(digitalPinToInterrupt(4));  // 433 MHz receiver is connected to GPIO Pin 4.

	superbuttons.SetHandler(SuperButtonHandlerFunction);

}






unsigned long ulRepeatTimestamp=0;


void loop()
{

	if (rcSwitch.available())	//if we've received a code
	{
		unsigned long value=rcSwitch.getReceivedValue();	//save the code
		rcSwitch.resetAvailable();	//release the receiver to listen for a new code

		if(value>=1024)	//ignore lower values which can result in spurious codes from rarely used protocols
		{
			superbuttons.FeedCode(value);	//feed the code to superbuttons
		}
	}

	superbuttons.Loop();	//it'll only execute once every 20ms on its own, it's fine to call it more often


	if(millis()-ulRepeatTimestamp>=100)	//once every 100 milliseconds is plenty
	{

		ulRepeatTimestamp=millis();

		//handle our repeating buttons here. The library doesn't do that for us, because the aim is to _minimize_ the number of messages transmitted over for example MQTT.

		if(bVolumeDown)
		{
			iVolume--;
			if(iVolume<0) iVolume=0;
			Serial.printf("VOLUME: %i\n",iVolume);
		}

		if(bVolumeUp)
		{
			iVolume++;
			if(iVolume>100) iVolume=100;
			Serial.printf("VOLUME: %i\n",iVolume);
		}

	}
}


